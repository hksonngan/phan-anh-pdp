###################
#note on Release mode
#you have to choose the release option in visual studio
#cmake is not allowed for setting this option
#but you can query this information using the cmake variable CMAKE_CFG_INTDIR
#see also readme.txt file
###################
cmake_minimum_required(VERSION 2.8)

project(pdp)

set(MITK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../MITK" CACHE PATH "Path to the MITK source folder")
set(MITK_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../superbuild-master" CACHE PATH "Path to the MITK superbuild folder to link to")
#this variable should replace CMAKE_CFG_INTDIR
set(MITK_INSTALL_CONFIG_DIR "Release" CACHE PATH "The configuration to take the 3rdparty dlls from during install")
set(CGAL_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cgal-3.8-x64" CACHE PATH "Path to the CGAL isntalled folder (after the build)")

set(CMAKE_MODULE_PATH ${MITK_SOURCE_DIR}/CMake ${CMAKE_MODULE_PATH})

#add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)

# Handle Qt stuff
#################
find_package(Qt4 REQUIRED QtCore QtGui)
include(${QT_USE_FILE})

# Handle CGAL stuff
###################
set(CGAL_INCLUDES
	"${CGAL_INSTALL_DIR}/include"
	"${CGAL_INSTALL_DIR}/boost/"
)
set(CGAL_LIBDIRS
	"${CGAL_INSTALL_DIR}/lib"
	"${CGAL_INSTALL_DIR}/boost/lib"
)
set(CGAL_LIBS
	optimized "CGAL-vc90-mt" debug "CGAL-vc90-mt-gd"
	optimized "libboost_thread-vc90-mt-1_46_1" debug "libboost_thread-vc90-mt-gd-1_46_1"
)

#enable OpenMP
##############
find_package(OpenMP)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
##############
#end OpenMP

# Handle ITK stuff
##################
set(ITK_INCLUDES
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ITK/Code/Algorithms"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ITK/Code/BasicFilters"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ITK/Code/Common"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ITK/Code/IO"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ITK/Code/Numerics"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ITK/Code/Numerics/FEM"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ITK/Code/Numerics/NeuralNetworks"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ITK/Code/Numerics/Statistics"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ITK/Code/Patented"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ITK/Code/Review"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ITK/Code/Review/Statistics"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ITK/Code/SpatialObject"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ITK/Utilities"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ITK/Utilities/vxl/vcl"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ITK/Utilities/vxl/core"
	"${MITK_BUILD_DIR}/ITK-build"
	"${MITK_BUILD_DIR}/ITK-build/Utilities/"
	"${MITK_BUILD_DIR}/ITK-build/Utilities/vxl/vcl"
	"${MITK_BUILD_DIR}/ITK-build/Utilities/vxl/core"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ex-ITK1235/InsightToolkit-3.20.0/Wrapping/WrapITK/ExternalProjects/ItkVtkGlue/src"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/ITK/Wrapping/WrapITK/ExternalProjects/ItkVtkGlue/src"
)
set(ITK_LIBDIRS
	"${MITK_BUILD_DIR}/ITK-build/bin/${MITK_INSTALL_CONFIG_DIR}" # CMAKE_CFG_INTDIR
	"${MITK_BUILD_DIR}/GDCM-build/bin/${MITK_INSTALL_CONFIG_DIR}" # CMAKE_CFG_INTDIR
)
set(ITK_LIBS
	"ITKCommon"
	"itksys"
	"itkv3p_netlib"
	"itkvcl"
	"ITKNumerics"
	"itkvnl"
	"itkvnl_algo"
	"ITKBasicFilters"

	# Necessary for loading DICOM files:
	"ITKDICOMParser"
	"ITKIO"
	"itkjpeg8"
	"ITKMetaIO"
	"ITKniftiio"
	"ITKNrrdIO"
	"itkpng"
	"itktiff"
	"itkzlib"
	"ITKznz"
	"gdcmCommon"
	"gdcmDICT"
	"gdcmDSED"
	"gdcmMSFF"
)

# Handle VTK stuff
##################
set(VTK_INCLUDES
	"${MITK_BUILD_DIR}/CMakeExternals/Source/VTK/Common"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/VTK/Imaging"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/VTK/Graphics"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/VTK/Filtering"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/VTK/GUISupport/Qt"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/VTK/Hybrid"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/VTK/IO"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/VTK/Rendering"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/VTK/Widgets"
	"${MITK_BUILD_DIR}/VTK-build"
	"${MITK_BUILD_DIR}/VTK-build/Rendering"
	#MOD
	"${MITK_BUILD_DIR}/CMakeExternals/Source/VTK/VolumeRendering"
)
set(VTK_LIBDIRS
	"${MITK_BUILD_DIR}/VTK-build/bin/${MITK_INSTALL_CONFIG_DIR}" #CMAKE_CFG_INTDIR
)
set(VTK_LIBS
	"vtkRendering"
	"vtkImaging"
	"vtkGraphics"
	"vtkCommon"
	"vtkFiltering"
	"vtkHybrid"
	"vtkIO"
	"vtkWidgets"
)

# Handle MITK stuff
###################
set(MITK_INCLUDES
	"${MITK_SOURCE_DIR}/Core/Code/Algorithms"
	"${MITK_SOURCE_DIR}/Core/Code/Controllers"
	"${MITK_SOURCE_DIR}/Core/Code/DataManagement"
	"${MITK_SOURCE_DIR}/Core/Code/Interactions"
	"${MITK_SOURCE_DIR}/Core/Code/IO"
	"${MITK_SOURCE_DIR}/Core/Code/Rendering"
	"${MITK_SOURCE_DIR}/CoreUI/Qmitk"
	"${MITK_SOURCE_DIR}/Modules/QmitkExt"
	"${MITK_SOURCE_DIR}/Utilities/"
	"${MITK_SOURCE_DIR}/Utilities/mbilog"
	"${MITK_SOURCE_DIR}/Utilities/ipPic"
	"${MITK_BUILD_DIR}/MITK-build"
	"${MITK_BUILD_DIR}/MITK-build/CoreUI/Qmitk"
	"${MITK_BUILD_DIR}/MITK-build/Modules/QmitkExt"
	"${MITK_BUILD_DIR}/MITK-build/modulesConf"
	"${MITK_BUILD_DIR}/MITK-build/Utilities/mbilog"

	"${MITK_BUILD_DIR}/GDCM-build/Source/Common"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/GDCM/Source/Common"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/GDCM/Source/DataDictionary"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/GDCM/Source/DataStructureAndEncodingDefinition"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/GDCM/Source/MediaStorageAndFileFormat"
	"${MITK_BUILD_DIR}/CMakeExternals/Source/GDCM/Utilities/C99"
	
	"${MITK_SOURCE_DIR}/Modules/SceneSerialization"
	"${MITK_SOURCE_DIR}/Utilities/Poco/Zip/include"
	"${MITK_SOURCE_DIR}/Utilities/Poco/Foundation/include"
	"${MITK_SOURCE_DIR}/CoreUI/Bundles/org.mitk.gui.common/src"
	"${MITK_SOURCE_DIR}/Modules"
	"${MITK_SOURCE_DIR}/Modules/MitkExt/DataManagement"
	"${MITK_SOURCE_DIR}/Modules/MitkExt/Controllers"
	"${MITK_SOURCE_DIR}/Modules/MitkExt/Interactions"
	"${MITK_SOURCE_DIR}/Modules/MitkExt/Rendering"
	"${MITK_SOURCE_DIR}/Modules/MitkExt/Algorithms"
	"${MITK_SOURCE_DIR}/Modules/Bundles/org.mitk.gui.qt.datamanager/src/internal"
)
set(MITK_LIBDIRS
	"${MITK_BUILD_DIR}/MITK-build/bin/${MITK_INSTALL_CONFIG_DIR}" #CMAKE_CFG_INTDIR
)
set(MITK_LIBS
	"mitkCore"
	"mitkCoreExt"
	"mbilog"
	"Qmitk"
	"QmitkExt"
	"QVTK"
)

# Putting it together
#####################
set(ALL_INCLUDE_DIRECTORIES ${ITK_INCLUDES} ${VTK_INCLUDES} ${MITK_INCLUDES} ${CGAL_INCLUDES})
include_directories(${ALL_INCLUDE_DIRECTORIES})

set(ALL_LIBDIRS ${ITK_LIBDIRS} ${VTK_LIBDIRS} ${MITK_LIBDIRS} ${CGAL_LIBDIRS})
link_directories(${ALL_LIBDIRS})

# Qt MOC file generation
########################
set(QT_MOC_GENERATION_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/app/thickening_detector.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/app/thickening_detection_step_base.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/gui/mainwindow.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/gui/home.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/gui/progress.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/gui/single_load.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/gui/single_thickenings.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/gui/settings.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/gui/settings_base.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/gui/settings_common.h"	
	#MOD THREEDEDITOR
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/threeDEditing.h"			
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/toolbar.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/dataManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/global.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/QmitkInfoDialog.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/timerThread.h"
)
QT4_WRAP_CPP(MOC_CPPS ${QT_MOC_GENERATION_FILES})

file(GLOB ALL_GUI "${CMAKE_CURRENT_SOURCE_DIR}/res/*.ui")
QT4_WRAP_UI(UI_CPPS ${ALL_GUI})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB ALL_RES "${CMAKE_CURRENT_SOURCE_DIR}/res/*.qrc" "${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/*.qrc")
QT4_ADD_RESOURCES(RES_CPPS ${ALL_RES})

set(ALL_QT_GENERATED_CODE ${MOC_CPPS} ${UI_CPPS} ${RES_CPPS})

# PDP Sourcecode files
######################

# Intentionally left out *.cxx because those are moc generated.
file(GLOB_RECURSE ALL_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.txx"
)

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif(DOXYGEN_FOUND)
 

# Automatically create Filters (a.k.a Subdirectories) within Visual Studios Project View
# which reflect our projects folder structure (keeping it nice and clean)
# :: found on cmake mailinglist http://permalink.gmane.org/gmane.comp.programming.tools.cmake.user/10226 
IF( MSVC )
    FOREACH( pathSourceFile ${ALL_SRC} )
	
        GET_FILENAME_COMPONENT( pathSourceGroup ${pathSourceFile} PATH )
		
        IF( NOT ${pathSourceGroup} STREQUAL ${CMAKE_SOURCE_DIR} )
            # Make the path relative to the base source path
            STRING( REPLACE ${CMAKE_SOURCE_DIR} "" pathSourceGroup ${pathSourceGroup} )
   
            # Remove starting / and \
            STRING( SUBSTRING ${pathSourceGroup} 0 1 firstChar )
            IF( firstChar STREQUAL "/" OR
                firstChar STREQUAL "\\" )
                   
                STRING( LENGTH ${pathSourceGroup} strLen )
                MATH( EXPR strLen "${strLen} - 1" )
                STRING( SUBSTRING ${pathSourceGroup} 1 ${strLen}  pathSourceGroup )
            ENDIF()
           
            # Replace / with \ so the source groups work correctly in MSVC
            STRING( REPLACE "/" "\\" pathSourceGroup ${pathSourceGroup} )
           
            # Add file to the source group
            GET_FILENAME_COMPONENT( fileExtension ${pathSourceFile} EXT )
            IF( ${fileExtension} MATCHES ".c" OR
                ${fileExtension} MATCHES ".cpp" OR ${fileExtension} MATCHES ".txx")
                SOURCE_GROUP( "Source Files\\${pathSourceGroup}" FILES ${pathSourceFile} )
            ELSEIF( ${fileExtension} MATCHES ".h" OR
                    ${fileExtension} MATCHES ".hpp" )
                SOURCE_GROUP( "Header Files\\${pathSourceGroup}" FILES ${pathSourceFile} )
            ELSE()
                MESSAGE( SEND_ERROR "Source grouping failed on file ${pathSourceFile}" )
            ENDIF()
           
        ENDIF()
       
    ENDFOREACH()
ENDIF()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#MOD THREEDEDITOR
set(THREEDEDITOR_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/threeDEditing.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/toolbar.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/dataManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/global.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/QmitkInfoDialog.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/RenderingManagerProperty.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/WheelScrollInteractor.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/RadiusInteractor.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/timerThread.h"
	
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/threeDEditing.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/toolbar.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/dataManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/QmitkInfoDialog.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/RenderingManagerProperty.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/WheelScrollInteractor.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/RadiusInteractor.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/timerThread.cpp"
)
SOURCE_GROUP( "Source Files\\ThreeDEditing" FILES ${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/threeDEditing.cpp
					                              ${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/toolbar.cpp
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/dataManager.cpp
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/RenderingManagerProperty.cpp
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/WheelScrollInteractor.cpp
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/RadiusInteractor.cpp
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/timerThread.cpp
                                                  )
SOURCE_GROUP( "Header Files\\ThreeDEditing" FILES ${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/threeDEditing.h
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/toolbar.h
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/dataManager.h
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/global.h
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/RenderingManagerProperty.h
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/WheelScrollInteractor.h
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/RadiusInteractor.h
                                                  ${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/timerThread.h
                                                  )

#MOD THREEDEDITOR
add_executable(pdp ${ALL_SRC} ${THREEDEDITOR_SRC} ${ALL_QT_GENERATED_CODE})


set(ALL_LIBRARIES ${ITK_LIBS} ${VTK_LIBS} ${MITK_LIBS} ${QT_LIBRARIES} ${CGAL_LIBS})
target_link_libraries(pdp ${ALL_LIBRARIES})

# Copying DLLs
##############
install(TARGETS pdp DESTINATION bin)

# Just copy over all dlls there are in MITK. If you want it more fine grained, go ahead and good luck!
file(GLOB_RECURSE ALL_DLLS
	"${MITK_BUILD_DIR}/*.dll"
)

install(FILES ${ALL_DLLS} DESTINATION bin)
install(FILES "${QT_BINARY_DIR}/phonon4.dll" DESTINATION bin)
install(FILES "${QT_BINARY_DIR}/phonond4.dll" DESTINATION bin)
install(FILES "${QT_BINARY_DIR}/QtCore4.dll" DESTINATION bin)
install(FILES "${QT_BINARY_DIR}/QtCored4.dll" DESTINATION bin)
install(FILES "${QT_BINARY_DIR}/QtGui4.dll" DESTINATION bin)
install(FILES "${QT_BINARY_DIR}/QtGuid4.dll" DESTINATION bin)
install(FILES "${QT_BINARY_DIR}/QtNetwork4.dll" DESTINATION bin)
install(FILES "${QT_BINARY_DIR}/QtNetworkd4.dll" DESTINATION bin)
install(FILES "${QT_BINARY_DIR}/QtOpenGL4.dll" DESTINATION bin)
install(FILES "${QT_BINARY_DIR}/QtOpenGLd4.dll" DESTINATION bin)
install(FILES "${QT_BINARY_DIR}/QtSql4.dll" DESTINATION bin)
install(FILES "${QT_BINARY_DIR}/QtSqld4.dll" DESTINATION bin)
install(FILES "${QT_BINARY_DIR}/QtSvg4.dll" DESTINATION bin)
install(FILES "${QT_BINARY_DIR}/QtSvgd4.dll" DESTINATION bin)
install(FILES "${QT_BINARY_DIR}/QtWebKit4.dll" DESTINATION bin)
install(FILES "${QT_BINARY_DIR}/QtWebKitd4.dll" DESTINATION bin)

ADD_CUSTOM_TARGET(run)
ADD_CUSTOM_COMMAND(TARGET run POST_BUILD COMMAND pdp.exe WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../install/bin/)
ADD_DEPENDENCIES(run ALL INSTALL pdp)

# Adding tools
ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/Tools/BulgeTool/src/release/ CMakeLists.txt/BulgeTool)
#ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/Tools/CutTool/src/release/ CMakeLists.txt/CutTool)
#ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/Tools/FillHoleTool/src/release/ CMakeLists.txt/FillHoleTool)
#ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/Tools/MoveROI/src/release/ CMakeLists.txt/MoveROITool)
#ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/Tools/SmoothTool/src/release/ CMakeLists.txt/SmoothTool)
#ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/Tools/SurfaceRefinementTool/src/release/ CMakeLists.txt/SurfaceRefinementTool)
#ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/Tools/TestTool/src/release/ CMakeLists.txt/TestTool)
#ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/Tools/RegistrationTool/src/release/ CMakeLists.txt/RegistrationTool)
#ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/Tools/ActiveSurfaceTool/src/release/ CMakeLists.txt/ActiveSurfaceTool)
#ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/Tools/ActiveContourTool/src/release/ CMakeLists.txt/ActiveContourTool)
#ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/Tools/DragTool/src/release/ CMakeLists.txt/DragTool)
#ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/Tools/LiveWireTool/src/release/ CMakeLists.txt/LiveWireTool)
#ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/../SVN/ThreeDEditing/Tools/RegionGrowingTool/src/release/ CMakeLists.txt/RegionGrowingTool)

# Copying tool dlls
ADD_CUSTOM_TARGET(CopyAllDll)
ADD_CUSTOM_COMMAND(TARGET CopyAllDll
          POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt/BulgeTool/Release/mitkBulgeTool.dll ${CMAKE_CURRENT_BINARY_DIR}/../../Hendrik_pdp/SVN/ThreeDEditing/Tools/Tools/
          #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt/CutTool/Release/mitkCutTool.dll ${CMAKE_CURRENT_BINARY_DIR}/../SVN/ThreeDEditing/Tools/Tools/     
          #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt/FillHoleTool/Release/mitkFillHoleTool.dll ${CMAKE_CURRENT_BINARY_DIR}/../SVN/ThreeDEditing/Tools/Tools/
          #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt/MoveROITool/Release/mitkExternalTools.dll ${CMAKE_CURRENT_BINARY_DIR}/../SVN/ThreeDEditing/Tools/Tools/
          #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt/SmoothTool/Release/mitkSmoothTool.dll ${CMAKE_CURRENT_BINARY_DIR}/../SVN/ThreeDEditing/Tools/Tools/
          #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt/SurfaceRefinementTool/Release/mitkSurfaceRefinementTool.dll ${CMAKE_CURRENT_BINARY_DIR}/../SVN/ThreeDEditing/Tools/Tools/
		  #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt/TestTool/Release/mitkTestTool.dll ${CMAKE_CURRENT_BINARY_DIR}/../SVN/ThreeDEditing/Tools/Tools/
		  #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt/RegistrationTool/Release/mitkRegistrationTool.dll ${CMAKE_CURRENT_BINARY_DIR}/../SVN/ThreeDEditing/Tools/Tools/
		  #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt/ActiveSurfaceTool/Release/mitkActiveSurfaceTool.dll ${CMAKE_CURRENT_BINARY_DIR}/../SVN/ThreeDEditing/Tools/Tools/
		  #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt/ActiveContourTool/Release/mitkActiveContourTool.dll ${CMAKE_CURRENT_BINARY_DIR}/../SVN/ThreeDEditing/Tools/Tools/
		  #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt/DragTool/Release/mitkDragTool.dll ${CMAKE_CURRENT_BINARY_DIR}/../SVN/ThreeDEditing/Tools/Tools/
		  #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt/LiveWireTool/Release/mitkLiveWireTool.dll ${CMAKE_CURRENT_BINARY_DIR}/../SVN/ThreeDEditing/Tools/Tools/
		  #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt/RegionGrowingTool/Release/mitkRegionGrowingTool.dll ${CMAKE_CURRENT_BINARY_DIR}/../SVN/ThreeDEditing/Tools/Tools/
)