cmake_minimum_required(VERSION 3.13)

project(frozen-bubble CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT (CMAKE_BUILD_TYPE OR DEFINED ENV{CMAKE_BUILD_TYPE}))
    message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "${DEFAULT_BUILD_TYPE}")
endif()

if (("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"))
    add_compile_options(-Wall -Wextra -pedantic)
endif()

find_package(SDL2 REQUIRED)
find_package(SDL2_image QUIET)
find_package(SDL2_mixer QUIET)

find_package(iniparser REQUIRED static)

if(NOT SDL2_image_FOUND OR NOT SDL2_mixer_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND AND NOT SDL2_image_FOUND)
        pkg_check_modules(SDL2_image IMPORTED_TARGET SDL2_image)
    endif()
    if(PkgConfig_FOUND AND NOT SDL2_mixer_FOUND)
        pkg_check_modules(SDL2_mixer IMPORTED_TARGET SDL2_mixer)
    endif()
endif()

if(NOT SDL2_image_FOUND)
    message(FATAL_ERROR "SDL2_image not found")
endif()

if(NOT SDL2_mixer_FOUND)
    message(FATAL_ERROR "SDL2_mixer not found")
endif()

if(UNIX OR APPLE)
    add_subdirectory(server)
endif()

add_executable(frozen-bubble
    src/frozenbubble.cpp
    src/main.cpp
    src/menubutton.cpp
    src/mainmenu.cpp
    src/gamesettings.cpp
    src/audiomixer.cpp
    src/shaderstuff.cpp
)

set_property(TARGET frozen-bubble PROPERTY CXX_STANDARD 17)
set_property(TARGET frozen-bubble PROPERTY CXX_EXTENSIONS OFF)

target_compile_definitions(frozen-bubble
    PRIVATE DATA_DIR=\"${CMAKE_SOURCE_DIR}/share\"
)

target_include_directories(frozen-bubble
    SYSTEM
    PRIVATE ${SDL2_INCLUDE_DIRS}
    PRIVATE ${SDL2_IMAGE_INCLUDE_DIRS}
    PRIVATE ${SDL2_MIXER_INCLUDE_DIRS}
)

target_link_libraries(frozen-bubble PRIVATE
    SDL2::SDL2main
    SDL2::SDL2
    iniparser-static
    $<IF:$<BOOL:${PkgConfig_FOUND}>,$<TARGET_NAME_IF_EXISTS:PkgConfig::SDL2_image>, $<TARGET_NAME_IF_EXISTS:SDL2_image::SDL2_image>>
    $<IF:$<BOOL:${PkgConfig_FOUND}>,$<TARGET_NAME_IF_EXISTS:PkgConfig::SDL2_mixer>, $<TARGET_NAME_IF_EXISTS:SDL2_mixer::SDL2_mixer>>
)
