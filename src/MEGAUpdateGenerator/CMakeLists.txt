#
# MEGA Update Generator
# Binary to generate desktop app update files, keys and certificates.
#

add_executable(MEGAUpdateGenerator)

set(UPDATE_GENERATOR_SOURCES
    MEGAUpdateGenerator.cpp
)

target_sources(MEGAUpdateGenerator
    PRIVATE
    ${UPDATE_GENERATOR_SOURCES}
)

# Load and link needed libraries for the CHATlib target
find_package(cryptopp CONFIG REQUIRED)
target_link_libraries(MEGAUpdateGenerator
    PRIVATE
    cryptopp::cryptopp
    MEGA::SDKlib
)

