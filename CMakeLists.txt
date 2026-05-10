cmake_minimum_required(VERSION 3.20)
project(inventory C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

pkg_check_modules(XLSXWRITER xlsxwriter)

set(SOURCES
    src/main.c
    src/list.c
    src/sort.c
    src/fileio.c
    src/export.c
    src/ui/product_object.c
    src/ui/app.c
    src/ui/main_window.c
    src/ui/product_dialog.c
    src/ui/filter_window.c
)

add_executable(inventory ${SOURCES})

target_include_directories(inventory PRIVATE src src/ui ${GTK4_INCLUDE_DIRS})
target_compile_options(inventory PRIVATE ${GTK4_CFLAGS_OTHER} -Wall -Wextra -Wpedantic)
target_link_libraries(inventory ${GTK4_LIBRARIES})
target_link_directories(inventory PRIVATE ${GTK4_LIBRARY_DIRS})

if(XLSXWRITER_FOUND)
    target_compile_definitions(inventory PRIVATE HAVE_XLSXWRITER)
    target_include_directories(inventory PRIVATE ${XLSXWRITER_INCLUDE_DIRS})
    target_link_libraries(inventory ${XLSXWRITER_LIBRARIES})
    target_link_directories(inventory PRIVATE ${XLSXWRITER_LIBRARY_DIRS})
endif()

if(WIN32)
    add_custom_command(TARGET inventory POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            -DTARGET_FILE="$<TARGET_FILE:inventory>"
            -DTARGET_DIR="$<TARGET_FILE_DIR:inventory>"
            -P "${CMAKE_SOURCE_DIR}/cmake/copy_dlls.cmake"
        COMMENT "Copying GTK4 runtime DLLs to build directory"
    )
endif()

enable_testing()

add_executable(test_runner
    src/list.c
    src/sort.c
    tests/test_list.c
    tests/test_sort.c
    tests/test_runner.c
)
target_include_directories(test_runner PRIVATE src)
target_compile_options(test_runner PRIVATE -Wall -Wextra)
add_test(NAME unit_tests COMMAND test_runner)
