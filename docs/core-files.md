# Files, paths and streams

This page covers the most common file system tasks: building paths, reading and writing files, listing folders and
using streams. Types here live in `AUI/IO`.

## Paths

[APath] is an [AString] with file system helpers, so it can be printed, compared and passed as a string. It always uses
forward slashes, on Windows too.

```cpp
APath dir = "/home/user/work";
APath file = dir / "report.txt";        // /home/user/work/report.txt
file.filename();                        // report.txt
file.filenameWithoutExtension();        // report
file.parent();                          // /home/user/work
file.extensionChanged("md");            // /home/user/work/report.md
```

<!-- aui:snippet aui.core/tests/FilesTest.cpp core_path_extension -->

Query and modify the file system with `exists()`, `isRegularFileExists()`, `isDirectoryExists()`, `fileSize()`,
`makeDir()`, `makeDirs()` (creates missing parents), `removeFile()` and `removeFileRecursive()`.

```cpp
APath cache = APath::getDefaultPath(APath::HOME) / ".cache" / "myapp";
cache.makeDirs();
```

Use `APath::copy(from, to)` and `APath::move(from, to)` to copy and move files.

### Well-known locations

`APath::getDefaultPath` returns `HOME`, `TEMP` or `APPDATA` of the current user on every platform. To store your own
application's settings or databases, prefer [AApplication::dataDir()].

For scratch files use `APath::nextRandomTemporary()`, which returns a non-existent path in the temp directory, and
wrap it in an [APathOwner] to have the file removed automatically:

```cpp
APathOwner scratch(APath::nextRandomTemporary());
AFileOutputStream(scratch.value()) << "temporary";
// the file is removed when `scratch` is destroyed
```

### Listing a folder

`listDir()` returns an `ADeque<APath>` with the children; each path includes the folder it was listed from. `.` and
`..` are skipped. Pass flags such as `AFileListFlags::DIRS` or `AFileListFlags::REGULAR_FILES` to choose what you
get.

```cpp
for (const APath& child : APath("/home/user/work").listDir()) {
    if (child.isRegularFileExists() && child.extension() == "txt") {
        ALogger::info("Demo") << child.filename() << ": " << child.fileSize() << " bytes";
    }
}
```

## Reading files

[AFileInputStream] opens a file for reading and throws an [AIOException] subclass (for example
`AFileNotFoundException`) if it can't. The easiest way to get all data out of a stream is `AByteBuffer::fromStream`:

```cpp
auto bytes = AByteBuffer::fromStream(AFileInputStream("config.txt"));
AString text = AString::fromUtf8(bytes);
```

For big files, read in chunks instead. `read()` returns how many bytes it has put in the buffer, and `0` means the
end of the stream.

```cpp
AFileInputStream file("big.bin");
char buffer[0x1000];
while (auto size = file.read(buffer, sizeof(buffer))) {
    process(buffer, size);
}
```

## Writing files

[AFileOutputStream] creates (or truncates) the file. Pass `true` as the second argument to append.
`operator<<` accepts strings, [AByteBuffer]s, and numbers (numbers are written as raw bytes in native byte order, not
as text).

```cpp
{
    AFileOutputStream out("log.txt", true /* append */);
    out << "line\n";
}
```

The file is closed when the stream is destroyed.

<!-- aui:snippet aui.core/tests/FilesTest.cpp core_file_copy -->

## Streams

Files are just one kind of [IInputStream] and [IOutputStream]. Code that takes the interfaces works with files,
network sockets, memory buffers and compressed data alike:

```cpp
AString readAll(IInputStream& is) {
    return AString::fromUtf8(AByteBuffer::fromStream(is));
}
```

Besides files, `aui::core` provides [AByteBufferInputStream] (stream over a buffer), [AConcatInputStream] (several
streams in a row), [ALimitedInputStream] (first N bytes of a stream) and [APipe] (an in-process pipe). `IInputStream`
also has `readExact()`, which keeps reading until the requested number of bytes arrive and throws if the stream ends.

!!! note

    Reading from a stream may block. Don't do it on the UI thread if the source can be slow; see [core-async].

To store and restore structures, JSON is the convenient choice: [json-writing] and [json-parsing].
