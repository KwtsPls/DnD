project(demogorgon)
cmake_minimum_required(VERSION 3.0)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-3.0)

include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})
add_definitions(${GTK_CFLAGS_OTHER})

add_executable(demogorgon main.c query-editor.c results-view.c ../utils/networking_utils.c)
target_link_libraries(demogorgon ${GTK_LIBRARIES})