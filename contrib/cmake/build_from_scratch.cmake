#[[
    Run this file from its current folder in script mode, with triplet as a defined parameter:

        cmake -DTRIPLET=<triplet> [-DEXTRA_ARGS=<-DVAR=VALUE>[;<-DVAR=VALUE...>] ] [-DTARGET=<target>[;<targets>...] ] -P build_from_scratch.cmake
		
	eg, for getting started on windows (comment out the pdfium lines in preferred-ports-megasync.txt first):
	
		cmake -DTRIPLET=x64-windows-mega -DEXTRA_ARGS="-DUSE_PDFIUM=0" -P build_from_scratch.cmake
		
    It will set up and build 3rdparty in a folder next to the SDK repo, and also
    build the SDK against those 3rd party libraries.
    pdfium must be supplied manually, or it can be commented out in preferred-ports-megasync.txt
    After 3rd party dependencies are downloaded, the build of the project itself may be controlled with
    an optional TARGET parameter. It accepts one target, or multiple targets separated by semicolons:
        -DTARGET=target1
        "-DTARGET=target1;target2"
    If provided, the script will build only the targets specified. If not, the script will build the 
    whole project in a manner equivalent to calling `make all`.

    For getting started debugging on Mac x86, these steps work.  Note that some dependencies for generating image/video thumbnails are skipped.
    Paths/versions etc need to be adjusted for your setup

         [edit preferred-ports to avoid building pdfium, ffmpeg, freeimage, etc]
         [to build and debug in xcode, add -DCMAKEGENERATOR=Xcode to the next line]
         /Applications/CMake.app/Contents/bin/cmake -DTRIPLET=x64-osx-mega -DEXTRA_ARGS="-DUSE_PDFIUM=0;-DUSE_FREEIMAGE=0;-DUSE_MEDIAINFO=0;-DUSE_FFMPEG=0;-DHAVE_FFMPEG=0;-DUSE_LIBRAW=0;-DFULLREQUIREMENTS=0;-DMEGA_QT_VERSION=5.12.12" -P build_from_scratch.cmake

         [if not using XCode, you can run direct from the command line like this]
         [if using XCode you still need to run macdeplyqt as below, but in subdirectory Debug]
         [cd to build directory, eg ../../build-x64-osx-mega-Debug]
         make MEGAsync
         /Users/Shared/Qt/5.12.12/clang_64/bin/macdeployqt ./MEGAsync.app -no-strip
         set DYLD_LIBRARY_PATH=/Users/Shared/Qt/5.12.12/clang_64/bin/;/Users/[YOU]/repos/megasync_build/3rdparty_desktop/vcpkg/installed/x64-osx-mega/debug/lib/
         ./MEGAsync.app/Contents/MacOS/MEGAsync
]]

