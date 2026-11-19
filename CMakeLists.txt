cmake_minimum_required(VERSION 3.16)
project(W_AMR_Sim VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

qt_standard_project_setup()

set(PROJECT_SOURCES
    src/main.cpp
)

set(PROJECT_HEADERS
)

add_executable(w_amr_sim
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

target_include_directories(w_amr_sim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SQLITE3_INCLUDE_DIRS}
)

target_link_libraries(w_amr_sim PRIVATE
    Qt6::Widgets
    ${SQLITE3_LIBRARIES}
)