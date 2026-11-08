cmake_minimum_required(VERSION 3.21)
project(GoPieMenu VERSION 2.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
include(CTest)
option(GPM_BUILD_DESKTOP "Build the desktop application" ON)

if(NOT WIN32 AND NOT APPLE)
    message(FATAL_ERROR "GoPieMenu currently targets Windows and macOS only.")
endif()

if(APPLE)
    # Objective-C++ carries the AppKit bridging; it cannot be listed in project()
    # because only Apple toolchains provide an OBJCXX compiler.
    enable_language(OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 20)
    set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
    # Qt 6.9 supports macOS 12 and newer. Keep the bundle and the deployment target aligned.
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version")
    add_compile_options($<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc> $<$<COMPILE_LANGUAGE:OBJCXX>:-fblocks>)
endif()

find_package(Qt6 6.9 REQUIRED COMPONENTS Core Gui Network LinguistTools)

add_library(gpm_core STATIC
    src/core/ConfigValidation.cpp
    src/core/ConfigManager.cpp
    src/core/EditorSession.cpp
    src/core/InputState.cpp
    src/core/Shortcut.cpp
    src/core/TranslationManager.cpp
    src/platform/mac/MacKeyMap.cpp
)
target_include_directories(gpm_core PUBLIC src)
target_link_libraries(gpm_core PUBLIC Qt6::Core Qt6::Gui)
target_compile_definitions(gpm_core PUBLIC APP_VERSION="${PROJECT_VERSION}")
if(WIN32)
    target_compile_definitions(gpm_core PUBLIC UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
    target_link_libraries(gpm_core PUBLIC user32)
endif()
if(APPLE)
    target_sources(gpm_core PRIVATE src/platform/mac/MacApplications.mm)
    target_link_libraries(gpm_core PUBLIC "-framework AppKit" "-framework Foundation")
endif()
if(MSVC)
    target_compile_options(gpm_core PUBLIC /utf-8 /W4)
endif()

set(GPM_TRANSLATIONS
    resources/i18n/gopiemenu_zh_TW.ts
    resources/i18n/gopiemenu_zh_CN.ts
)
qt_add_lrelease(TS_FILES ${GPM_TRANSLATIONS} QM_FILES_OUTPUT_VARIABLE GPM_QM_FILES)
qt_add_resources(gpm_core "translations" PREFIX "/i18n"
    BASE "${CMAKE_CURRENT_BINARY_DIR}" FILES ${GPM_QM_FILES})
add_custom_target(update-translations
    COMMAND Qt6::lupdate src -no-obsolete -ts ${GPM_TRANSLATIONS}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Update translation catalogs from C++ and QML sources")

add_library(gpm_updates STATIC src/update/ReleaseInfo.cpp src/update/UpdateManager.cpp)
target_link_libraries(gpm_updates PUBLIC gpm_core Qt6::Network)
if(WIN32)
    target_link_libraries(gpm_updates PRIVATE shell32)
endif()

if(GPM_BUILD_DESKTOP)
    find_package(Qt6 6.9 REQUIRED COMPONENTS Widgets Quick Qml QuickControls2 Svg)
    add_library(gpm_ui STATIC
        src/ui/icons/IconService.cpp
        src/ui/icons/IconCatalog.cpp
        src/ui/rendering/MenuScene.cpp
        src/ui/PieMenuWidget.cpp
        src/ui/RuntimeMenuController.cpp
        src/ui/widgets/ListMenuWidget.cpp
        src/ui/widgets/ActionOverlay.cpp
        src/ui/quick/PiePreviewItem.cpp
        src/ui/quick/ColorWheelItem.cpp
        src/ui/SettingsWindow.cpp
        src/ui/InputRecorder.cpp
        src/ui/RunningApplications.cpp
    )
    target_include_directories(gpm_ui PUBLIC src)
    target_link_libraries(gpm_ui PUBLIC gpm_core gpm_updates Qt6::Widgets Qt6::Quick Qt6::Qml Qt6::QuickControls2 Qt6::Svg)
    if(WIN32)
        target_link_libraries(gpm_ui PRIVATE dwmapi)
    endif()
    if(APPLE)
        # Non-activating persistent menus and editor activation need AppKit directly.
        target_sources(gpm_ui PRIVATE src/platform/mac/MacWindow.mm)
        target_link_libraries(gpm_ui PUBLIC "-framework AppKit")
    endif()

    file(GLOB GPM_ICON_FILES CONFIGURE_DEPENDS resources/icon/*.svg)
    qt_add_resources(gpm_ui "icons" PREFIX "/icons" BASE "resources/icon" FILES ${GPM_ICON_FILES})
    file(GLOB GPM_QML_FILES CONFIGURE_DEPENDS src/ui/qml/*.qml src/ui/qml/*.js)
    qt_add_resources(gpm_ui "editor" PREFIX "/editor" BASE "src/ui/qml" FILES ${GPM_QML_FILES})

    add_library(gpm_actions STATIC
        src/core/ActionExecutor.cpp
        src/core/actions/LaunchAppAction.cpp
        src/core/actions/RunCommandAction.cpp
        src/core/actions/SendHotkeyAction.cpp
        src/core/actions/OpenFileAction.cpp
        src/core/actions/OpenURLAction.cpp
    )
    target_link_libraries(gpm_actions PUBLIC gpm_core)
    if(APPLE)
        # SendHotkeyAction posts Quartz events and checks Accessibility trust.
        target_link_libraries(gpm_actions PUBLIC "-framework ApplicationServices" "-framework CoreGraphics")
    endif()

    set(GPM_APP_SOURCES
        src/main.cpp
        src/core/IHookProvider.h
    )
    if(WIN32)
        list(APPEND GPM_APP_SOURCES src/core/HookManager.cpp resources/windows_icon.rc)
    endif()
    if(APPLE)
        list(APPEND GPM_APP_SOURCES
            src/core/HookManager.h
            src/core/HookManagerMac.cpp
            resources/macos/GoPieMenu.icns
        )
        set_source_files_properties(resources/macos/GoPieMenu.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    endif()

    add_executable(GoPieMenu ${GPM_APP_SOURCES})
    target_link_libraries(GoPieMenu PRIVATE gpm_ui gpm_actions)
    if(WIN32)
        set_target_properties(GoPieMenu PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
    if(APPLE)
        target_link_libraries(GoPieMenu PRIVATE "-framework ApplicationServices" "-framework CoreGraphics")
        set_target_properties(GoPieMenu PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_BUNDLE_NAME "GoPieMenu"
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.gopiemenu.app"
            MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
            MACOSX_BUNDLE_ICON_FILE "GoPieMenu.icns"
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/Info.plist.in"
        )
    endif()
    install(TARGETS GoPieMenu BUNDLE DESTINATION . RUNTIME DESTINATION bin)
    target_sources(gpm_ui PRIVATE src/ui/TrayManager.cpp resources/resources.qrc)
endif()

if(BUILD_TESTING)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/artifacts")
    find_package(Qt6 REQUIRED COMPONENTS Test)
    # The offscreen plugin keeps the QML tests deterministic on a headless macOS runner.
    if(APPLE)
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/artifacts/offscreen-screen.json" "{
  \"synchronousWindowSystemEvents\": false,
  \"windowFrameMargins\": true,
  \"screens\": [{
    \"name\": \"primary\",
    \"x\": 0,
    \"y\": 0,
    \"width\": 1920,
    \"height\": 1080,
    \"logicalDpi\": 96,
    \"logicalBaseDpi\": 96,
    \"dpr\": 1.0
  }]
}
")
        set(GPM_UI_TEST_ENVIRONMENT "QT_QPA_PLATFORM=offscreen:configfile=${CMAKE_CURRENT_BINARY_DIR}/artifacts/offscreen-screen.json;QT_QUICK_BACKEND=software")
    else()
        set(GPM_UI_TEST_ENVIRONMENT "QT_QUICK_BACKEND=software")
    endif()

    add_executable(gpm_core_tests tests/CoreTests.cpp)
    target_link_libraries(gpm_core_tests PRIVATE gpm_core Qt6::Test)
    add_test(NAME core COMMAND gpm_core_tests -o artifacts/core.txt,txt)
    add_executable(gpm_update_tests tests/UpdateTests.cpp)
    target_link_libraries(gpm_update_tests PRIVATE gpm_updates Qt6::Test)
    target_compile_definitions(gpm_update_tests PRIVATE GPM_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME updates COMMAND gpm_update_tests -o artifacts/updates.txt,txt)

    find_program(GPM_POWERSHELL NAMES pwsh powershell)
    if(GPM_POWERSHELL)
        add_test(NAME release-notes COMMAND "${GPM_POWERSHELL}" -NoProfile -File "${CMAKE_CURRENT_SOURCE_DIR}/tests/ReleaseNotesTests.ps1")
    endif()

    if(GPM_BUILD_DESKTOP)
        add_executable(gpm_ui_tests tests/UiTests.cpp)
        target_link_libraries(gpm_ui_tests PRIVATE gpm_ui gpm_actions Qt6::Test)
        foreach(scale 1 1.25 1.5 2)
            add_test(NAME ui-${scale} COMMAND gpm_ui_tests -o artifacts/ui-${scale}.txt,txt)
            set_tests_properties(ui-${scale} PROPERTIES ENVIRONMENT "${GPM_UI_TEST_ENVIRONMENT};QT_SCALE_FACTOR=${scale}" RUN_SERIAL TRUE)
        endforeach()
        if(WIN32)
            # Native input coverage needs a real Windows desktop and injects system input.
            add_executable(gpm_input_target WIN32 tests/InputTarget.cpp)
            target_link_libraries(gpm_input_target PRIVATE user32)
            add_executable(gpm_native_input_tests tests/NativeInputTests.cpp src/core/HookManager.cpp src/core/IHookProvider.h)
            target_link_libraries(gpm_native_input_tests PRIVATE gpm_ui gpm_actions Qt6::Test)
            add_dependencies(gpm_native_input_tests gpm_input_target)
            foreach(scale 1 1.25 1.5 2)
                add_test(NAME native-input-${scale} COMMAND gpm_native_input_tests -o artifacts/native-input-${scale}.txt,txt)
                set_tests_properties(native-input-${scale} PROPERTIES ENVIRONMENT "QT_SCALE_FACTOR=${scale}" RUN_SERIAL TRUE)
            endforeach()
        endif()
    endif()
endif()
