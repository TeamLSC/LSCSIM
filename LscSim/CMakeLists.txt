#----------------------------------------------------------------------------
# Project Name: LscSim
# Description: Simulation library & executable for LscSim package
#----------------------------------------------------------------------------
# Setup the project and require CMake version above 2.8
#----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(LscSim)

#----------------------------------------------------------------------------
# Add variables for the git revision
#----------------------------------------------------------------------------
# Get the current working branch
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE LscSim_GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

# Get the latest abbreviated commit hash of the working branch
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE LscSim_GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

#----------------------------------------------------------------------------
# Find ROOT and setup it
#----------------------------------------------------------------------------
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED)
include(${ROOT_USE_FILE})

#----------------------------------------------------------------------------
# Find Geant4 and setup it
#----------------------------------------------------------------------------
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
    find_package(Geant4 REQUIRED ui_all vis_all)
else()
    find_package(Geant4 REQUIRED)
endif()
include(${Geant4_USE_FILE})
#For backward compatibility
set(CMAKE_CXX_STANDARD 17)

#----------------------------------------------------------------------------
# Add variables for run scripts and macros
#----------------------------------------------------------------------------
set(LSCSIM_JOB_NAME "scratch" CACHE STRING "Representative name for this compilation")
if("${LSCSIM_JOB_NAME}" STREQUAL "")
    set(LSCSIM_JOB_NAME "scratch" FORCE)
endif()

set(LSCSIM_DECAY_SOURCE_NAME "U238 Th232 K40 U235 Pb210 Na22 I125 I126 Te121 Te121m Te123m Te125m Te127m H3" CACHE STRING "Target isotopes list for decay simulation (Seperated by space)")
set(LSCSIM_DECAY_RATE "4.9E-18 1.6E-18 1.8E-17 3.1E-17 1E-9 8.5E-9 1.4E-7 6.2E-7 4.2E-7 4.9E-8 6.7E-8 1.4E-7 7.6E-8 1.8E-9" CACHE STRING "Half life list for target isotopes (Seperated by space, Unit in Hz)")
set(LSCSIM_NCORE "100" CACHE STRING "Total number of jobs for simulation of each isotope in the list")
set(LSCSIM_SOURCE_CATEGORY "internal" CACHE STRING "Simulation category for decay simulation (internal or external-pmt)")
set(LSCSIM_DECAY_EVTNUM "1000" CACHE STRING "The number of events each job will simulate")

#----------------------------------------------------------------------------
# Set advanced properties to variables
#----------------------------------------------------------------------------
mark_as_advanced(LSCSIM_DECAY_SOURCE_NAME)
mark_as_advanced(LSCSIM_DECAY_RATE)
mark_as_advanced(LSCSIM_NCORE)
mark_as_advanced(LSCSIM_SOURCE_CATEGORY)
mark_as_advanced(LSCSIM_DECAY_EVTNUM)

#----------------------------------------------------------------------------
# Set variables for run scripts and macros
#----------------------------------------------------------------------------
if(NOT DEFINED ENV{SIMOUT_PATH})
    set(SIMOUT_PATH ${PROJECT_BINARY_DIR}/output)
else ()
    set(SIMOUT_PATH $ENV{SIMOUT_PATH})
endif()

set(LSCSIM_WORK_DIR ${CMAKE_BINARY_DIR})
set(RUN_SCRIPTS
    run_test.csh
    run_background.csh
    )
set(SIM_MACROS
    )
set(SESSION_MACROS
    init_vis.mac
    vis.mac
    gui.mac
    )

#----------------------------------------------------------------------------
# Configure the scripts and macros
#----------------------------------------------------------------------------
foreach(_SCRIPT ${RUN_SCRIPTS})
    configure_file (
        ${PROJECT_SOURCE_DIR}/run/${_SCRIPT}.in
        ${PROJECT_BINARY_DIR}/run/${_SCRIPT}
        )
endforeach()

foreach(_MACRO ${SIM_MACROS})
    configure_file (
        ${PROJECT_SOURCE_DIR}/mac/${_MACRO}.in
        ${PROJECT_BINARY_DIR}/mac/${_MACRO}
        )
endforeach()
foreach(_MACRO ${SESSION_MACROS})
    configure_file (
        ${PROJECT_SOURCE_DIR}/${_MACRO}
        ${PROJECT_BINARY_DIR}/${_MACRO}
        )
endforeach()

