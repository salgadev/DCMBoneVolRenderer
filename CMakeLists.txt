
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project(DCMBoneVolRender)

find_package(VTK COMPONENTS 
  vtkCommonColor
  vtkCommonCore
  vtkCommonDataModel
  vtkIOImage
  vtkInteractionStyle
  vtkRenderingCore
  vtkRenderingFreeType
  vtkRenderingOpenGL2
  vtkRenderingVolume
  vtkRenderingVolumeOpenGL2 QUIET)
if (NOT VTK_FOUND)
  message("Skipping DCMBoneVolRender: ${VTK_NOT_FOUND_MESSAGE}")
  return ()
endif()
message (STATUS "VTK_VERSION: ${VTK_VERSION}")
if (VTK_VERSION VERSION_LESS "8.90.0")
  # old system
  include(${VTK_USE_FILE})
  add_executable(DCMBoneVolRender MACOSX_BUNDLE DCMBoneVolRender.cxx )
  target_link_libraries(DCMBoneVolRender PRIVATE ${VTK_LIBRARIES})
else ()
  # include all components
  add_executable(DCMBoneVolRender MACOSX_BUNDLE DCMBoneVolRender.cxx )
  target_link_libraries(DCMBoneVolRender PRIVATE ${VTK_LIBRARIES})
  # vtk_module_autoinit is needed
  vtk_module_autoinit(
    TARGETS DCMBoneVolRender
    MODULES ${VTK_LIBRARIES}
    )
endif () 
