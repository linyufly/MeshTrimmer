cmake_minimum_required(VERSION 2.8)

project(MeshTrimmer)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

if (ITKVtkGlue_LOADED)
  find_package(VTK REQUIRED)
  include(${VTK_USE_FILE})
endif()

add_executable(MeshTrimmerTest mesh_trimmer_test.cc)
target_link_libraries(MeshTrimmerTest ${VTK_LIBRARIES})