#----------------------------------------------------------------------------
# Copy the various files to build directory
#----------------------------------------------------------------------------
file(COPY ${PROJECT_SOURCE_DIR}/mac ${PROJECT_SOURCE_DIR}/run ${PROJECT_SOURCE_DIR}/data DESTINATION ${PROJECT_BINARY_DIR}
    PATTERN *.in EXCLUDE)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/output)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/output/log)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/output/mac)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/output/root)
#file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/output/${LSCSIM_JOB_NAME})
#file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/output/${LSCSIM_JOB_NAME}/log)
#file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/output/${LSCSIM_JOB_NAME}/mac)
#file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/output/${LSCSIM_JOB_NAME}/root)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#----------------------------------------------------------------------------
file(GLOB LscSim_EXEC_SOURCE ${PROJECT_SOURCE_DIR}/test/lscsim.cc)
file(GLOB LscSim_GitRev_SOURCE ${PROJECT_SOURCE_DIR}/src/LscSimGitRevision.cc)
file(GLOB LscSim_LIB_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB LscSim_LIB_HEADERS ${PROJECT_SOURCE_DIR}/LscSim/*.hh)

#----------------------------------------------------------------------------
# Add definitions for git commit hash
#----------------------------------------------------------------------------
set_source_files_properties(${LscSim_EXEC_SOURCE} PROPERTIES
    COMPILE_DEFINITIONS "LscSim_GIT_BRANCH=${LscSim_GIT_BRANCH};LscSim_GIT_COMMIT_HASH=${LscSim_GIT_COMMIT_HASH}" )
set_source_files_properties(${LscSim_GitRev_SOURCE} PROPERTIES
    COMPILE_DEFINITIONS "LscSim_GIT_BRANCH=${LscSim_GIT_BRANCH};LscSim_GIT_COMMIT_HASH=${LscSim_GIT_COMMIT_HASH}" )

#----------------------------------------------------------------------------
# Add the executables and libraries and link it to the Geant4 framework and
# MCObjs library
#----------------------------------------------------------------------------
add_library(LscSimL SHARED ${LscSim_LIB_SOURCES})
target_link_libraries(LscSimL ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})
add_executable(lscsim ${LscSim_EXEC_SOURCE})
target_link_libraries(lscsim LscSimL ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})

#----------------------------------------------------------------------------
# Check dependencies for this project and set include directories and libraries
#----------------------------------------------------------------------------
if(NOT DEFINED CupSim)
    message(SEND_ERROR "This project cannot be built without CupSim. Please check about this.")
else()
    include_directories(${CupSim_INCLUDE_DIR})
    target_include_directories(lscsim PUBLIC ${CupSim_INCLUDE_DIR})
    target_include_directories(LscSimL PUBLIC ${CupSim_INCLUDE_DIR})
    target_link_libraries(lscsim CupSimL)
endif()

if(NOT DEFINED MCObjs)
    message(SEND_ERROR "This project cannot be built without MCObjs. Please check about this.")
else()
    include_directories(${MCObjs_INCLUDE_DIR})
    target_include_directories(lscsim PUBLIC ${MCObjs_INCLUDE_DIR})
    target_include_directories(LscSimL PUBLIC ${MCObjs_INCLUDE_DIR})
    target_link_libraries(lscsim MCObjs)
endif()

#----------------------------------------------------------------------------
# Setup global and local include directories for this project
#----------------------------------------------------------------------------
include_directories(${PROJECT_SOURCE_DIR} ${Geant4_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS})
target_include_directories(lscsim PUBLIC ${PROJECT_SOURCE_DIR} ${Geant4_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS})
target_include_directories(LscSimL PUBLIC ${PROJECT_SOURCE_DIR} ${Geant4_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS})
if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(LscSimL PUBLIC -fdiagnostics-color=always)
    target_compile_options(lscsim PUBLIC -fdiagnostics-color=always)
endif()

#----------------------------------------------------------------------------
# Expose this public includes and library to other subprojects through cache
# variable.
#----------------------------------------------------------------------------
set(${PROJECT_NAME}_INCLUDE_DIR ${PROJECT_SOURCE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
set(${PROJECT_NAME}_LIBRARY ${PROJECT_BINARY_DIR}/lib${PROJECT_NAME}.so
    CACHE INTERNAL "${PROJECT_NAME}: Path to library" FORCE)
set(${PROJECT_NAME} "Exist!"
    CACHE INTERNAL "${PROJECT_NAME}: Variable for checking dependencies" FORCE)
