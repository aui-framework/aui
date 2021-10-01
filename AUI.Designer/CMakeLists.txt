cmake_minimum_required(VERSION 3.10)

if(NOT ANDROID)
    if (TARGET aui.views)
        add_subdirectory(registrator)

        aui_executable(aui.designer)
        aui_compile_assets(aui.designer)
        target_link_libraries(aui.designer PRIVATE aui.core)
        target_link_libraries(aui.designer PRIVATE aui.data)
        target_link_libraries(aui.designer PRIVATE aui.views)
        target_link_libraries(aui.designer PRIVATE aui.crypt)
        target_link_libraries(aui.designer PRIVATE aui.designer.Registrator)
    endif()
else()
    message("Not building aui.designer because of not native platform. Please specify AUI_SDK_BIN with containing folder
             of your aui.toolbox.")
endif()