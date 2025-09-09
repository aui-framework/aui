# Minimal UI Template XMake

<!-- aui:example ui -->
Minimal UI boilerplate template XMake.

![](imgs/minimal-template.png)

## Source Code
### Project Structure

```mermaid
graph TD
    A[project_template_ui_xmake] --> B[xmake.lua]
    A --> C[src/]
    C --> D[MainWindow.h]
    C --> E[MainWindow.cpp]
    C --> F[main.cpp]
    A --> G[.vscode/]
    G --> H[c_cpp_properties.json]
```

---

### xmake.lua
<!-- aui:include examples/ui/minimal_ui_xmake/xmake.lua -->

### src/MainWindow.h
<!-- aui:include examples/ui/minimal_ui_xmake/src/MainWindow.h -->

### src/MainWindow.cpp
<!-- aui:include examples/ui/minimal_ui_xmake/src/MainWindow.cpp -->

### src/main.cpp
<!-- aui:include examples/ui/minimal_ui_xmake/src/main.cpp -->

### .vscode/c_cpp_properties.json
<!-- aui:include examples/ui/minimal_ui_xmake/.vscode/c_cpp_properties.json -->
