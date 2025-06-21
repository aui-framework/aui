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

# Workflow

`master` branch contains code of the release candidate. Code of this branch must compile without errors and should pass
all tests on all supported platforms.
Each feature of the framework is created in a separate branch in the `feat/` folder. Pulling changes from your branch to
`master` is possible only if the merge with your branch does not generate merge conflicts and does not break the build
and tests.

Merges from `feat/` branch are allowed only to `develop` branch. `develop` is the branch were our actual development
work happens. When we consider `develop` branch stable enough, we merge `develop` and `master` and create a release.

# AI

We'd happily accept your AI-assisted work, as soon as you have complete responsibility and understading of the changes
you propose and they don't not violate someone's copyright. Please manually review and adjust AI generated
code/documentation. We won't merge changes if they contain obvious AI traces:

- hallucinations
- AI assistant-specific unnecessary statements: "I'm happy to help", "Please let me know if have questions", etc
- overexplanation, repetitive points, fluff writing

Please treat AI as a tool, not as a complete replacement of a software engineer.

# Guides

## Creating an AUI module

Suppose whe want to create a module called `aui::my_module`:

1. In the main `CMakeLists.txt`:
   1. Find the `# define all components` anchor, put `define_aui_component(my_module)`.
   2. Find the `# all components for exporting` anchor, put `my_module` to `AUI_ALL_COMPONENTS`.
2. In `docs/Doxyfile`:
   1. Append `aui.my_module/src` to the `STRIP_FROM_INC_PATH` variable.
   2. Append `aui.my_module` to the `INPUT` variable.
3. If `aui::my_module` has external dependencies, handle them at `# add dependencies` in 
   `cmake/aui-config.cmake.in`.
4. Put the reference to the module in `docs/index.html`.
5. Create `aui.my_module` dir.
6. Copy & paste `CMakeLists.txt` from any small module (i.e. `aui::xml`) and configure it for your module.
7. Use `API_AUI_MY_MODULE` to export symbols from your module (it's created by the `aui_module` CMake function).

## Documentation generation

The documentation generation process is primarily handled through Python scripts located in the `doxygen` directory.
These scripts invoke `doxygen` which is a de facto standard doc generation tool for C++, applying additional checks and
post-processing on `doxygen`'s output, most notably:

- Adds footer contents
- Removes unnecessary UI elements which can't be disabled through `Doxyfile`
- Adds view/edit source links
- Adjusts the navigation tree
- Fixes formatting and indentation in code snippets
- Generates table of contents for each page
- Generates `#` links for each header
- Generates full-fledged documentation pages from unit tests, i.e., in
  [aui.uitests/tests/UIDataBindingTest.cpp](https://github.com/aui-framework/aui/blob/96fea693675bfcacf60f5c138f8574c676160932/aui.uitests/tests/UIDataBindingTest.cpp#L28)

Key files and directories:

- `doxygen/extra`: extra files to put into output
- `doxygen/gen`: Python scripts whose stdout is inserted to the documentation. In example, there's a
  `doxygen/gen/aui_app_ICON.py`, which is invoked by `\@pythongen{aui_app_ICON}` from within Doxygen.
- `doxygen/intermediate`: temporary directory for immediate parts on the documentation, which is picked up by Doxygen.
  This directory is gitignored.
- `doxygen/Doxyfile`: main Doxygen configuration file
- `doxygen/modules`: Python modules
- `doxygen/out/html`: Generated documentation output
- `doxygen/patches`: Documentation portions that are used in patching


To generate documentation, run the following command from the root of AUI repository:

@note
These commands are not tested on Windows.

```bash
git clone https://github.com/aui-framework/aui
cd aui

# only for the first time
python3 -m pip install -r doxygen/requirements.txt 

# generate docs
python3 doxygen/docs.py
```

After the command is complete, the local copy of HTML doc pages are available for manual review in `doxygen/out/html`
directory. These pages are static so they can be deployed on a static site hosting provider.

```bash
xdg-open doxygen/out/html/index.html
```