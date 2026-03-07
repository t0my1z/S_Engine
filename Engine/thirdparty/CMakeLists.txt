# GLFW
add_subdirectory(glfw)

# tinyobjloader
add_subdirectory(tinyobjloader)

# tinygltf
add_subdirectory(tinygltf)

# KTX -> For now we ignore it
#add_subdirectory(ktx)

# OpenAL
add_subdirectory(openal_soft)

# spdlog
add_subdirectory(spdlog)

# imgui
add_subdirectory(imgui)

# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)

# JSON
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE json/include)