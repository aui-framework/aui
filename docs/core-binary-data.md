# Binary data, compression and identifiers

This page is about bytes: holding them, encoding them as text, compressing, archiving, and generating identifiers.
Reading and writing files is covered in [core-files], text in [core-strings].

## Holding bytes

[AByteBuffer] is a growing array of bytes. Fill it with `operator<<` (it is an [IOutputStream]) or by constructing it
from a pointer and size, and get a non-owning window into it with `slice()`.

```cpp
AByteBuffer buffer;
buffer << std::uint32_t(0xcafebabe) << "tail";       // raw bytes of the number, then the string

AByteBufferView head = buffer.slice(0, 4);           // first 4 bytes, no copy
AByteBuffer copy = AByteBuffer(head);                // own copy
```

!!! warning

    Numbers are written as is, in the native byte order. If you exchange data between machines, agree on an encoding.

A buffer can be constructed from a string with `AByteBuffer::fromString()`, and read back as text with
`AString::fromUtf8(buffer)`. For a different text encoding, use the `AString(AByteBufferView, AStringEncoding)`
constructor and `AString::encode()`.

## Hex and Base64

<!-- aui:snippet aui.core/tests/Base64Test.cpp core_base64 -->

`toHexString()` and `AByteBuffer::fromHexString()` work the same way for hexadecimal.

## Compression

`aui::zlib::compress` and `aui::zlib::decompress` from `AUI/Util/LZ.h` convert between [AByteBuffer]s (both append to
the destination buffer).

<!-- aui:snippet aui.core/tests/ZlibTest.cpp core_zlib -->

If the unpacked data is large, `aui::zlib::decompressToStream` returns an [IInputStream] which decompresses on the fly,
so you can process it chunk by chunk.

## ZIP archives

`AUI/Util/Archive.h` reads and writes ZIP files.

To create an archive, wrap an [AFileOutputStream] (any seekable output stream works) in a `aui::archive::zip::Writer`
and add files with a lambda that writes their contents:

<!-- aui:snippet aui.core/tests/ZlibTest.cpp core_zip_write -->

To read, call `aui::archive::zip::read` with a seekable input stream and a visitor. It is called for each entry, and
the entry's data is available through `open()`. Don't use the stream after the visitor returns.

```cpp
aui::archive::zip::read(AFileInputStream("archive.zip"), [](const aui::archive::FileEntry& entry) {
    ALogger::info("Zip") << entry.name;
    auto data = AByteBuffer::fromStream(entry.open());
});
```

To simply unpack everything to a folder, pass the `aui::archive::ExtractTo` visitor:

```cpp
aui::archive::zip::read(AFileInputStream("archive.zip"), aui::archive::ExtractTo { .prefix = "unpacked" });
```

## UUIDs and random data

[AUuid] parses and prints UUIDs in canonical and raw formats and throws `AUuidException` on malformed input.

<!-- aui:snippet aui.core/tests/UuidTest.cpp core_uuid -->

Use [ARandom] to generate new identifiers or random bytes:

```cpp
ARandom random;
AUuid id = random.nextUuid();
AByteBuffer nonce = random.nextBytes(16);
```

!!! note

    [ARandom] is not a cryptographically secure generator. For secrets, hashing and encryption use `aui::crypt`.
