cmake_minimum_required(VERSION 3.24)

project(SEngineApp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

add_subdirectory(Engine)
add_subdirectory(App)

add_custom_target(install_all
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR}
    DEPENDS App
)