cmake_minimum_required(VERSION 3.14)
project(Rift_Backend_DX9 VERSION 1.0.1)

# add our plugin dir to the module path so that we could locate the DX9 package
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_library(
        Rift_Backend_DX9
        STATIC
        private/Engine/Backend/DX9/DX9_Backend.cpp
        private/Engine/Backend/DX9/DX9_Shader.cpp
        private/Engine/Backend/DX9/DX9_ShaderProgram.cpp
        private/Engine/Backend/DX9/DX9_VertexBuffer.cpp
        private/Engine/Backend/DX9/DX9_Texture.cpp
)

# platform checks to disallow compilation on different platforms than Windows
if (NOT WIN32)
    message(FATAL_ERROR "DirectX is only supported on Windows OS.")
endif ()

find_package(DX9)

target_include_directories(
        Rift_Backend_DX9
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/public"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/private" ${DX9_INCLUDE_DIRS}
)

target_compile_definitions(
        Rift_Backend_DX9
        PUBLIC D3D_DEBUG_INFO
)

target_link_libraries(Rift_Backend_DX9 RiftEngine_Core_Runtime Rift_Runtime_Logging ${DX9_LIBRARIES})