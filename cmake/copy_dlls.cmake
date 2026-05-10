# Копирует MinGW64 DLL (не системные) рядом с exe.
# Параметры: TARGET_FILE, TARGET_DIR

set(LDD_EXE "C:/msys64/mingw64/bin/ldd.exe")

execute_process(
    COMMAND "${LDD_EXE}" "${TARGET_FILE}"
    OUTPUT_VARIABLE LDD_OUT
    ERROR_QUIET
)

string(REPLACE "\n" ";" LDD_LINES "${LDD_OUT}")

foreach(LINE ${LDD_LINES})
    if(LINE MATCHES "=> (/[a-z]/[^ ]+\\.dll)" AND LINE MATCHES "msys64|mingw64")
        set(MSYS_PATH "${CMAKE_MATCH_1}")
        string(REGEX REPLACE "^/([a-z])/" "\\1:/" WIN_PATH "${MSYS_PATH}")
        if(EXISTS "${WIN_PATH}")
            file(COPY "${WIN_PATH}" DESTINATION "${TARGET_DIR}")
        endif()
    endif()
endforeach()
