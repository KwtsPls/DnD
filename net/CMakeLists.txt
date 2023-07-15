project(net)
cmake_minimum_required(VERSION 3.0)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-3.0)

include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})
add_definitions(${GTK_CFLAGS_OTHER})

add_executable(net main.c ../utils/networking_utils.c ../rdbms/compiler/tokenizer.c ../rdbms/compiler/parser.c ../rdbms/compiler/semantic.c ../rdbms/disk/block.c ../rdbms/memory/buffer_manager.c ../rdbms/memory/priority_queue.c ../rdbms/table/databox.c ../rdbms/table/record.c ../rdbms/table/table.c ../rdbms/table/result_set.c ../rdbms/db_files/heapfile.c ../rdbms/db_files/result_file.c ../rdbms/db_files/db_file.c ../rdbms/joins/bnl.c)
target_link_libraries(net ${GTK_LIBRARIES})