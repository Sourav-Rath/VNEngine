cmake_minimum_required(VERSION 3.16)

project(VNEngine VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

# =========================
# EXECUTABLE
# =========================
qt_add_executable(appVNEngine
    main.cpp

    Core/DialogueManager.cpp
    Core/DialogueManager.h

    Core/ChoiceModel.cpp
    Core/ChoiceModel.h


    Data/Node.h
)

# =========================
# INCLUDE PATHS
# =========================
target_include_directories(appVNEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Data
)

# =========================
# QML MODULE + RESOURCES
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
# LINK
# =========================
target_link_libraries(appVNEngine PRIVATE Qt6::Quick)