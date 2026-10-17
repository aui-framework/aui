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
        }).onSuccess([this](int result) {
            // callback thread is not specified, so jump to the UI thread when touching views
            AUI_UI_THREAD {
                mResult = result;
            };
        }).onError([](const AException& e) {
            ALogger::err("Downloader") << "Failed: " << e;
        });
    }

private:
    AAsyncHolder mAsync;
    int mResult = 0;
};
```

`onSuccess`, `onError` and `onFinally` are called exactly once when the future resolves, or immediately if it has
already resolved. They don't keep the future alive, so something has to own it: that's the job of [AAsyncHolder]
(a set of `AFuture<>` that cancels everything in the destructor, so a callback can't run on a dead `this`).

`AUI_UI_THREAD { ... };` posts a lambda to the main thread's event loop.

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
