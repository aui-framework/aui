# Parsing XML

This page shows how to read XML with [aui::xml](xml.md). The parser is event-driven: `AXml::read` pulls bytes from an
input stream and reports attributes, text and nested elements to visitor objects you provide. Nothing is stored for you,
so you decide what to keep.

| Interface                | Receives                                                            |
|--------------------------|---------------------------------------------------------------------|
| [IXmlDocumentVisitor]    | top-level elements and the `<?xml ... ?>` header                    |
| [IXmlEntityVisitor]      | attributes, text and nested elements of a single element            |
| [IXmlHeaderVisitor]      | attributes of the `<?xml ... ?>` header                             |

## Reading attributes

Given a file like this:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<settings>
    <window width="800" height="600" title="Demo"/>
</settings>
```

create one visitor per element you are interested in. `visitEntity` returns the visitor for the element it was called
for; the parser then feeds that element's attributes and children into it.

```cpp
#include <AUI/Xml/AXml.h>
#include <AUI/IO/AFileInputStream.h>

// ignores an element and everything inside of it; see "Ignoring elements"
struct IgnoreVisitor: IXmlEntityVisitor {
    _<IXmlEntityVisitor> visitEntity(AString entityName) override {
        return _new<IgnoreVisitor>();
    }
};

struct WindowVisitor: IXmlEntityVisitor {
    int width = 0;
    int height = 0;
    AString title;

    void visitAttribute(const AString& name, AString value) override {
        if (name == "width") {
            width = value.toInt().valueOr(0);
        } else if (name == "height") {
            height = value.toInt().valueOr(0);
        } else if (name == "title") {
            title = std::move(value);
        }
    }
};

struct SettingsVisitor: IXmlEntityVisitor {
    _<WindowVisitor> window;

    _<IXmlEntityVisitor> visitEntity(AString entityName) override {
        if (entityName == "window") {
            window = _new<WindowVisitor>();
            return window;
        }
        return _new<IgnoreVisitor>();
    }
};

struct DocumentVisitor: IXmlDocumentVisitor {
    _<SettingsVisitor> settings;

    _<IXmlEntityVisitor> visitEntity(AString entityName) override {
        settings = _new<SettingsVisitor>();
        return settings;
    }
};

auto document = _new<DocumentVisitor>();
AXml::read(_new<AFileInputStream>("settings.xml"), document);
// document->settings->window->width == 800
```

Visitors are plain objects: keep the results in their fields and read them after `AXml::read` returns. Attributes are
reported in the order they appear. An attribute without a value (`<input disabled>`) is reported with an empty string.

Attribute values come as [AString]; use `toInt()` / `toDouble()` for numbers, which return an `AOptional`.

## Reading from a string

`AXml::read` takes an [IInputStream], so a string needs to be wrapped in an `AStringStream`:

```cpp
#include <AUI/IO/AStringStream.h>

AXml::read(_new<AStringStream>(AString(R"(<item id="1"/>)")), document);
```

## Text and nested elements

`visitTextEntity` is called with the text between tags, and `visitEntity` for each nested element. The calls follow
document order, so a visitor can accumulate a list of children and text pieces:

```cpp
struct ListVisitor: IXmlEntityVisitor {
    AVector<AString> items;

    _<IXmlEntityVisitor> visitEntity(AString entityName) override {
        items << std::move(entityName);
        return _new<IgnoreVisitor>();
    }
};
```

Text that consists of whitespace only (indentation, line breaks) is reported too, and so can be empty strings between
adjacent elements; filter them out with `trim()` if you only care about meaningful text.

## Ignoring elements

To ignore an element along with all of its attributes, text and children, return `nullptr` from `visitEntity`. The
parser still reads the element through to its closing tag, so the rest of the document is parsed as usual.

```cpp
struct Visitor: IXmlEntityVisitor {
    _<IXmlEntityVisitor> visitEntity(AString entityName) override {
        if (entityName == "debug") {
            return nullptr;   // <debug>...</debug> is skipped
        }
        return _new<Visitor>();
    }
};
```

## The XML header

[IXmlDocumentVisitor] additionally has `visitHeader`, called for `<?xml ... ?>`. The default implementation returns
`nullptr`, which ignores the header (unlike elements, this is safe). Return an [IXmlHeaderVisitor] to get its
attributes:

```cpp
struct HeaderVisitor: IXmlHeaderVisitor {
    void visitAttribute(const AString& name, const AString& value) override {
        ALogger::info("Xml") << name << " = " << value;   // version = 1.0, encoding = UTF-8
    }
};

struct DocumentVisitor: IXmlDocumentVisitor {
    _<IXmlHeaderVisitor> visitHeader() override {
        return _new<HeaderVisitor>();
    }
    // ...
};
```

## Handling errors

A malformed document makes `AXml::read` throw [AXmlParseError], which derives from `std::runtime_error`:

```cpp
try {
    AXml::read(_new<AStringStream>(AString("<item id=1/>")), document);
} catch (const AXmlParseError& e) {
    ALogger::err("Xml") << "Bad xml: " << e.what();
}
```

The parser is lenient: it throws only when an attribute value is not quoted or a closing tag is malformed. A truncated
document ends silently at the end of the input without an error, so do not rely on the exception to validate
documents.

The error message ends with the position of the offending character in the form `row:column`.

## Limitations

aui::xml is a minimal parser (it is what [AText] uses for its HTML-like markup), not a full XML implementation.
Be aware of the following:

- Element names may consist of letters and digits only: `my-tag`, `my_tag` and `ns:tag` are not recognized. Attribute
  names may additionally contain `:`.
- XML entities (`&amp;`, `&lt;`, `&#10;`) are not decoded; you receive them as is. Instead, backslash escapes (`\n`,
  `\t`, `\\`, `\uXXXX`) are processed in text and attribute values, as in other AUI tokenizers, which a regular XML
  document does not expect. A literal backslash in the data is therefore best avoided.
- Comments (`<!-- -->`), `<![CDATA[ ]]>` and `<!DOCTYPE>` are not supported.
- Namespaces, schemas and validation are not supported.
- There is no XML writer. To produce XML, build the string yourself, escaping `&`, `<`, `>` and quotes.

If you need more than this, use a dedicated XML library and import it with AUI.Boot (see [aui.boot]).
