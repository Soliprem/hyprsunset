cmake_minimum_required(VERSION 3.12)

file(READ "${CMAKE_SOURCE_DIR}/VERSION" VER_RAW)
string(STRIP ${VER_RAW} VERSION)

project(
  hyprsunset
  DESCRIPTION "An application to enable a blue-light filter on Hyprland"
  VERSION ${VERSION})

set(CMAKE_MESSAGE_LOG_LEVEL "STATUS")

add_compile_definitions(HYPRSUNSET_VERSION="${VERSION}")

message(STATUS "Configuring hyprsunset!")

configure_file(systemd/hyprsunset.service.in systemd/hyprsunset.service @ONLY)

# Get git info hash and branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
  COMMAND git rev-parse HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
  COMMAND bash -c "git show ${GIT_COMMIT_HASH} | head -n 5 | tail -n 1"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_MESSAGE
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
  COMMAND bash -c "git diff-index --quiet HEAD -- || echo \"dirty\""
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_DIRTY
  OUTPUT_STRIP_TRAILING_WHITESPACE)
#

include_directories(.)
set(CMAKE_CXX_STANDARD 26)
add_compile_options(
  -Wall
  -Wextra
  -Wno-unused-parameter
  -Wno-unused-value
  -Wno-missing-field-initializers
  -Wno-narrowing
  -Wno-pointer-arith)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(
  deps
  REQUIRED
  IMPORTED_TARGET
  wayland-client
  wayland-protocols
  hyprutils>=0.2.3
  hyprlang
  hyprwayland-scanner>=0.4.0)

file(GLOB_RECURSE SRCFILES "src/*.cpp")

add_executable(hyprsunset ${SRCFILES})

pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)
message(STATUS "Found wayland-protocols at ${WAYLAND_PROTOCOLS_DIR}")
pkg_get_variable(WAYLAND_SCANNER_DIR wayland-scanner pkgdatadir)
message(STATUS "Found wayland-scanner at ${WAYLAND_SCANNER_DIR}")

function(protocolnew protoPath protoName external)
  if(external)
    set(path ${protoPath})
  else()
    set(path ${WAYLAND_PROTOCOLS_DIR}/${protoPath})
  endif()
  add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/protocols/${protoName}.cpp
           ${CMAKE_SOURCE_DIR}/protocols/${protoName}.hpp
    COMMAND hyprwayland-scanner --client ${path}/${protoName}.xml
            ${CMAKE_SOURCE_DIR}/protocols/
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  target_sources(hyprsunset PRIVATE protocols/${protoName}.cpp
                                    protocols/${protoName}.hpp)
endfunction()
function(protocolWayland)
  add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/protocols/wayland.cpp
           ${CMAKE_SOURCE_DIR}/protocols/wayland.hpp
    COMMAND hyprwayland-scanner --wayland-enums --client
            ${WAYLAND_SCANNER_DIR}/wayland.xml ${CMAKE_SOURCE_DIR}/protocols/
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  target_sources(hyprsunset PRIVATE protocols/wayland.cpp protocols/wayland.hpp)
endfunction()

protocolwayland()

pkg_check_modules(hyprland_protocols_dep REQUIRED IMPORTED_TARGET hyprland-protocols>=0.4.0)
pkg_get_variable(HYPRLAND_PROTOCOLS hyprland-protocols pkgdatadir)
message(STATUS "hyprland-protocols dependency set to ${HYPRLAND_PROTOCOLS}")

protocolnew("${HYPRLAND_PROTOCOLS}/protocols" "hyprland-ctm-control-v1" true)

target_compile_definitions(hyprsunset
                           PRIVATE "-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")
target_compile_definitions(hyprsunset PRIVATE "-DGIT_BRANCH=\"${GIT_BRANCH}\"")
target_compile_definitions(
  hyprsunset PRIVATE "-DGIT_COMMIT_MESSAGE=\"${GIT_COMMIT_MESSAGE}\"")
target_compile_definitions(hyprsunset PRIVATE "-DGIT_DIRTY=\"${GIT_DIRTY}\"")

target_link_libraries(hyprsunset rt)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_link_libraries(hyprsunset PkgConfig::deps)

target_link_libraries(hyprsunset pthread ${CMAKE_THREAD_LIBS_INIT}
                      wayland-cursor)

if(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES DEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -no-pie -fno-builtin")
  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} -pg -no-pie -fno-builtin")
  set(CMAKE_SHARED_LINKER_FLAGS
      "${CMAKE_SHARED_LINKER_FLAGS} -pg -no-pie -fno-builtin")
endif(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES DEBUG)

if(NOT DEFINED CMAKE_INSTALL_MANDIR)
    set(CMAKE_INSTALL_MANDIR "${CMAKE_INSTALL_PREFIX}/share/man")
endif()

include(GNUInstallDirs)

install(TARGETS hyprsunset)
install(FILES ${CMAKE_BINARY_DIR}/systemd/hyprsunset.service DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/user)
