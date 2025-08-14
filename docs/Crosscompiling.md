# Crosscompiling (Android and iOS)

# Supported platforms

| `AUI_BUILD_FOR` value | Host platform       | Target platform | Project environment                        | Project dir                         |
|-----------------------|---------------------|-----------------|--------------------------------------------|-------------------------------------|
| `android`             | Windows Linux macOS | Android         | Gradle (can be opened with Android Studio) | `${CMAKE_BINARY_DIR}/app_project`   |
| `ios`                 | macOS               | iOS             | Xcode                                      | `${CMAKE_BINARY_DIR}/app_project`   |
