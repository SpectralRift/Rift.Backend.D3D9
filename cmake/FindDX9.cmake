# Adapted version of https://github.com/adobe/webkit/blob/master/Source/cmake/FindDirectX.cmake

find_path(DX9_INCLUDE_DIRS NAMES "d3dx9.h" PATHS
        "$ENV{PROGRAMFILES}/Microsoft DirectX SDK*/Include"
        "$ENV{PROGRAMFILES\(x64\)}/Microsoft DirectX SDK*/Include"
        "$ENV{DXSDK_DIR}/Include"
)

get_filename_component(DX9_ROOT_DIR "${DX9_INCLUDE_DIRS}/.." ABSOLUTE)

IF (CMAKE_CL_64)
    set(DX9_LIBRARY_PATHS "${DX9_ROOT_DIR}/Lib/x64")
ELSE ()
    set(DX9_LIBRARY_PATHS "${DX9_ROOT_DIR}/Lib/x86" "${DX9_ROOT_DIR}/Lib")
ENDIF ()

find_library(DX9_D3D9_LIBRARY d3d9 ${DX9_LIBRARY_PATHS} NO_DEFAULT_PATH)
find_library(DX9_D3DX9_LIBRARY d3dx9 ${DX9_LIBRARY_PATHS} NO_DEFAULT_PATH)

set(DX9_LIBRARIES ${DX9_D3D9_LIBRARY} ${DX9_D3DX9_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DX9 DEFAULT_MSG DX9_ROOT_DIR DX9_LIBRARIES DX9_INCLUDE_DIRS)
mark_as_advanced(DX9_INCLUDE_DIRS DX9_D3D9_LIBRARY DX9_D3DX9_LIBRARY)