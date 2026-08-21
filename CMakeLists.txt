cmake_minimum_required(VERSION 3.21)

project(ArxivKnowledgeManagement
        VERSION 1.1.0
        DESCRIPTION "Knowledge management for scientific publications from arXiv"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Network Sql Test)

qt_standard_project_setup()

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
