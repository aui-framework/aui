AUI is maintained by 1 developer so community help and contributions are heavily appreciated. Here's some helpful
information.

# Becoming a contributor

1. Follow [AUI's code style](docs/Code style and recommendations.md).
2. Anyone may become an AUI contributor: there are no restrictions.
3. Tasks and todos are listed on the [GitHub Issues page](https://github.com/aui-framework/aui/issues). Issues marked
   as `good first issue` are relatively simple and can be done without deep dive into AUI.
4. AUI follows git flow-like branching model. `master` branch is a release candidate branch. `develop` branch is current 
   develop branch. Each feature of the framework is created in a separate branch from `develop` in the `feat/` folder. 
   pulling changes from your branch to `develop` is possible only if the merge with your branch does not generate merge 
   conflicts and does not break the build and tests.

   That is, if you want to implement a feature, you should fork AUI's repository (if you don't have direct access to
   it's repository), checkout the `develop` branch and create a new branch called `feat/feature-name`, where you work
   under your feature.

   When you have finished your work, you should open a pull request from the `feat/feature-name` branch of your
   repository to the `develop` branch of the AUI's original repository.


# Creating an AUI module

Suppose whe want to create a module called `aui::my_module`:

1. In the main `CMakeLists.txt`:
   1. Find the `# define all components` anchor, put `define_aui_component(my_module)`.
   2. Find the `# all components for exporting` anchor, put `my_module` to `AUI_ALL_COMPONENTS`.
2. In `docs/Doxyfile`:
   1. Append `aui.my_module/src` to the `STRIP_FROM_INC_PATH` variable.
   2. Append `aui.my_module` to the `INPUT` variable.
3. If `aui::my_module` has dependencies (excluding `aui::core`), handle them at `# add dependencies` in 
   `cmake/aui-config.cmake.in`.
4. Put the reference to the module in `docs/index.html`.
5. Create `aui.my_module` dir.
6. Copy & paste `CMakeLists.txt` from any small module (i.e. `aui::xml`) and configure it for your module.
7. Use `API_AUI_MY_MODULE` to export symbols from your module (it's created by the `aui_module` CMake function).