# AGENTS.md — AUI Framework

Guidance for AI coding agents working with the **AUI Framework** (Advanced Universal Interface):
a cross-platform, high-performance, module-based C++20 framework for hardware-accelerated graphical
desktop applications. Inspired by Qt, but pure C++ — no custom languages, no external compilers.

## Repository layout

AUI is a monorepo of independent modules. Each `aui.<name>/` folder is a self-contained module with
this canonical structure:

```
aui.<module>/
  CMakeLists.txt   # uses aui_module(...) / aui_executable(...)
  src/             # sources (auto-globbed by aui_module/aui_executable)
  tests/           # GTest/GMock unit tests (enabled via aui_enable_tests)
  benchmarks/      # optional micro-benchmarks
  3rdparty/        # optional vendored deps
  assets/          # optional aui-assets (compiled/embedded via aui_compile_assets)
```

### Modules

| Module            | Purpose                                                         |
|-------------------|-----------------------------------------------------------------|
| `aui.core`        | Central library: types, containers, threading, i/o, reflection, signal-slot, properties |
| `aui.views`       | UI toolkit: views, layouts, ASS styling, rendering              |
| `aui.image`       | Image loading and processing                                    |
| `aui.crypt`       | OpenSSL wrapper                                                  |
| `aui.curl`        | http(s)/ftp requests                                            |
| `aui.network`     | TCP/UDP networking                                              |
| `aui.json`        | JSON parser                                                     |
| `aui.xml`         | XML parser                                                      |
| `aui.audio`       | Audio recording and playback                                    |
| `aui.uitests`     | UI testing utilities                                            |
| `aui.updater`     | App auto-updates for non-centralized distribution               |
| `aui.toolbox`     | Build-time tool (e.g. converts assets to embeddable `.cpp`)     |
| `aui.mysql`, `aui.remote_tools` | Additional/optional modules                      |

### Top-level things worth knowing

- `aui.boot.cmake` — AUI.Boot: the CMake-based package manager. **It has extensive inline docs** in a
  `#[==[DOCUMENTATION ... ]==]` block (several hundred lines). Read that block in full before answering
  any question about `auib_import`, caching (`AUIB_CACHE`), precompiled packages, or its CLI mode
  (`cmake -P aui.boot.cmake <command>`).
- `docs/` — the full documentation site (MkDocs). Start with `docs/index.md` and `docs/getting-started.md`.
- `examples/` — see below.
- `cmake/` — build helpers (`aui.build.cmake`, platform/toolchain files, packaging).
- `CMakePresets.json`, top-level `CMakeLists.txt` — the framework's own build.
- `cmake-build-*/` — local build trees (generated; not source).

---

## `examples/` folder

`examples/` contains runnable sample apps demonstrating AUI features. It is grouped by theme:

- `examples/app/` — near-production apps (`minesweeper`, `notes`, `game_of_life`, `fractal`, `auigram`, `template`).
- `examples/ui/` — focused UI-building samples (`minimal_ui`, `button*`, `checkbox*`, `progressbar`,
  `infinite_lazy_list`, `hot_code_reload`, `opengl_simple`, `embedded_sdl`, `views`, ...).
- `examples/7guis/` — the classic *7 GUIs* benchmark tasks (`counter`, `temperature_converter`,
  `flight_booker`, `timer`, `crud`, `circle_drawer`, `cells`).

Build examples by configuring the framework with `-DAUI_BUILD_EXAMPLES=TRUE`, then run e.g.
`./bin/aui.example.views`. The `examples/README.md` notes this is a limited subset — the full list is
at <https://aui-framework.github.io/master/examples/>. **Use these as canonical, copy-pasteable
references** for idiomatic AUI code (UI DSL, layouts, data binding, assets).

---

## Key documentation map (`docs/`)

Read the relevant doc before making non-trivial changes:

- **Getting started / build**: `getting-started.md`, `app-build-overview.md`, `aui-configure-flags.md`
- **CMake API**: `aui_module.md`, `aui_executable.md`, `aui_link.md`, `aui_app.md`, `aui-assets.md`, `macros.md`
- **UI**: `ui-building-overview.md`, `views-overview.md`, `aui-box-model.md`, `render-to-texture.md`,
  `retained_immediate_ui.md`
- **Testing**: `writing-tests.md` (GTest + GMock; enable with `aui_enable_tests`)
- **Style & contributing**: `code-style.md`, `contributing.md`, `forking-aui.md`
- **Packaging & platforms**: `packaging.md`, `android.md`, `ios.md`, `linux.md`, `macos.md`,
  `windows.md`, `emscripten.md`, `crosscompiling*.md`
- **Runtime**: `runtime-dependency-resolution.md`, `devtools.md`, `troubleshoot-list.md`

---

## Code style (see `docs/code-style.md` for the full rules)

- 4 spaces, no tabs (2 spaces acceptable inside UI-building DSL blocks). Max line length 120.
- Public API classes are `CamelCase` prefixed with `A` (e.g. `AString`, `AWindow`, `AView`). The
  `A` / `aui::` / `AUI_` prefixes mark **stable public API**; unprefixed symbols are *internal*
  (usable, but no API-stability guarantee).
