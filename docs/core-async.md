# Background tasks and timers

A GUI application must keep its UI thread free. This page shows how to move work off it, get the result back, stop it
when it's no longer needed, and run something periodically.

## Running a task on the thread pool

`AUI_THREADPOOL` runs a lambda on the global [AThreadPool] and returns an [AFuture] of whatever the lambda returns.
`*future` (or `future->get()`) blocks until the result is ready.

<!-- aui:snippet aui.core/tests/ThreadingTest.cpp core_async_result -->

!!! warning

    A task lives as long as its [AFuture]. If you drop the future (for example by not storing the return value of
    `AUI_THREADPOOL`), the task is cancelled. Store the future in a field or hand it to an [AAsyncHolder].

`AUI_THREADPOOL` captures by value, like `[=]`. If you need another capture list, use `AUI_THREADPOOL_X` followed by
your own lambda: `AUI_THREADPOOL_X [&] { ... }`.

## Receiving the result without blocking

Blocking on a future in the UI thread defeats its purpose. Attach callbacks instead:

```cpp
class Downloader: public AObject {
public:
    void start() {
        mAsync << (AUI_THREADPOOL {
            return heavyComputation();
        }).onValue([this](int result) {
            // callback thread is not specified, so jump to the UI thread when touching views
            AUI_UI_THREAD {
                mResult = result;
            };
        }).onException([](const AException& e) {
            ALogger::err("Downloader") << "Failed: " << e;
        });
    }

private:
    AAsyncHolder mAsync;
    int mResult = 0;
};
```

