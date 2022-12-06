cmake_minimum_required(VERSION 3.20)
project(llvm)

set(CMAKE_CXX_STANDARD 23)
#set(LLVM_DIR D:/software/vcpkg/vcpkg/installed/x64-windows-static/share/llvm)
list(APPEND CMAKE_PREFIX_PATH D:/software/vcpkg/vcpkg/installed/x64-windows-static-md/share/llvm)

set(ZLIB_INCLUDE_DIR "D:/software/vcpkg/vcpkg/installed/x64-windows-static-md/include" )
set(ZLIB_LIBRARY "D:/software/vcpkg/vcpkg/installed/x64-windows-static-md/lib/zlib.lib")

find_package(LLVM REQUIRED CONFIG)
find_package(ZLIB)


message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(xxx main.cpp)

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core irreader executionengine target codegen X86Desc X86Info X86CodeGen OrcJIT)

MESSAGE(${llvm_libs})
# Link against LLVM libraries
target_link_libraries(xxx ${llvm_libs})