function(usage_exit err_msg)
    message(FATAL_ERROR
"${err_msg}
Build from scratch helper script usage:
    cmake -DTRIPLET=<triplet> [-DTARGET=<target>[;<targets>...] ] -P build_from_scratch.cmake
Script must be run from its current folder")
endfunction()

set(_script_cwd ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT EXISTS "${_script_cwd}/build_from_scratch.cmake")
    usage_exit("Script was not run from its containing directory")
endif()

if(NOT TRIPLET)
    usage_exit("Triplet was not provided")
endif()

if(NOT EXTRA_ARGS)
    set(_extra_cmake_args "")
else()
    set(_extra_cmake_args ${EXTRA_ARGS})
endif()

if(NOT MEGA_QT_VERSION)
    set(MEGA_QT_VERSION 5.12.12)
endif()

set(_triplet ${TRIPLET})
set(_app_dir "${_script_cwd}/../..")
set(_sdk_dir "${_app_dir}/src/MEGASync/mega")

message(STATUS "Building for triplet ${_triplet} with APP  ${_app_dir}  SDK ${_sdk_dir} ")

if (VCPKG_PREBUILT)
    set (_3rdparty_dir "${VCPKG_PREBUILT}")
	set (_build3rdparty 0)
else()
    set (_3rdparty_dir "${_app_dir}/../3rdparty_desktop")
	set (_build3rdparty 1)
endif()

set(CMAKE_EXECUTE_PROCESS_COMMAND_ECHO STDOUT)

set(_cmake ${CMAKE_COMMAND})

function(execute_checked_command)
    execute_process(
        ${ARGV}
        RESULT_VARIABLE _result
        ERROR_VARIABLE _error
    )

    if(_result)
        message(FATAL_ERROR "Execute_process command had nonzero exit code ${_result} with error ${_error}")
    endif()
endfunction()

if (${_build3rdparty})

	file(MAKE_DIRECTORY ${_3rdparty_dir})

	# Configure and build the build3rdparty tool

	execute_checked_command(
		COMMAND ${_cmake}
			-S ${_sdk_dir}/contrib/cmake/build3rdParty
			-B ${_3rdparty_dir}
			-DCMAKE_BUILD_TYPE=Release
	)

	execute_checked_command(
		COMMAND ${_cmake}
			--build ${_3rdparty_dir}
			--config Release
	)

	# Use the prep tool to set up just our dependencies and no others

	if(WIN32)
		set(_3rdparty_tool_exe "${_3rdparty_dir}/Release/build3rdParty.exe")
		set(_3rdparty_vcpkg_dir "${_3rdparty_dir}/Release/vcpkg/")
	else()
		set(_3rdparty_tool_exe "${_3rdparty_dir}/build3rdParty")
		set(_3rdparty_vcpkg_dir "${_3rdparty_dir}/vcpkg/")
	endif()

	set(_3rdparty_tool_common_args
		--ports "${_script_cwd}/preferred-ports-megasync.txt"
		--triplet ${_triplet}
		--sdkroot ${_sdk_dir}
	)

	execute_checked_command(
		COMMAND ${_3rdparty_tool_exe}
			--setup
			--removeunusedports
			--nopkgconfig
			${_3rdparty_tool_common_args}
		WORKING_DIRECTORY ${_3rdparty_dir}
	)

	execute_checked_command(
		COMMAND ${_3rdparty_tool_exe}
			--build
			${_3rdparty_tool_common_args}
		WORKING_DIRECTORY ${_3rdparty_dir}
	)

endif(${_build3rdparty})

# Allows use of the VCPKG_XXXX variables defined in the triplet file
# We search our own custom triplet folder, and then the standard ones searched by vcpkg
foreach(_triplet_dir
    "${_script_cwd}/vcpkg_extra_triplets/"
    "${_sdk_dir}/contrib/cmake/vcpkg_extra_triplets/"
    "${_3rdparty_vcpkg_dir}/triplets/"
    "${_3rdparty_vcpkg_dir}/triplets/community"
)
    set(_triplet_file "${_triplet_dir}/${_triplet}.cmake")
    if(EXISTS ${_triplet_file})
        message(STATUS "Using triplet ${_triplet} at ${_triplet_file}")
        include(${_triplet_file})
        set(_triplet_file_found 1)
        break()
    endif()
endforeach()

if(NOT _triplet_file_found)
    message(FATAL_ERROR "Could not find triplet ${_triplet} in Mega vcpkg_extra_triplets nor in vcpkg triplet folders")
endif()

# Now set up to build this repo
# Logic between Windows and other platforms diverges slightly here:
# in the CMake paradigm, Visual Studio is what's called a multi-configuration generator,
# meaning we can do separate builds (Debug, Release) from one configuration.
# The default generators on *nix platforms (Ninja, Unix Makefiles) are single-config, so we
# configure once for each build type
# In a future extension, we may wish to have users provide the configs as a semicolon-separated list

set(_common_cmake_args
    "-DMega3rdPartyDir=${_3rdparty_dir}"
    "-DVCPKG_TRIPLET=${_triplet}"
    -DUSE_THIRDPARTY_FROM_VCPKG=1
    -DMEGA_QT_VERSION=${MEGA_QT_VERSION}
    -S ${_script_cwd}
)

if(TARGET)
    set(_cmake_target_args "--target" ${TARGET})
    #set(_cmake_target_args --target ${TARGET})
#elseif(TARGETS) # allow TARGETS as a synonym for TARGET :)
    #set(_cmake_target_args --target ${TARGETS})
endif()

set(_generator "")
if(WIN32)
    set(_generator "-G Visual Studio 16 2019")
else()
    if (CMAKEGENERATOR)
        set(_generator "-G ${CMAKEGENERATOR}")
    endif()
endif()

if(WIN32)
    if(_triplet MATCHES "staticdev$")
        list(APPEND _extra_cmake_args -DMEGA_LINK_DYNAMIC_CRT=0 -DUNCHECKED_ITERATORS=1)
    endif()

    if(VCPKG_TARGET_ARCHITECTURE STREQUAL "x86")
        set(_arch "Win32")
    else()
        set(_arch ${VCPKG_TARGET_ARCHITECTURE})
    endif()

    set(_toolset ${VCPKG_PLATFORM_TOOLSET})
    set(_build_dir "${_app_dir}/build-${_triplet}")

    file(MAKE_DIRECTORY ${_build_dir})

    execute_checked_command(
        COMMAND ${_cmake}
            ${_generator}
            -A ${_arch}
            # Could also pass -T VCPKG_PLATFORM_TOOLSET
            -B ${_build_dir}
            ${_common_cmake_args}
            ${_extra_cmake_args}
    )

    #foreach(_config "Release" "Debug")
	foreach(_config "Debug" "Release")
        execute_checked_command(
            COMMAND ${_cmake}
                --build ${_build_dir}
                ${_cmake_target_args}
                --config ${_config}
                --parallel 4
        )
    endforeach()
else()

    # Are we building for OSX?
    if (VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        # Determine the host's architecture.
        execute_process(
            COMMAND uname -m
            OUTPUT_VARIABLE HOST_ARCHITECTURE
            OUTPUT_STRIP_TRAILING_WHITESPACE)

        # Are we crosscompiling? Compare against the triplet value
        if (NOT HOST_ARCHITECTURE STREQUAL VCPKG_OSX_ARCHITECTURES)
            set(_toolchain_cross_compile_args
                "-DCMAKE_OSX_ARCHITECTURES=${VCPKG_OSX_ARCHITECTURES}")
                message(STATUS "Cross compiling for arch ${VCPKG_OSX_ARCHITECTURES} in ${HOST_ARCHITECTURE} system.")
        endif ()

        # Clean up after ourselves.
        unset(HOST_ARCHITECTURE)
    endif ()

    foreach(_config "Debug" "Release")
        set(_build_dir "${_app_dir}/build-${_triplet}-${_config}")
        file(MAKE_DIRECTORY ${_build_dir})

        execute_checked_command(
            COMMAND ${_cmake}
                ${_generator}
                -B ${_build_dir}
                "-DCMAKE_BUILD_TYPE=${_config}"
                ${_common_cmake_args}
                ${_extra_cmake_args}
                ${_toolchain_cross_compile_args}
        )

        execute_checked_command(
            COMMAND ${_cmake}
                --build ${_build_dir}
                ${_cmake_target_args}
                --parallel 4
        )
    endforeach()
endif()
