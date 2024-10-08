
cmake_minimum_required(VERSION 3.10)
project (EnableStandard CUDA)

#Goal for this example:
#build cuda sources that require C++11 to be enabled.

add_library(CUDAStatic11 STATIC static.cu)
add_library(CUDADynamic11 SHARED shared.cu)

add_executable(CudaOnlyEnableStandard main.cu)
target_link_libraries(CudaOnlyEnableStandard PRIVATE CUDAStatic11 CUDADynamic11)

target_compile_features(CUDADynamic11 PRIVATE cuda_std_11)
set_target_properties(CUDAStatic11 PROPERTIES CUDA_STANDARD 11)
set_target_properties(CUDAStatic11 PROPERTIES CUDA_STANDARD_REQUIRED TRUE)

#Verify CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES
foreach(dir ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
  if(NOT IS_DIRECTORY "${dir}")
    message(FATAL_ERROR
      "CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES entry\n"
      " ${dir}\n"
      "is not an existing directory."
      )
  endif()
endforeach()
