cmake_minimum_required(VERSION 3.6)
project(DADIU_CompetenceProject CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(DEBUG_MODE TRUE)
set(CMAKE_BUILD_TYPE Debug)

add_definitions(-DDEBUG)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

if(ARCH STREQUAL "x86")
    SET(PLATFORM_ARCH "x86")
else()
    SET(PLATFORM_ARCH "x64")
endif()

#########################################################
# LIBRARIES
#########################################################

IF(WIN32)
    SET(PLATFORM_PATHS ${CMAKE_SOURCE_DIR}/libraries/windows)
    SET(LIBRARY_EXTENSION "lib")
ELSEIF(APPLE)
    SET(PLATFORM_PATHS ${CMAKE_SOURCE_DIR}/libraries/macos)
    SET(LIBRARY_EXTENSION "dylib")
ELSEIF(UNIX)
    SET(PLATFORM_PATHS ${CMAKE_SOURCE_DIR}/libraries/linux)
    SET(LIBRARY_EXTENSION "so")
ENDIF()

SET(CROSS_PLATFORM_PATHS ${CMAKE_SOURCE_DIR}/libraries/multi)


IF(APPLE)
    message("//////////////////////////////////////////////////////////////")
    message("// MACOS has not been tested and will most likely not work ///")
    message("//////////////////////////////////////////////////////////////")

    INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
    FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
    MARK_AS_ADVANCED (OpenGL_LIBRARY)
    SET(EXTRA_LIBS ${OpenGL_LIBRARY})

ELSE (APPLE)
    find_package(OpenGL REQUIRED)
    include_directories(${OPENGL_INCLUDE_DIR})

    IF(UNIX)
        find_package(GLEW REQUIRED)
        include_directories(${GLEW_INCLUDE_DIR})
    ELSE(UNIX)
        find_package(glew REQUIRED) 
        include_directories(${GLEW_INCLUDE_DIR})

    ENDIF(UNIX)

    SET(EXTRA_LIBS ${EXTRA_LIBS} ${OPENGL_LIBRARY} ${GLEW_LIBRARY})
ENDIF (APPLE)

find_package(SDL2 REQUIRED)
find_package(SDL2_IMAGE REQUIRED)
find_package(SDL2_MIXER REQUIRED)

include_directories(${SDL2_INCLUDE_DIR}) #sdl2_image and mixer is placed in the same sdl2 include folder

##### OPENVR

if(WIN32)
    SET(OPENVR_LIBRARY "${PLATFORM_PATHS}/openvr/${PLATFORM_ARCH}/openvr_api.${LIBRARY_EXTENSION}")
else()
    SET(OPENVR_LIBRARY "${PLATFORM_PATHS}/openvr/${PLATFORM_ARCH}/libopenvr_api.${LIBRARY_EXTENSION}")
endif()

include_directories(${CROSS_PLATFORM_PATHS}/openvr/headers)

#####

include_directories(include)
include_directories(src)

SET(EXTRA_LIBS ${EXTRA_LIBS} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${OPENVR_LIBRARY})


## Debug Libraries ?

#########################################################
# EXECUTABLE
#########################################################

file(GLOB_RECURSE FILES
    "src/*.cpp"
    "src/*.hpp"
)

add_executable(Demo ${FILES})
target_link_libraries(Demo ${EXTRA_LIBS})

file(COPY data DESTINATION .)

IF (WIN32)
    SET(SDL2_DLL_FOLDER "${PLATFORM_PATHS}/SDL2/lib/${PLATFORM_ARCH}")

    file(GLOB DLLFileList
        "${SDL2_DLL_FOLDER}/*.dll"
    )

    set(DLLFileList ${DLLFileList} "${PLATFORM_PATHS}/openvr/${PLATFORM_ARCH}/openvr_api.dll")


    file(COPY ${DLLFileList} DESTINATION Debug)
    file(COPY ${DLLFileList} DESTINATION Release)
ELSE(WIN32)
    file(COPY data DESTINATION .)
ENDIF(WIN32)
