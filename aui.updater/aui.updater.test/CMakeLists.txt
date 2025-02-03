aui_executable(aui.updater.test)
aui_link(aui.updater.test PUBLIC aui::core aui::updater)
add_dependencies(Tests aui.updater.test)