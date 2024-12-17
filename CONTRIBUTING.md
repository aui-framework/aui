# Contributing

You are always free to contribute to this project and add yourself to the authors list :)

If you have issues following this guide, we'd happily help you if you open a pull request. We respect your time spent
on contributing to our project and we'll your best to make your contribution happen.

## Workflow
`master` branch contains code of the release candidate. Code of this branch must compile without errors and should pass
all tests on all supported platforms.
Each feature of the framework is created in a separate branch in the `feat/` folder. Pulling changes from your branch to
`master` is possible only if the merge with your branch does not generate merge conflicts and does not break the build
and tests.

Merges from `feat/` branch are allowed only to `develop` branch. `develop` is the branch were our actual development
work happen. When we consider `develop` branch stable enough, we merge `develop` and `master` and create a release.

## Code style

We have `.clang-format` in root directory which describes our code style. Also, this document is used in code style
check pipeline.

Visual Studio Code with clang plugin should pickup this file by itself.

In CLion, click "4 spaces" button in bottom right corner (near `UTF-8`) and select "Enable ClangFormat" ([source](https://www.jetbrains.com/help/clion/clangformat-as-alternative-formatter.html#clion-support)).

We recommend to use this `.clang-format` in your projects as well, so you wont struggle with indentation while building
UIs.
