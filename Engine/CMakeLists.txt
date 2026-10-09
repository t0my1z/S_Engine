find_package(Vulkan REQUIRED)

add_subdirectory(thirdparty EXCLUDE_FROM_ALL)

add_library(SEngine)

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.c
)
file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS 
    include/*.h
    include?*.hpp
)

target_sources(SEngine 
    PRIVATE ${ENGINE_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES ${ENGINE_HEADERS}
)

target_include_directories(SEngine 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${Vulkan_INCLUDE_DIRS}
)

target_compile_definitions(SEngine PUBLIC 
    VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1 
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 
    VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS=1
)

target_link_libraries(SEngine
    PUBLIC
        Vulkan::Vulkan #also gives glm
        glfw
        tinyobjloader
        tinygltf
        nlohmann_json
        stb
        OpenAL::OpenAL
        spdlog::spdlog
        imgui
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}
             FILES ${ENGINE_SOURCES} ${ENGINE_HEADERS})

#Archive for .a / .lib | Library for .so / .dll
install(TARGETS SEngine
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    FILE_SET HEADERS DESTINATION include
)

include(CompileShaders)
compile_project_shaders(SEngine)

include(InstallResources)
install_resources(SEngine)
