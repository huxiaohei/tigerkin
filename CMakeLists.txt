cmake_minimum_required(VERSION 3.18)
project(tigerkin)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

find_library(YAMLCPP yaml-cpp)
find_library(PTHREAD pthread)
message("***", ${YAMLCPP})

set(LIB_SRC
		src/mutex.cpp
		src/log.cpp
		src/util.cpp
		src/config.cpp
		src/thread.cpp
		src/coroutine.cpp
	)

add_library(TIGERKIN SHARED ${LIB_SRC})


set(LIBS
		TIGERKIN
		${YAMLCPP}
		${PTHREAD}
	)

add_executable(test_log tests/log_test.cpp)
add_dependencies(test_log TIGERKIN)
target_link_libraries(test_log TIGERKIN ${YAMLCPP} ${PTHREAD})

add_executable(test_util tests/util_test.cpp)
target_link_libraries(test_util ${LIBS})

add_executable(test_mutex tests/mutex_test.cpp)
target_link_libraries(test_mutex ${LIBS})

add_executable(test_config tests/config_test.cpp)
target_link_libraries(test_config ${LIBS})

add_executable(test_thread tests/thread_test.cpp)
target_link_libraries(test_thread ${LIBS})

add_executable(test_coroution tests/coroutine_test.cpp)
target_link_libraries(test_coroution ${LIBS})


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATKIN{PROJECT_SOURCE_DIR}/lib)
