cmake_minimum_required(VERSION 3.18)
project(tigerkin)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

find_library(YAMLCPP yaml-cpp)
find_library(PTHREAD pthread)
find_library(DLSYM dl)
message("***", ${YAMLCPP})

set(LIB_SRC
		src/mutex.cpp
		src/log.cpp
		src/util.cpp
		src/config.cpp
		src/thread.cpp
		src/coroutine.cpp
		src/scheduler.cpp
		src/iomanager.cpp
		src/timer.cpp
		src/hook.cpp
		src/fdmanager.cpp
		src/address.cpp
		src/socket.cpp
		src/bytearray.cpp
		src/stream.cpp
		src/tcp_server.cpp
		src/stream/socket_stream.cpp
		src/http/http.cpp
		src/http/http_request_parser.cpp
		src/http/http_response_parser.cpp
		src/http/http_parser.cpp
		src/http/http_session.cpp
		src/http/http_server.cpp	
	)

add_library(TIGERKIN SHARED ${LIB_SRC})


set(LIBS
		TIGERKIN
		${YAMLCPP}
		${PTHREAD}
		${DLSYM}
	)

add_executable(test_log tests/log_test.cpp)
add_dependencies(test_log TIGERKIN)
target_link_libraries(test_log TIGERKIN ${YAMLCPP} ${PTHREAD} ${DLSYM})

add_executable(test_util tests/util_test.cpp)
target_link_libraries(test_util ${LIBS})

add_executable(test_mutex tests/mutex_test.cpp)
target_link_libraries(test_mutex ${LIBS})

add_executable(test_config tests/config_test.cpp)
target_link_libraries(test_config ${LIBS})

add_executable(test_thread tests/thread_test.cpp)
target_link_libraries(test_thread ${LIBS})

add_executable(test_coroutine tests/coroutine_test.cpp)
target_link_libraries(test_coroutine ${LIBS})

add_executable(test_scheduler tests/scheduler_test.cpp)
target_link_libraries(test_scheduler ${LIBS})

add_executable(test_iomanager tests/iomanager_test.cpp)
target_link_libraries(test_iomanager ${LIBS})

add_executable(test_hook tests/hook_test.cpp)
target_link_libraries(test_hook ${LIBS})

add_executable(test_address tests/address_test.cpp)
target_link_libraries(test_address ${LIBS})

add_executable(test_socket tests/socket_test.cpp)
target_link_libraries(test_socket ${LIBS})

add_executable(test_bytearray tests/bytearray_test.cpp)
target_link_libraries(test_bytearray ${LIBS})

add_executable(test_http tests/http_test.cpp)
target_link_libraries(test_http ${LIBS})

add_executable(test_http_parser tests/http_parser_test.cpp)
target_link_libraries(test_http_parser ${LIBS})

add_executable(test_tcp_server tests/tcp_server_test.cpp)
target_link_libraries(test_tcp_server ${LIBS})


add_executable(test_http_server tests/http_server_test.cpp)
target_link_libraries(test_http_server ${LIBS})

add_executable(example_echo_server examples/echo_server.cpp)
target_link_libraries(example_echo_server ${LIBS})



set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATKIN{PROJECT_SOURCE_DIR}/lib)
