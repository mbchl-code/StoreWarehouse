cmake_minimum_required(VERSION 3.20)
project(inventory C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

pkg_check_modules(XLSXWRITER REQUIRED xlsxwriter)

set(SOURCES
    src/main.c
    src/list.c
    src/sort.c
    src/filter.c
    src/fileio.c
    src/export.c
    src/ui/product_object.c
    src/ui/app.c
    src/ui/main_window.c
    src/ui/product_dialog.c
    src/ui/filter_window.c
)

if(WIN32)
    list(APPEND SOURCES resources/app.rc)
endif()

add_executable(inventory ${SOURCES})

target_include_directories(inventory PRIVATE src src/ui ${GTK4_INCLUDE_DIRS})
target_compile_options(inventory PRIVATE ${GTK4_CFLAGS_OTHER} -Wall -Wextra -Wpedantic)
target_link_libraries(inventory ${GTK4_LIBRARIES})
target_link_directories(inventory PRIVATE ${GTK4_LIBRARY_DIRS})

target_include_directories(inventory PRIVATE ${XLSXWRITER_INCLUDE_DIRS})
target_link_libraries(inventory ${XLSXWRITER_LIBRARIES})
target_link_directories(inventory PRIVATE ${XLSXWRITER_LIBRARY_DIRS})

if(WIN32)
    find_program(PYTHON_EXE NAMES python python3 HINTS "C:/Python313" "C:/Python312" "C:/Python311")
    if(PYTHON_EXE)
        # Post-build: copy DLLs next to exe for running from CLion
        add_custom_command(TARGET inventory POST_BUILD
            COMMAND "${PYTHON_EXE}"
                "${CMAKE_SOURCE_DIR}/cmake/copy_dlls.py"
                "$<TARGET_FILE:inventory>"
                "$<TARGET_FILE_DIR:inventory>"
            COMMENT "Copying GTK4 runtime DLLs to build directory"
            VERBATIM
        )

        # Custom target: collect full distribution into dist/
        set(DIST_DIR "${CMAKE_SOURCE_DIR}/dist")
        add_custom_target(collect_dist
            COMMAND "${PYTHON_EXE}"
                "${CMAKE_SOURCE_DIR}/cmake/collect_dist.py"
                "$<TARGET_FILE:inventory>"
                "${DIST_DIR}"
            DEPENDS inventory
            COMMENT "Collecting distribution package into dist/"
            VERBATIM
        )
    endif()

    # CPack / NSIS installer — run: cmake --build . --target package
    set(DIST_DIR "${CMAKE_SOURCE_DIR}/dist")
    install(DIRECTORY "${DIST_DIR}/" DESTINATION ".")

    set(CPACK_GENERATOR "NSIS")
    set(CPACK_PACKAGE_NAME "Inventory")
    set(CPACK_PACKAGE_VERSION "1.0.0")
    set(CPACK_PACKAGE_VENDOR "Student Project")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Система управления товарным инвентарём")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Inventory")
    set(CPACK_NSIS_DISPLAY_NAME "Инвентарь магазина")
    set(CPACK_NSIS_PACKAGE_NAME "Инвентарь магазина")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/resources/icon.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/resources/icon.ico")
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Инвентарь.lnk' '$INSTDIR\\\\inventory.exe'")
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$SMPROGRAMS\\\\$START_MENU\\\\Инвентарь.lnk'")
    include(CPack)
endif()

enable_testing()

add_executable(test_gtk tests/test_gtk.c)
target_include_directories(test_gtk PRIVATE ${GTK4_INCLUDE_DIRS})
target_compile_options(test_gtk PRIVATE ${GTK4_CFLAGS_OTHER})
target_link_libraries(test_gtk ${GTK4_LIBRARIES})
target_link_directories(test_gtk PRIVATE ${GTK4_LIBRARY_DIRS})
if(WIN32 AND PYTHON_EXE)
    add_custom_command(TARGET test_gtk POST_BUILD
        COMMAND "${PYTHON_EXE}"
            "${CMAKE_SOURCE_DIR}/cmake/copy_dlls.py"
            "$<TARGET_FILE:test_gtk>"
            "$<TARGET_FILE_DIR:test_gtk>"
        VERBATIM
    )
endif()

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
