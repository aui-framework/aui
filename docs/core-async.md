# Background tasks and timers

A GUI application must keep its UI thread free. This page shows how to move work off it, get the result back, stop it
when it's no longer needed, and run something periodically.

## Running a task on the thread pool

[AThreadPool::async] runs a lambda on the global [AThreadPool] and returns an [AFuture] of whatever the lambda
returns. `*future` (or `future->get()`) blocks until the result is ready.

<!-- aui:snippet aui.core/tests/ThreadingTest.cpp core_async_result -->

!!! warning

    A task lives as long as its [AFuture]. If you drop the future (for example by not storing the return value of
    `async`), the task is cancelled. Store the future in a field or hand it to an [AAsyncHolder].

Spell the captures out. A task that captures `this`, a raw pointer or a reference may still be running when that
object is destroyed, see [Cancelling](#cancelling). `AUI_THREADPOOL_X [captures] { ... }` is the same as `async` in
the form of a macro.

`AUI_THREADPOOL { ... }` is deprecated: its capture is `[=]`, which captures `this` without saying so.

## Receiving the result without blocking

Blocking on a future in the UI thread defeats its purpose. Attach callbacks instead:

```cpp
class Downloader: public AObject {
public:
    void start() {
        mAsync << AThreadPool::global().async([] {
            return heavyComputation();
        }).onValue([this](int result) {
            // callback thread is not specified, so jump to the UI thread when touching views
            AThread::main()->enqueue(this, [this, result] {
                mResult = result;
            });
        }).onException([](const AException& e) {
            ALogger::err("Downloader") << "Failed: " << e;
        });
    }

private:
    int mResult = 0;
    AAsyncHolder mAsync;   // the last member: it is destroyed first, before the members its tasks use
};
```

`onValue`, `onException` and `onFinally` are called exactly once when the future resolves, or immediately if it has
already resolved. They don't keep the future alive, so something has to own it: that's the job of [AAsyncHolder]
(a set of `AFuture<>` that cancels everything and waits for running tasks in the destructor, so a callback can't run
on a dead `this`).
Calling `onValue` on a temporary future and dropping the result produces a compiler warning for this reason.

An exception reaches `onException` and `*future` wrapped into [AInvocationTargetException]. Every future between the
throw and the catch adds one level, so the chain of causes shows the whole path through nested coroutines.

A lambda posted to another thread is not in the holder. `AThread::main()->enqueue(this, fn)` posts `fn` to the main
thread's event loop bound to `this`: if the object is destroyed by then, `fn` is dropped; while `fn` runs, the object
is held. The object has to be owned by a shared pointer, as `_new<Downloader>()` is. `AUI_UI_THREAD { ... }` is
deprecated: nothing owns what it posts, and its `[=]` captures `this` without saying so.

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

Neither `cancel()` nor destroying the last copy waits for a running task. A task that does not reach an interruption
point keeps running after its future is gone. If the task uses an object (for example, it captures `this`), stop it
before destroying the object: call `cancel()` and then `wait()`, which returns when the task is no longer running.
[AAsyncHolder] does this in its destructor, so declare it after the members its tasks use:

```cpp
class Scene {
public:
    ~Scene() {
        // a single future: stop the task before the members it uses are destroyed
        if (mCompose != nullptr) {
            mCompose.cancel();
            mCompose.wait();
        }
    }

private:
    AMutex mSync;
    AVector<Entity> mEntities;
    AFuture<> mCompose = nullptr;   // its task locks mSync and reads mEntities
    AAsyncHolder mAsync;            // the last member: its destructor cancels and waits before the members above die
};
```

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

### Lifetime of what a coroutine uses

A coroutine keeps its parameters and locals in a frame on the heap, and the frame outlives the call that started it.
Mind what it refers to:

- **Captures of a lambda that is a coroutine** live in the lambda object, not in the frame. A temporary lambda is
  destroyed at the end of the full expression, while the coroutine is suspended, so its captures dangle after the
  first `co_await`. Pass what the coroutine needs as parameters by value instead:
  ```cpp
  mAsync << [](_<Model> model) -> AFuture<> {   // not [model]() -> AFuture<>
      co_await model->load();
  }(mModel);
  ```
- **Reference and pointer parameters**, `this` of a member coroutine included, are stored as they are. The object
  has to outlive the coroutine.
- **Cancellation doesn't destroy the coroutine at once.** The coroutine stays suspended until the awaited future
  completes; then it is resumed on its thread and `co_await` throws to unwind it. `catch` blocks and destructors of
  its locals run at that moment, which may be after the object that owned the future is destroyed: don't touch
  `this` there. [AAsyncHolder] doesn't wait for that either.
- **After `switchTo` to another thread**, the coroutine is in the same position as a thread pool task: the owner has
  to wait for it before destroying what it uses (see [Cancelling](#cancelling)).

A member coroutine that runs on the UI thread only, awaits futures of tasks that capture by value, and is held by an
[AAsyncHolder] declared last is safe as long as it doesn't touch `this` while it unwinds: the object lives and dies on
the same thread, so the coroutine can't be running while the object is destroyed.

`co_await` of a future returns to the thread the coroutine was on, so it never moves the coroutine. To do that, use
[AAbstractThread::switchTo], for example to run blocking code on a thread of your own and get back to the UI:

```cpp
AFuture<> MyView::reload() {
    co_await mDatabaseThread->switchTo();
    auto title = mDatabase->queryTitle();   // blocking, on mDatabaseThread
    co_await AThread::main()->switchTo();
    setText(title);                         // UI thread
}
```

`switchTo` suspends the coroutine and posts its resumption to the thread's message queue with `enqueue`. The coroutine continues when the thread processes its messages, so the thread
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
| `AThreadPool::global().async(...)`      | `new Worker(...)`                       | `async(Dispatchers.Default)`    | `Task.Run(...)`                     | `tokio::spawn_blocking(...)`  |
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
