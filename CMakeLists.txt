cmake_minimum_required(VERSION 3.1...3.15)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(ViaductABY
  VERSION 1.0
  DESCRIPTION "Runtime library for the Viaduct compiler"
  LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(ABY CONFIG REQUIRED)
find_package(RELIC REQUIRED)
find_package(JNI REQUIRED)

if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    add_definitions("-DHAS_BOOST")
endif()

# copied from ENCRYPTO_utils/CMakeLists.txt 
#looks dirty, but since the relic library is poorly exported let's treat it as an external library
include_directories(${RELIC_INCLUDE_DIRS})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set(RELIC_LIB_DIR "/usr/local/lib/librelic_s.a")

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC")

add_library(RELIC::relic STATIC IMPORTED)
set_target_properties(RELIC::relic PROPERTIES
	IMPORTED_LOCATION ${RELIC_LIB_DIR}
)

add_subdirectory(src)
