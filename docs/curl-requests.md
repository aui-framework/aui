# HTTP requests with aui::curl

Requests are described with [ACurl::Builder] and executed either synchronously (this page) or asynchronously (see
[curl-async]). Builder methods return `Builder&`, so calls are chained.

## Simple GET

`runBlocking()` performs the request on the calling thread and returns a `Response` with `code`, `contentType` and
`body`.

<!-- aui:snippet aui.curl/tests/CurlTest.cpp curl_blocking -->

!!! warning

    `runBlocking()` blocks the thread. Never call it from the UI thread; use `runAsync()` instead.

## Query parameters

For GET, `withParams` encodes key-value pairs and appends them to the url.

```cpp
auto response = ACurl::Builder("https://example.com/search")
    .withParams({
        { "q", "hello world" },
        { "page", "2" },
    })
    .runBlocking();
```

## Headers

```cpp
auto response = ACurl::Builder("https://example.com/api/me")
    .withHeaders({
        "Authorization: Bearer " + token,
        "Accept: application/json",
    })
    .runBlocking();
```

To read response headers, set `withHeaderCallback`; it is called for each received header line.

## POST, PUT, DELETE

`withMethod` accepts `ACurl::Method::HTTP_GET` (default), `HTTP_POST`, `HTTP_PUT` and `HTTP_DELETE`.

A form-encoded POST is made with `withParams`:

```cpp
ACurl::Builder("https://example.com/login")
    .withMethod(ACurl::Method::HTTP_POST)
    .withParams({
        { "user", "alex" },
        { "password", "secret" },
    })
    .runBlocking();
```

For an arbitrary body (JSON, for instance) use `withBody` with a string, and set `Content-Type` yourself. The reply can
be parsed with [json-parsing]:

```cpp
auto reply = ACurl::Builder("https://example.com/api/items")
    .withMethod(ACurl::Method::HTTP_POST)
    .withHeaders({ "Content-Type: application/json; charset=utf-8" })
    .withBody(AJson::toString(AJson::Object { { "name", "box" } }).toStdString())
    .runBlocking();
auto json = AJson::fromBuffer(reply.body);
```

`withBody` also has overloads accepting a read callback, and `withInputStream` uploads from any `IInputStream`, such as
a file.

## Uploading files

[AFormMultipart] builds a `multipart/form-data` body; pass it to `withMultipart`.

```cpp
AFormMultipart form;
form["comment"] = { .value = AString("Nice") };
form["file"] = { .value = _new<AFileInputStream>("photo.png"), .filename = AString("photo.png") };
ACurl::Builder("https://example.com/upload")
    .withMethod(ACurl::Method::HTTP_POST)
    .withMultipart(form)
    .runBlocking();
```

## Error handling

There are two kinds of errors.

A *protocol* error, such as 404, is not a failure from curl's point of view. Check `response.code`:

```cpp
if (response.code != ACurl::ResponseCode::HTTP_200_OK) {
    // handle
}
```

A *network* error (DNS failure, timeout, TLS problem) throws [ACurl::Exception] from `runBlocking()`; `curlStatus()`
holds the libcurl code. When using [ACurl] directly (`ACurl(builder).run()`), exceptions are off by default; enable them
with `throwExceptionOnError(true)`, or use `withErrorCallback` (which also disables throwing).

<!-- aui:snippet aui.curl/tests/CurlTest.cpp curl_throw -->

!!! note

    `withTimeout` is the same as `withLowSpeedTime`: it sets how long the transfer speed may stay below the low speed
    limit (1 KiB/s by default, see `withLowSpeedLimit`) before the request is aborted. It is not a limit of the total
    request duration.
