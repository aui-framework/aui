clang-format is a de facto standard tool to auto format (C++) code with style described by `.clang-format` file that
typically located inside your project's root directory. AUI @ref CLANG_FORMAT "has such file".

Since AUI abuses C++'s syntax, it's important to set up appropriate auto formatting, or you will often find yourself
struggling with AUI's DSL especially in large portions of layout. For your project, we recommend to start with AUI's
formatting configuration listed above. AUI's [App Template](https://github.com/aui-framework/example_app) has
`.clang-format` already. Your IDE should pick up it without further configuration.

# Always Use Trailing Comma in Initializer Lists

When it comes to `clang-format` there's one an unobvious feature when using AUI's DSL. Consider the example:

<table>
  <tr>
    <th>With Trailing Comma</th>
    <th>Without Trailing Comma</th>
  </tr>
  <tr>
<td>
@code{cpp}
setContents(Vertical {
  Button { "Up" },
  Button { "Down" },
});
@endcode
</td>
<td>
@code{cpp}
setContents(Vertical {
  Button { "Up" },
  Button { "Down" }
});
@endcode
</td>
  </tr>
</table>

See the difference? The second example lacks one comma. If we try to trigger `clang-format`
(<kbd>ALT</kbd>+<kbd>CTRL</kbd>+<kbd>L</kbd> in **CLion**), we'll get the following results (assuming AUI's
@ref CLANG_FORMAT ".clang-format"):

<table>
  <tr>
    <th>With Trailing Comma</th>
    <th>Without Trailing Comma</th>
  </tr>
  <tr>
<td>
@code{cpp}
setContents(Vertical {
  Button { "Up" },
  Button { "Down" },
});
@endcode
</td>
<td>
@code{cpp}
setContents(Vertical {
  Button { "Up" }, Button { "Down" } });
@endcode
</td>
  </tr>
</table>

The first example left as is (correct), the second example formatted confusingly.

When using any kind of AUI's DSL with initializer lists please always use trailing comma. Not only it helps with
reordering, git diffs, etc..., but also `.clang-format` makes proper formatting for lists with trailing commas.

# Use clang-format off/on

In some scenarios clang-format may fight against you, especially with complicated syntax. You can use
`// clang-format off` and `// clang-format on` to disable and enable clang-format, respectively.

@snippet aui.json/tests/JsonFieldsTest.cpp clang format toggle

# AUI's .clang-format {#CLANG_FORMAT}

@include .clang-format
