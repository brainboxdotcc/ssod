cmake_minimum_required (VERSION 3.16)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
find_package(MySQL)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(BOT_NAME "ssod")

project(${BOT_NAME})

aux_source_directory("src" coresrc)
aux_source_directory("src/commands" coresrc)
aux_source_directory("src/parser" coresrc)
aux_source_directory("src/js" coresrc)
add_executable(${BOT_NAME} ${coresrc})

target_precompile_headers(${BOT_NAME} INTERFACE "/usr/local/include/fmt/format.h")

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/gen/emoji.h)
	message(FATAL_ERROR "include/gen/emoji.h missing - You need to generate the list of emojis first from the admin website via `php artisan app:upload-sprites`")
endif()

string(ASCII 27 Esc)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set_target_properties(${BOT_NAME} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
find_package(DPP REQUIRED)

if(APPLE)
	set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
	find_package(OpenSSL REQUIRED)
else()
	find_package(OpenSSL REQUIRED)
endif()

set(CMAKE_CXX_FLAGS "-g -O2 -Wall -Wno-array-bounds -Wno-stringop-overflow -Wno-psabi -Wempty-body -Wignored-qualifiers -Wimplicit-fallthrough -Wmissing-field-initializers -Wsign-compare -Wtype-limits -Wuninitialized -Wshift-negative-value -fuse-ld=mold")
target_compile_definitions(${BOT_NAME} PUBLIC DPP_CORO=ON SPDLOG_FMT_EXTERNAL=ON _GNU_SOURCE)

target_include_directories(${BOT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${DPP_INCLUDE_DIR}
)

target_link_libraries(${BOT_NAME}
	fmt
	spdlog
	mysqlclient
	crypto
	ssl
	z
	pcre
	sentry
	${CMAKE_THREAD_LIBS_INIT}
	${DPP_LIBRARIES}
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	COMMAND "../sentry-symbols.sh"
)