`onValue`, `onException` and `onFinally` are called exactly once when the future resolves, or immediately if it has
already resolved. They don't keep the future alive, so something has to own it: that's the job of [AAsyncHolder]
(a set of `AFuture<>` that cancels everything in the destructor, so a callback can't run on a dead `this`).
Calling `onValue` on a temporary future and dropping the result produces a compiler warning for this reason.

An exception reaches `onException` and `*future` wrapped into [AInvocationTargetException]. Every future between the
throw and the catch adds one level, so the chain of causes shows the whole path through nested coroutines.

`AUI_UI_THREAD { ... };` posts a lambda to the main thread's event loop.

## Running code on a particular thread

[AAbstractThread::async] runs a lambda on a given thread and returns an [AFuture] of its result. The usual target is
the UI thread, from a worker or a network callback:

```cpp
AFuture<AString> title = AThread::main()->async([this] { return mLabel->text(); });
```

The lambda runs when the thread processes its messages, never inline. If it returns an `AFuture<T>`, the result is
`AFuture<T>`, not `AFuture<AFuture<T>>`. Don't block on the result on that same thread: the lambda can't run while
the thread waits.

## Cancelling

Cancel a future explicitly with `cancel()`, or just destroy the last copy. If the task is already running, its thread
gets interrupted: blocking calls like `AThread::sleep`, waiting on an `AConditionVariable` and `AThread::interruptionPoint()`
throw an exception that unwinds the task.

<!-- aui:snippet aui.core/tests/ThreadingTest.cpp core_async_cancel -->

Use `AThread::interruptionPoint()` inside long loops to make them cancellable:

```cpp
auto task = AUI_THREADPOOL_X [] {
    for (const auto& item : items) {
        AThread::interruptionPoint();
        process(item);
    }
};
```

## Coroutines

On compilers with C++20 coroutine support, an [AFuture] can be a coroutine return type and be awaited with `co_await`.
The coroutine resumes on the thread that started it, so UI code can stay linear:

```cpp
AFuture<> MyView::load() {
    co_await AThread::asyncSleep(500ms);
    auto result = co_await loadSomethingAsync();   // any function returning AFuture<T>
    setText(result);                               // we're back on the original thread
}
```

Keep the returned future in an [AAsyncHolder].

A coroutine is cancelled together with its future: when the last copy of the future is destroyed, the coroutine stops
at its next `co_await` with an exception that unwinds it.

In a coroutine, [AAbstractThread::switchTo] moves the rest of the coroutine to another thread:

```cpp
AFuture<> MyView::reload() {
    auto data = co_await loadData();        // on whatever thread called reload()
    co_await AThread::main()->switchTo();
    setText(data.title);                    // UI thread
}
```

`switchTo` suspends the coroutine and posts its resumption to the thread's message queue with `enqueue`, like
`AUI_UI_THREAD { ... }` posts a lambda. The coroutine continues when the thread processes its messages, so the thread
must run an event loop: the UI thread has one; a thread of your own needs an [AEventLoop]. `switchTo` does not
suspend if the coroutine is on that thread already. The coroutine stays on the thread until the next switch, including
after a following `co_await future`: an awaited future resumes the coroutine through the queue of the thread it
was suspended on.

## Waiting for a future inside an asynchronous function

A function that returns an `AFuture` often has to wait for another future first. There are three ways:

- **`co_await`**, in a coroutine. The coroutine suspends; the thread keeps running.
- **`then`**, without coroutines. The next step goes into a callback; if it returns another `AFuture`, `then`
  returns that future's result, so the steps don't nest:
  ```cpp
  AFuture<AImage> loadAvatar(AString userId) {
      return fetchUser(std::move(userId)).then([](const User& user) { return fetchImage(user.avatarUrl); });
  }
  ```
  `AFuture<>::all` and `AFuture<>::any` combine several futures the same way, and [AAbstractThread::async] returns
  the result of a future its lambda returns. Inside [AFuture::create], hand a future over with
  `resolver.supplyFrom(other)`.
- **`*future`**, only on a thread pool thread. The worker runs other tasks of the pool while waiting (a stackful
  coroutine), so the pool doesn't deadlock; don't hold locks across the wait. Anywhere else `*future` blocks the
  thread: never do it on the UI thread or in a C++20 coroutine.

## Wrapping a callback-based API

[AFuture::create] turns a callback-based API (a native dialog, a network library, a timer) into a future. Its body
gets an [AFuture::Resolver], the producer side of the future, and may move it anywhere: to another thread, into a
callback.

```cpp
AFuture<APath> browseForFile() {
    return AFuture<APath>::create([](AFuture<APath>::Resolver resolver) {
        nativeDialog.show([resolver](APath path) {
            resolver.supplyValue(std::move(path));
        });
    });
}
```

The future keeps its promise even if the code around it doesn't:

- the first `supplyValue` or `supplyException` wins, later ones are ignored;
- an exception thrown by the body of `create` fails the future;
- if every copy of the resolver is destroyed without a result (a forgotten branch, an early return, an exception in a
  callback), the future fails with [AFutureAbandonedException] instead of hanging forever.

The resolver also tells whether anybody still waits: `isCancelled()` and `onCancel(callback)` let the producer stop
the work when the future is cancelled.

A future that is complete from the start is made with [AFuture::fromValue] or [AFuture::fromException].

## Fire and forget

Destroying the last copy of a future cancels the operation. When the result is not needed, but the operation has to
finish anyway (sending a log, saving a file), call [AFuture::keepAlive]:

```cpp
sendAnalytics(event).onException([](const AException& e) {
    ALogger::warn("Analytics") << e;
}).keepAlive();
```

The operation then runs until it completes or fails, and the callbacks are still called.

## Combining futures

[AFuture::all] waits for several futures and gives their values as a tuple; it fails as soon as one of them fails.
[AFuture::any] takes the first future to complete or fail; the value comes as a variant whose index is the position of
that future. `void` futures give `std::monostate`. Both drop the futures that are still running once they are done, so
those are cancelled.

```cpp
AFuture<> MyView::load() {
    auto [user, avatar] = co_await AFuture<>::all(loadUser(mId), loadAvatar(mId));
    // ...
}
```

`future.withTimeout(5s)` fails with [ATimeoutException] as the cause if `future` does not complete in time. It works
like `AFuture<>::any(future, AThread::asyncSleep(5s))` that keeps the type of the value, except that on timeout
`future` is cancelled even if somebody else holds it, so its producer can abort the operation.

## Coming from other languages

| AUI                                     | JavaScript                              | Kotlin                          | C#                                  | Rust                          |
|-----------------------------------------|-----------------------------------------|---------------------------------|-------------------------------------|-------------------------------|
| `AFuture<T>`                            | `Promise<T>`                            | `Deferred<T>`                   | `Task<T>`                           | `impl Future<Output = T>`     |
| `AUI_THREADPOOL { ... }`                | `new Worker(...)`                       | `async(Dispatchers.Default)`    | `Task.Run(...)`                     | `tokio::spawn_blocking(...)`  |
| `AFuture<T>::create(...)`               | `new Promise((resolve, reject) => ...)` | `suspendCancellableCoroutine`   | `TaskCompletionSource<T>`           | `oneshot::channel()`          |
| `Resolver::onCancel`                    | `AbortSignal`                           | `invokeOnCancellation`          | `CancellationToken`                 | drop of the receiver          |
| `onValue` / `onException` / `onFinally` | `then` / `catch` / `finally`            | `invokeOnCompletion`            | `ContinueWith`                      | `.map(...)` / `.or_else(...)` |
| `thread->async(...)`                    | `postMessage` to a worker               | `async(Dispatchers.Main)`       | `Dispatcher.InvokeAsync(...)`       | `spawn` on a local executor   |
| `co_await thread->switchTo()`           | —                                       | `withContext(Dispatchers.Main)` | `await dispatcher` (`SwitchTo`)     | —                             |
| `co_await future`                       | `await promise`                         | `deferred.await()`              | `await task`                        | `future.await`                |
| `then(...)`                             | `then(v => ...)`                        | `async { f(d.await()) }`        | `ContinueWith(t => ...)`            | `.map(...)`                   |
| `AFuture<>::all(...)`                   | `Promise.all`                           | `awaitAll`                      | `Task.WhenAll`                      | `join!`                       |
| `AFuture<>::any(...)`                   | `Promise.race`                          | `select`                        | `Task.WhenAny`                      | `select!`                     |
| `withTimeout(duration)`                 | `AbortSignal.timeout`                   | `withTimeout`                   | `WaitAsync(timeout)`                | `tokio::time::timeout`        |
| `fromValue` / `fromException`           | `Promise.resolve` / `Promise.reject`    | `CompletableDeferred(value)`    | `Task.FromResult` / `FromException` | `ready(...)`                  |
| `AAsyncHolder`                          | —                                       | `CoroutineScope`                | —                                   | `JoinSet`                     |
| `keepAlive()`                           | default behavior                        | `GlobalScope.launch`            | default behavior                    | `tokio::spawn`                |

The main difference from JavaScript and C#: destroying the last copy of an `AFuture` cancels the operation, as in
Kotlin's structured concurrency and in Rust, where dropping a future stops it. Keep the future (in a field or an
[AAsyncHolder]) while you need the result, or call `keepAlive()`.

