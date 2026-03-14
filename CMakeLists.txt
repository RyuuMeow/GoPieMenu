cmake_minimum_required(VERSION 3.20)
project(GoPieMenu VERSION 1.0.0 LANGUAGES CXX)

# ── C++20 Standard ──────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Qt Configuration ────────────────────────────────────────────────────────
set(CMAKE_PREFIX_PATH "F:/Programming/Cxx/lib/Qt/6.9.0/msvc2022_64" CACHE PATH "Qt prefix path")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# ── Source Files ────────────────────────────────────────────────────────────
set(MODEL_SOURCES
    src/models/Types.h
    src/models/PieItem.h
    src/models/PieMenuConfig.h
    src/models/AppConfig.h
    src/models/StyleConfig.h
)

set(CORE_SOURCES
    src/core/IConfigProvider.h
    src/core/IHookProvider.h
    src/core/IActionHandler.h
    src/core/ConfigManager.h
    src/core/ConfigManager.cpp
    src/core/HookManager.h
    src/core/HookManager.cpp
    src/core/ActionExecutor.h
    src/core/ActionExecutor.cpp
    src/core/actions/LaunchAppAction.h
    src/core/actions/LaunchAppAction.cpp
    src/core/actions/RunCommandAction.h
    src/core/actions/RunCommandAction.cpp
    src/core/actions/SendHotkeyAction.h
    src/core/actions/SendHotkeyAction.cpp
    src/core/actions/OpenFileAction.h
    src/core/actions/OpenFileAction.cpp
    src/core/actions/OpenURLAction.h
    src/core/actions/OpenURLAction.cpp
)

set(UI_SOURCES
    src/ui/PieMenuWidget.h
    src/ui/PieMenuWidget.cpp
    src/ui/SettingsWindow.h
    src/ui/SettingsWindow.cpp
    src/ui/TrayManager.h
    src/ui/TrayManager.cpp
    src/ui/widgets/HotkeyRecorder.h
    src/ui/widgets/HotkeyRecorder.cpp
    src/ui/widgets/ColorPickerButton.h
    src/ui/widgets/ColorPickerButton.cpp
    src/ui/widgets/PiePreviewWidget.h
    src/ui/widgets/PiePreviewWidget.cpp
    src/ui/widgets/IconPickerDialog.h
    src/ui/widgets/IconPickerDialog.cpp
    src/ui/widgets/WindowPickerDialog.h
    src/ui/widgets/WindowPickerDialog.cpp
    src/ui/widgets/ListMenuWidget.h
    src/ui/widgets/ListMenuWidget.cpp
)

set(RESOURCE_FILES
    resources/resources.qrc
    resources/windows_icon.rc
)

# ── Executable ──────────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME} WIN32
    src/main.cpp
    ${MODEL_SOURCES}
    ${CORE_SOURCES}
    ${UI_SOURCES}
    ${RESOURCE_FILES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# ── Compiler Definitions ────────────────────────────────────────────────────
target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    APP_VERSION="${PROJECT_VERSION}"
)

# ── Install / Deploy ────────────────────────────────────────────────────────
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
