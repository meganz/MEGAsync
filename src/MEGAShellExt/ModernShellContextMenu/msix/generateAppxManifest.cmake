set(AppxManifestPath "${PackagingPath}/AppxManifest.xml")

execute_process(
    COMMAND makepri createconfig "/o" "/cf" "${PackagingPath}/priconfig.xml" "/dq" "en-US"
)

execute_process(
    COMMAND MakePri.exe new "/cf" "${PackagingPath}/priconfig.xml" "/pr" "${PackagingPath}"
    "/mn" "${AppxManifestPath}" "/o" "/of" "${PackagingPath}/resources.pri"
)

message(STATUS "${CMAKE_CURRENT_LIST_DIR}/AppxManifest.xml.in")
configure_file(${CMAKE_CURRENT_LIST_DIR}/AppxManifest.xml.in ${PackagingPath}/AppxManifest.xml @ONLY)

# Remove resource temp files
message(STATUS "Remove temporary files .xml and .pri")

# Remove priconfig,xml
execute_process(
        COMMAND ${CMAKE_COMMAND} -E rm "${PackagingPath}/priconfig.xml"
    )

# Create msix package
execute_process(
    COMMAND makeappx pack "/nv" "/o" "/d" "${PackagingPath}" "/p" "${MsixOutputPath}"
)

# Remove the rest of xml files
file(GLOB XML_FILES "${PackagingPath}/*.xml")
foreach(XML_FILE ${XML_FILES})
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E rm "${XML_FILE}"
    )
endforeach()

file(GLOB PRI_FILES "${PackagingPath}/*.pri")
foreach(PRI_FILE ${PRI_FILES})
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E rm "${PRI_FILE}"
    )
endforeach()