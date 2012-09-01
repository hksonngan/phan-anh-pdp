###################
#note on Release mode
#you have to choose the release option in visual studio
#cmake is not allowed for setting this option
#but you can query this information using the cmake variable CMAKE_CFG_INTDIR
#see also readme.txt file
###################
cmake_minimum_required(VERSION 2.8)

PROJECT ( ExternalTool )

set(MITK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../MITK" CACHE PATH "Path to the MITK source folder")
set(MITK_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../superbuild-master" CACHE PATH "Path to the MITK superbuild folder to link to")
#this variable should replace CMAKE_CFG_INTDIR
set(MITK_INSTALL_CONFIG_DIR "Debug" CACHE PATH "The configuration to take the 3rdparty dlls from during install")
set(CGAL_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cgal-3.8-x64" CACHE PATH "Path to the CGAL isntalled folder (after the build)")

set(CMAKE_MODULE_PATH ${MITK_SOURCE_DIR}/CMake ${CMAKE_MODULE_PATH})

add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)

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
	optimized "libboost_thread-vc90-mt-1_47" debug "libboost_thread-vc90-mt-gd-1_47"
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
	"${MITK_SOURCE_DIR}/Modules/MitkExt/Controllers"
	"${MITK_SOURCE_DIR}/Modules/MitkExt/Interactions"
	"${MITK_SOURCE_DIR}/Modules/MitkExt/DataManagement"
	"${MITK_SOURCE_DIR}/Modules/MitkExt/Rendering"
	"${MITK_SOURCE_DIR}/Modules/MitkExt/Algorithms"
	"${MITK_SOURCE_DIR}/Modules/Bundles/org.mitk.gui.qt.datamanager/src/internal"
	
	# Test fuer #include <QmitkSegmentationView.h>
	"${MITK_SOURCE_DIR}/Modules/Bundles/org.mitk.gui.qt.segmentation/src/internal"
	"${MITK_SOURCE_DIR}/CoreUI/Bundles/org.mitk.gui.qt.common/src"
	"${MITK_SOURCE_DIR}/BlueBerry/Bundles/org.blueberry.ui.qt/src"
	"${MITK_SOURCE_DIR}/BlueBerry/Bundles/org.blueberry.ui/src"
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

set(ALL_LIBDIRS ${ITK_LIBDIRS} ${VTK_LIBDIRS} ${MITK_LIBDIRS} )#${CGAL_LIBDIRS})
link_directories(${ALL_LIBDIRS})


# Qt MOC file generation
########################
set(QT_MOC_GENERATION_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/../../SVN/ThreeDEditing/Tools/ExampleTool/src/QmitkComplexExternalToolGUI.cpp"
)
QT4_WRAP_CPP(MOC_CPPS ${QT_MOC_GENERATION_FILES})

file(GLOB ALL_GUI "${CMAKE_CURRENT_SOURCE_DIR}/res/*.ui")
QT4_WRAP_UI(UI_CPPS ${ALL_GUI})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB ALL_RES "${CMAKE_CURRENT_SOURCE_DIR}/res/*.qrc" "${CMAKE_CURRENT_SOURCE_DIR}/../../SVN/ThreeDEditing/*.qrc")
QT4_ADD_RESOURCES(RES_CPPS ${ALL_RES})

set(ALL_QT_GENERATED_CODE ${MOC_CPPS} ${UI_CPPS} ${RES_CPPS})

SET( TOOL_QT3GUI_FILES
     QmitkComplexExternalToolGUI.cpp
   )

# Fill in all tool files (implementations) here.
# The macro later will process the file names and
# expect that these files contain a class of the
# same name as the file, i.e mitkPlainExternalTool.cpp
# is expected to contain a class mitk::PlainExternalTool.
# The macro will create a factory for this tool, which
# is constructed for these expected names, in this case
# the factory name would be mitk::PlainExternalToolFactory.
SET( TOOL_FILES
     #mitkPlainExternalTool.cpp
     mitkComplexExternalTool.cpp
   )

# List here all additional files which need to be compiled
# into the new library.
SET( TOOL_ADDITIONAL_CPPS
)

# List here all additional headers which need to be run
# through Qt's moc. The moc-created files will automatically
# be added to TOOL_ADDITIONAL_CPPS and compiled into the lib.
SET( TOOL_ADDITIONAL_MOC_H
)

INCLUDE(CMake/ToolExtensionHelpers.cmake)

# Create a library called libmitkExternalTools.so/DLL.
# The macro will create this library and add all tools,
# tool GUIs and their respective factories to it.
MITK_GENERATE_TOOLS_LIBRARY(mitkExternalTools)

set(ALL_LIBRARIES ${ITK_LIBS} ${VTK_LIBS} ${MITK_LIBS} ${QT_LIBRARIES}) #${CGAL_LIBS}
target_link_libraries(mitkExternalTools ${ALL_LIBRARIES})
