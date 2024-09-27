function(get_clang_format)
    message(STATUS "Downloading .clang-format")

    set(FILE_URL "https://raw.githubusercontent.com/meganz/sdk/master/.clang-format")
    set(OUTPUT_FILE ${PROJECT_SOURCE_DIR}/.clang-format)
    file(DOWNLOAD ${FILE_URL} ${OUTPUT_FILE}
        SHOW_PROGRESS
        STATUS status
    )

    if(NOT status EQUAL 0)
        message(FATAL_ERROR "Download failed with status: ${status}")
    endif()
endfunction()