## Periodic work with ATimer

[ATimer] emits `fired` every `period` until stopped. It is started explicitly:

```cpp
class Clock: public AObject {
public:
    Clock() {
        connect(mTimer->fired, me::tick);
        mTimer->start();
    }

    void tick() {
        // ...
    }

private:
    _<ATimer> mTimer = _new<ATimer>(1s);
};
```

`fired` is emitted from a dedicated timer thread. If the receiver is a view (or any [AObject] with
`setSlotsCallsOnlyOnMyThread(true)`), the slot is invoked on the receiver's thread instead. See [signal_slot].

The complete example is in `examples/app/game_of_life` and `examples/7guis/timer`.

## Protecting shared data

A plain [AMutex] works with `std::unique_lock`. If the data should never be touched without the lock, wrap it in
[AMutexWrapper]. Debug builds check that the mutex is held when you access the value.

```cpp
AMutexWrapper<AVector<AString>> log;

void append(AString line) {
    std::unique_lock lock(log);
    log->push_back(std::move(line));
}
```

## Your own threads

Most of the time the thread pool is enough. If you need a dedicated thread, create an [AThread]:

```cpp
auto thread = _new<AThread>([] {
    AThread::setName("Worker");
    while (!AThread::current()->isInterrupted()) {
        // ...
        AThread::sleep(100ms);
    }
});
thread->start();
// ...
thread->interrupt();
thread->join();
```

`AThread::sleep` throws on interruption, so a sleeping thread wakes immediately when `interrupt()` is called.
