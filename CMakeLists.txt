cmake_minimum_required(VERSION 3.14)
project(Rift_Backend_D3D9 VERSION 1.0.1)

# add our plugin dir to the module path so that we could locate the D3D9 package
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_library(
        Rift_Backend_D3D9
        STATIC
        private/Engine/Backend/D3D9/D3D9_Backend.cpp
        private/Engine/Backend/D3D9/D3D9_Shader.cpp
        private/Engine/Backend/D3D9/D3D9_ShaderProgram.cpp
        private/Engine/Backend/D3D9/D3D9_VertexBuffer.cpp
        private/Engine/Backend/D3D9/D3D9_Texture.cpp
)

# platform checks to disallow compilation on different platforms than Windows
if (NOT WIN32)
    message(FATAL_ERROR "DirectX is only supported on Windows OS.")
endif ()

find_package(DX9)

target_include_directories(
        Rift_Backend_D3D9
        PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/public"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/private" ${DX9_INCLUDE_DIRS}
)

target_compile_definitions(
        Rift_Backend_D3D9
        PUBLIC D3D_DEBUG_INFO
)

rift_resolve_module_libs("Rift.Core.Runtime;Rift.Runtime.Logging" RIFT_D3D9_DEPS)

target_link_libraries(Rift_Backend_D3D9 ${RIFT_D3D9_DEPS} ${DX9_LIBRARIES})