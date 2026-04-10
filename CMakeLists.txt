cmake_minimum_required(VERSION 3.16)

project(VNEngine VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================
# Find Qt
# =========================
find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

# =========================
# Create executable
# =========================
qt_add_executable(appVNEngine
    main.cpp
)

# =========================
# IMPORTANT: Include paths (FIXES Node.h issue)
# =========================
target_include_directories(appVNEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Data
)

# =========================
# Source files
# =========================
target_sources(appVNEngine PRIVATE

    Core/ChoiceModel.cpp
    Core/DialogueManager.cpp
)

# =========================
# Header files
# =========================
target_sources(appVNEngine PRIVATE

    Core/ChoiceModel.h
    Core/DialogueManager.h
    Data/Node.h
)

# =========================
# QML Module
# =========================
qt_add_qml_module(appVNEngine
    URI VNEngine
    VERSION 1.0

    QML_FILES
        Main.qml

    RESOURCES
        Data/story.json
)

# =========================
# Link Qt
# =========================
target_link_libraries(appVNEngine
    PRIVATE Qt6::Quick
)

# =========================
# App properties
# =========================
set_target_properties(appVNEngine PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)