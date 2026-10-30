# Async requests and downloads

[ACurl::Builder] offers several ways of not blocking the caller. Everything described here keeps the UI responsive.

## runAsync

`runAsync()` schedules the request on the global [ACurlMulti] (a background thread serving all simultaneous
requests) and returns `AFuture<ACurl::Response>`. Network errors are delivered as an exception of the future.

```cpp
ACurl::Builder("https://example.com/api/items").runAsync().onValue([this](const ACurl::Response& r) {
    // not the UI thread; hop to it before touching views.
    AUI_UI_THREAD_X [this, text = AString::fromUtf8(r.body)] {
        mLabel->setText(text);
    };
});
```

In a coroutine, `co_await` the future; to continue with another request, return it from `map`. See
[core-async] for the ways to wait for a future inside an asynchronous function.

To use a dedicated multi instead of the global one, pass it: `runAsync(curlMulti)`.

## Downloading to a file

Pick a destination instead of buffering the body in memory: `withOutputStream` takes any `IOutputStream`.

<!-- aui:snippet aui.curl/tests/CurlTest.cpp curl_to_file -->

For a large file, call `run()` on a worker thread (for example, with `AThreadPool`), or enqueue the [ACurl] into
[ACurlMulti]. Progress is available through `getNumberOfBytesDownloaded()` and `getContentLength()`.

Resuming or fetching a part: `withRanges(begin, end)`.

## Reading a response as a stream

`toInputStream()` returns an `IInputStream` fed by a background request. Reads block until data arrives, so it suits
parsers working with streams:

```cpp
auto stream = ACurl::Builder("https://example.com/data.json").toInputStream();
auto json = AJson::fromStream(*stream);
```

## Cancelling

`ACurl::close()` is thread-safe and non-blocking. After it is called, neither `success` nor `fail` is emitted.

## Signals

[ACurl] emits `success` and `fail` (carrying `ErrorDescription`). Use `withOnSuccess` and `withErrorCallback` on the
builder, or connect to the signals if you construct [ACurl] yourself and add it with `ACurlMulti::global() << curl`.

!!! note

    [ACurlMulti] processes additions on its own thread, so right after `operator<<` its `curls()` may still be empty.
