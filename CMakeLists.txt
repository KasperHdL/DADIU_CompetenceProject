cmake_minimum_required(VERSION 3.6)
project(DADIU_CompetenceProject CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(DEBUG_MODE TRUE)
set(CMAKE_BUILD_TYPE Debug)

add_definitions(-DDEBUG)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")


#########################################################
# LIBRARIES
#########################################################


IF(WIN32)
    SET(PLATFORM_PATHS ${CMAKE_SOURCE_DIR}/libraries/windows/)
ENDIF(WIN32)
IF(APPLE)
    SET(PLATFORM_PATHS ${CMAKE_SOURCE_DIR}/libraries/macos/)
ENDIF(APPLE)
IF(UNIX)
    SET(PLATFORM_PATHS ${CMAKE_SOURCE_DIR}/libraries/linux/)
ENDIF(UNIX)

SET(CROSS_PLATFORM_PATHS ${CMAKE_SOURCE_DIR}/libraries/multi/)


IF(APPLE)
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


include_directories(include)
include_directories(src)

SET(EXTRA_LIBS ${EXTRA_LIBS} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_IMAGE_LIBRARY})


IF(WIN32)
    if(DEBUG_MODE)
        SET(EXTRA_LIBS ${EXTRA_LIBS}) #DEBUG LIBS?
    else()
        SET(EXTRA_LIBS ${EXTRA_LIBS}) #RELEASE LIBS?
    endif()
ELSE(WIN32)
    SET(EXTRA_LIBS ${EXTRA_LIBS})
ENDIF(WIN32)

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
    if(ARCH STREQUAL "x86")
        SET(SDL2_DLL_FOLDER "${PLATFORM_PATHS}/SDL2/lib/x86")
    else()
        SET(SDL2_DLL_FOLDER "${PLATFORM_PATHS}/SDL2/lib/x64")
    endif()

    file(GLOB DLLFileList
        "${SDL2_DLL_FOLDER}/*.dll"
    )

    file(COPY ${DLLFileList} DESTINATION Debug)
    file(COPY ${DLLFileList} DESTINATION Release)
ELSE(WIN32)
    file(COPY data DESTINATION .)
ENDIF(WIN32)
