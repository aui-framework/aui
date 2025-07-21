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

We'd happily accept your AI-assisted work, as soon as you take complete responsibility thereof, and you have complete
understanding of the changes you propose. Make sure your AI-assisted work does not violate someone's copyright. Please
manually review and adjust AI-generated code/documentation. We won't merge changes if they contain obvious AI traces:

- hallucinations
- AI assistant-specific unnecessary statements: "I'm happy to help", "Please let me know if you have questions", etc
- overexplanation, repetitive points, fluff writing
- excessive usage of bullet points: nobody writes a page filled with bullet points entirely but AI

Please treat AI as a tool, not as a complete replacement of a software engineer.

# Understanding Story Point Values

Story points are designed as relative measurements of work complexity, deliberately decoupled from time-based metrics (
hours/days/weeks). They provide a general indication of effort required for task completion. It's important to note that
story point comparisons across different teams, departments, or organizations are inherently invalid.

For example, Team A's 100-point sprint delivery might represent more actual work than Team B's 200 points, as each team
develops their own story point scale. While this guide isn't meant to standardize story points globally, it offers
internal guidelines for consistent estimation.

Note: All stories include standard overhead for PR reviews, test verification, and deployment processes. This overhead
should be factored into estimates and can sometimes exceed the time needed for the primary task.

This section describes how the story points are determined for AUI tasks.

## 1 Point - Minimal Complexity

These represent the simplest possible changes. They involve single-line or minimal code modifications, such as text
corrections or content updates. Tasks typically involve simple configuration value modifications with no logic
alterations or risk of breakage. There are no external dependencies, and all necessary information is self-contained
within the ticket. The changes are covered by existing tests or basic smoke testing, with no test updates required.

## 2 Points - Low Complexity

These tasks represent approximately double the effort of a 1-point story. They may include minor third-party dependency
updates with API adjustments and basic method refactoring without output changes. New test creation might be necessary.
External dependencies are straightforward, such as Docker image whitelisting, and follow clear documentation or known
processes.

## 3 Points - Moderate Complexity

This is the first level where significant uncertainty enters the picture. These tasks require a notable but manageable
time investment and may involve multiple external dependencies. They often include major version updates of libraries
with breaking changes. Some cross-team communication is required, though clarifications can typically be resolved
through a single call or chat. This represents the upper range of typical story size.

## 5 Points - High Complexity

These stories are an extended version of 3-point complexity, involving a substantial workload with multiple unknowns to
resolve. They often include significant breaking changes across components and require extensive test modifications.
Multiple cross-team meetings are likely necessary, and broader stakeholder involvement is common.

## 8 Points - Very High Complexity

This represents the maximum reasonable scope for a single sprint. These stories involve major system changes or
refactoring, new integrations, or complete rewrites. They come with significant unknowns at the start and should be
considered for splitting into smaller stories. Examples include framework migrations or major library updates.
Requirements often have high uncertainty.

## 13+ Points - Excessive Complexity

Stories of this size exceed single sprint capacity and typically require multiple sprints to complete. They contain too
many unknowns and variables, carrying a high risk of underestimation. Such stories should be split into smaller
components or converted to epics. Examples include full API integrations or complete rebranding projects. These
large-scale changes are too complex to estimate accurately and manage effectively within a single sprint.

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