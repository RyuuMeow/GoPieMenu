cmake_minimum_required(VERSION 3.21)
project(GoPieMenu VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
include(CTest)
option(GPM_BUILD_DESKTOP "Build the Windows desktop application" ON)
find_package(Qt6 6.9 REQUIRED COMPONENTS Core Gui)

add_library(gpm_core STATIC
    src/core/ConfigValidation.cpp
    src/core/ConfigManager.cpp
    src/core/EditorSession.cpp
    src/core/InputState.cpp
    src/core/Shortcut.cpp
)
target_include_directories(gpm_core PUBLIC src)
target_link_libraries(gpm_core PUBLIC Qt6::Core Qt6::Gui)
target_compile_definitions(gpm_core PUBLIC UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN APP_VERSION="${PROJECT_VERSION}")
if(MSVC)
    target_compile_options(gpm_core PUBLIC /utf-8 /W4)
endif()
if(WIN32)
    target_link_libraries(gpm_core PUBLIC user32)
endif()

if(GPM_BUILD_DESKTOP)
    find_package(Qt6 6.9 REQUIRED COMPONENTS Widgets Quick Qml QuickControls2 Svg)
    add_library(gpm_ui STATIC
        src/ui/icons/IconService.cpp
        src/ui/icons/IconCatalog.cpp
        src/ui/rendering/MenuScene.cpp
        src/ui/PieMenuWidget.cpp
        src/ui/widgets/ListMenuWidget.cpp
        src/ui/quick/PiePreviewItem.cpp
        src/ui/SettingsWindow.cpp
        src/ui/InputRecorder.cpp
        src/ui/widgets/WindowPickerDialog.cpp
    )
    target_include_directories(gpm_ui PUBLIC src)
    target_link_libraries(gpm_ui PUBLIC gpm_core Qt6::Widgets Qt6::Quick Qt6::Qml Qt6::QuickControls2 Qt6::Svg)

    file(GLOB GPM_ICON_FILES CONFIGURE_DEPENDS resources/icon/*.svg)
    qt_add_resources(gpm_ui "icons" PREFIX "/icons" BASE "resources/icon" FILES ${GPM_ICON_FILES})
    file(GLOB GPM_QML_FILES CONFIGURE_DEPENDS src/ui/qml/*.qml src/ui/qml/*.js)
    qt_add_resources(gpm_ui "editor" PREFIX "/editor" BASE "src/ui/qml" FILES ${GPM_QML_FILES})

    add_executable(GoPieMenu WIN32
        src/main.cpp
        src/core/HookManager.cpp
        src/core/IHookProvider.h
        src/core/ActionExecutor.cpp
        src/core/actions/LaunchAppAction.cpp
        src/core/actions/RunCommandAction.cpp
        src/core/actions/SendHotkeyAction.cpp
        src/core/actions/OpenFileAction.cpp
        src/core/actions/OpenURLAction.cpp
        src/ui/TrayManager.cpp
        resources/resources.qrc
        resources/windows_icon.rc
    )
    target_link_libraries(GoPieMenu PRIVATE gpm_ui)
    install(TARGETS GoPieMenu DESTINATION bin)
endif()

if(BUILD_TESTING)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/artifacts")
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_executable(gpm_core_tests tests/CoreTests.cpp)
    target_link_libraries(gpm_core_tests PRIVATE gpm_core Qt6::Test)
    add_test(NAME core COMMAND gpm_core_tests -o artifacts/core.txt,txt)
    if(GPM_BUILD_DESKTOP)
        add_executable(gpm_ui_tests tests/UiTests.cpp)
        target_link_libraries(gpm_ui_tests PRIVATE gpm_ui Qt6::Test)
        foreach(scale 1 1.25 1.5 2)
            add_test(NAME ui-${scale} COMMAND gpm_ui_tests -o artifacts/ui-${scale}.txt,txt)
            set_tests_properties(ui-${scale} PROPERTIES ENVIRONMENT "QT_QUICK_BACKEND=software;QT_SCALE_FACTOR=${scale}" RUN_SERIAL TRUE)
        endforeach()
    endif()
endif()
