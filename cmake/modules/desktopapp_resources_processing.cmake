# This function allows to create aliases based on leafname, overwritten the main input file
#


function(process_resources_file INPUT_FILE)

    # Read the file
    file(READ "${INPUT_FILE}" XML_CONTENTS)

    # Perform the replacement
    # This regex captures:
    #   (1) full path
    #   (2) filename only (by capturing after last slash)
    string(REGEX REPLACE
        "<file>([^<]*[\\/])([^/<>\r\n]+)</file>"
        "<file alias=\"\\1\\2\">\\1\\2</file>"
        XML_CONTENTS_MODIFIED
        "${XML_CONTENTS}"
    )

    # Overwrite the original file
    file(WRITE "${INPUT_FILE}" "${XML_CONTENTS_MODIFIED}")

    message(STATUS "Resources files updated: ${INPUT_FILE}")

endfunction()
