#----------------------------------------------------------------------------
# Project Name: LSCSIM
# Description: A package for LscSim framework
#----------------------------------------------------------------------------
# Setup the project and require CMake version above 3.5
#----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(LscSim_Release)

set(CMAKE_COLOR_MAKEFILE TRUE)

#----------------------------------------------------------------------------
# Add variables for the git revision
#----------------------------------------------------------------------------
# Get the current working branch
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE Master_GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

# Get the latest abbreviated commit hash of the working branch
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE Master_GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

#----------------------------------------------------------------------------
# Add essential subdirectories
#----------------------------------------------------------------------------
add_subdirectory(MCObjs EXCLUDE_FROM_ALL)

add_subdirectory(CupSim EXCLUDE_FROM_ALL)
add_dependencies(CupSimL MCObjs)

add_subdirectory(LscSim)   #CHANGE THIS
add_dependencies(LscSimL CupSimL)   #CHANGE THIS

#----------------------------------------------------------------------------
# Setup the main include directories
#----------------------------------------------------------------------------
include_directories(${PROJECT_SOURCE_DIR}/include)

#----------------------------------------------------------------------------
# Copy the data files to build directory
#----------------------------------------------------------------------------
mark_as_advanced(CMAKE_INSTALL_PREFIX)
