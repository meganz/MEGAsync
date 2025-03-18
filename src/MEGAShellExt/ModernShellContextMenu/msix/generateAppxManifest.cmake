set(AppxManifestPath "${CMAKE_CURRENT_LIST_DIR}/AppxManifest.xml")
set(MSIPath "${OutputPath}/msi")
set(MSIName "MEGAShellExt.msix")
set(AssetsFolder "assets")

if(NOT EXISTS "${MSIPath}")
    file(MAKE_DIRECTORY ${MSIPath})
endif()

# Copy the assets folder to the build dir (and remove the previous one, in case it is outdated)
file(REMOVE_RECURSE "${MSIPath}/${AssetsFolder}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/${AssetsFolder}" DESTINATION "${MSIPath}/")
message(STATUS "Assets folder copied")

execute_process(
    COMMAND makepri createconfig "/o" "/cf" "${MSIPath}/priconfig.xml" "/dq" "en-US"
)

execute_process(
    COMMAND MakePri.exe new "/cf" "${MSIPath}/priconfig.xml" "/pr" "${MSIPath}"
    "/mn" "${MSIPath}/" "/o" "/of" "${MSIPath}/resources.pri"
)

set(MEGA_DESKTOP_APP_CERTIFICATE_PUBLISHER $ENV{MEGA_CERTIFICATE_PUBLISHER})

message (STATUS "Using certificate publisher: $ENV{MEGA_CERTIFICATE_PUBLISHER}")

configure_file(${CMAKE_CURRENT_LIST_DIR}/AppxManifest.xml.in ${MSIPath}/AppxManifest.xml @ONLY)

# Create msix package in the root build dir
execute_process(
    COMMAND makeappx pack "/nv" "/o" "/d" "${MSIPath}" "/p" "${OutputPath}/${MSIName}"
)