- Use the `GenericSpecific` naming pattern: `EventClose`, not `CloseEvent`.
- Functions/variables `lowerCamelCase`; constants `UPPER_SNAKE_CASE`; member fields `mLikeThis`.
- Getters: `lineNumber()`; setters: `setLineNumber(...)` / builder-style `withAccessible(...)`.
- Constructors/setters take by value + `std::move` so the caller chooses copy vs move.
- Use `const`, `noexcept`, `[[nodiscard]]` wherever possible.
- Prefer `#pragma once`. Doxygen comments use `@`-style, not `\`.
- Avoid macros; when unavoidable, prefix with `AUI_` in `UPPER_SNAKE_CASE`.
- Prefer concepts over SFINAE. Use assertion helpers `AUI_ASSERT` / `AUI_ASSERTX` (with a helpful tip
  message). Do NOT put algorithm-necessary logic inside asserts (they are stripped in release).
- **Always use a trailing comma in AUI DSL initializer lists** so `clang-format` formats them nicely:
  ```cpp
  setContents(Vertical {
      Label { "Up" },
      Label { "Down" },   // <- trailing comma
  });
  ```
- Formatting is governed by `.clang-format`; disable locally with `// clang-format off` / `on`.

---

## Building & testing (framework)

```bash
# configure once
cmake -B build -GNinja            # add -DAUI_BUILD_EXAMPLES=TRUE for examples
# build
cmake --build build --parallel
cmake --build build -t Tests
# run tests (GTest filters supported)
./build/bin/Tests "--gtest_filter=SomeSuite*"
```

Platform/compiler branching uses `AUI_PLATFORM_*` and `AUI_COMPILER_*` in both C++ (`#if`) and CMake
(`if()`), with platform-specific source dirs like `src/Platform/win32`, `src/Platform/linux`. Never
use compiler-specific things like `#ifdef _WIN32`.

---

## Contributing conventions

- git-flow-like: branch from `master` into `feat/<feature-name>`; open PRs targeting `master`.
- Don't break build or tests; run tests before proposing changes.
- **AI-assisted work is welcome, but must not contain obvious AI traces**: no hallucinations, no
  filler ("I'm happy to help"), no over-explanation, no walls of bullet points. Take full ownership
  and manually review generated code/docs.

---

## Consumer projects

An **AUI consumer project** is any application that imports AUI (rather than being AUI itself). It
typically looks like the official [`example_app`](https://github.com/aui-framework/example_app)
template:

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_app VERSION 0.0.1)
include(aui.boot.cmake)
set(AUI_VERSION v8.0.0-rc.21)
auib_import(aui https://github.com/aui-framework/aui
        COMPONENTS core views curl json crypt updater
        VERSION ${AUI_VERSION})
aui_executable(${PROJECT_NAME})              # auto-globs src/
aui_link(${PROJECT_NAME} PRIVATE aui::core aui::views)
aui_compile_assets(${PROJECT_NAME})          # compile/embed assets/
aui_enable_tests(${PROJECT_NAME})            # enable tests/
aui_app(TARGET ${PROJECT_NAME} NAME "..." VENDOR "..." ICON "assets/img/icon.svg")
```

When working in a consumer project:

- Edit the **consumer's** `src/`, `tests/`, `CMakeLists.txt`, `assets/` — never AUI's own source that
  was pulled into the cache/deps folder.
- Entry point is `AUI_ENTRY { ... }` (from `<AUI/Platform/Entry.h>`); build UI declaratively with the
  DSL (`using namespace declarative;`, `Vertical { ... }`, `Centered { ... }`, `Label`, `_new<AButton>()`),
  and follow the same code style as above.
- Use `_new<T>(...)` / `_<T>` smart pointers, signal-slot `connect(...)`, and the property/data-binding
  system as shown in `examples/` and the consumer's own `MainWindow`.
- Consumer templates ship with `.clang-format`, `.clang-tidy`, GitHub Actions CI, and Valgrind
  suppressions — respect them. Releases are cut by bumping the version in `CMakeLists.txt`.

### Viewing or editing AUI's own sources from a consumer project

If the user wants to **see AUI's source code** or **make changes to AUI itself** while working in a
consumer project, reconfigure their project with `-DAUIB_AUI_AS=TRUE`:

```bash
cd build
cmake .. -DAUIB_AUI_AS=TRUE
```

This tells AUI.Boot to import AUI via `add_subdirectory` instead of `find_package`, so AUI becomes an
editable part of the consumer's build tree — its sources show up as a folder in the IDE and edits take
effect immediately on the next build. It is the on-the-fly equivalent of passing `ADD_SUBDIRECTORY` to
`auib_import(...)` (no need to touch the consumer's `CMakeLists.txt`), and it can be toggled on an
existing build tree. Note: this **disables precompiled binaries** — AUI is built locally.

During configuration AUI.Boot prints where the editable source tree lives, e.g.:

```
Imported: aui () (/home/user/.aui/repo/aui/as/v8.0.0-rc.21/aui) (version v8.0.0-rc.21)
```

Edit AUI's code **there** (that path), not in the consumer's `src/`. To contribute those changes back
upstream, follow the fork/branch workflow in `docs/contributing.md` (`feat/<feature-name>` → PR to
`master`). See `docs/aui-configure-flags.md` and `aui.boot.cmake` (`AUIB_<PackageName>_AS`) for the
full flag reference.
