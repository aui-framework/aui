# Minimal UI Template XMake with Assets

<!-- aui:example ui -->
Minimal UI boilerplate template XMake with [aui-assets].

![](imgs/minimal-assets-template.png)

## Source Code
### Project Structure

```mermaid
graph TD
    A[project_template_ui_assets_xmake] --> B[xmake.lua]
    A --> C[src/]
    C --> D[MainWindow.h]
    C --> E[MainWindow.cpp]
    C --> F[main.cpp]
    A --> G[.vscode/]
    G --> H[c_cpp_properties.json]
    A --> I[xmake/]
    I --> J[aui.lua]
    A --> K[assets/img/]
    K --> L[logo.svg]
```

---

### xmake.lua
<!-- aui:include examples/ui/minimal_ui_assets_xmake/xmake.lua -->

### xmake/aui.lua
<!-- aui:include examples/ui/minimal_ui_assets_xmake/xmake/aui.lua -->

### src/MainWindow.h
<!-- aui:include examples/ui/minimal_ui_assets_xmake/src/MainWindow.h -->

### src/MainWindow.cpp
<!-- aui:include examples/ui/minimal_ui_assets_xmake/src/MainWindow.cpp -->

### src/main.cpp
<!-- aui:include examples/ui/minimal_ui_assets_xmake/src/main.cpp -->

### .vscode/c_cpp_properties.json
<!-- aui:include examples/ui/minimal_ui_assets_xmake/.vscode/c_cpp_properties.json -->
