cmake_minimum_required(VERSION 2.8)

PROJECT ( ExternalTool )

# Home
#set(MITK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../MITKpdp/MITK" CACHE PATH "Path to the MITK source folder")
#set(MITK_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../MITKpdp/superbuild-master" CACHE PATH "Path to the MITK superbuild folder to link to")
# Institut
set(MITK_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../PDP-MITK/MITK" CACHE PATH "Path to the MITK source folder")
set(MITK_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../PDP-MITK/release/superbuild-master" CACHE PATH "Path to the MITK superbuild folder to link to")
set(MITK_INSTALL_CONFIG_DIR "Release" CACHE PATH "The configuration to take the 3rdparty dlls from during install")
  
set(CMAKE_MODULE_PATH ${MITK_SOURCE_DIR}/CMake ${CMAKE_MODULE_PATH})
  
# When configuring cmake, 
# set ITK_DIR to the directory, 
# where you compiled or installed ITK
FIND_PACKAGE(ITK)
#FIND_PACKAGE(ITK PATHS "${MITK_BINARY_DIR}/CMakeExternals/Source/ITK" NO_DEFAULT_PATH)
#FIND_PACKAGE(ITK-3.20 PATHS ${THIRDPARTY_BUILD_FOLDERS}) # JAM: Patched (for our own ITK)

# When configuring cmake, 
# Set MITK_DIR to the directory, 
# where you compiled MITK
FIND_PACKAGE(MITK)
#set(MITK "C:/DA/MITKpdp/superbuild-master/MITK-build/bin/Debug")

# Make sure we find Qt3, this won't yet work with Qt4 
# (and MITK could even be compiled without any Qt)
#FIND_PACKAGE(Qt3)
FIND_PACKAGE(Qt4 REQUIRED QtCore QtGui)
include(${QT_USE_FILE})

# Just needs to be here (?)
ADD_DEFINITIONS(${QT_DEFINITIONS})

# Handle ITK stuff
##################
set(ITK_INCLUDES
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/ITK/Code/Algorithms"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/ITK/Code/BasicFilters"
	"${MITK_BINARY_DIR}/../CMakeExternals/Source/ITK/Code/Common"
	"${MITK_BINARY_DIR}/../CMakeExternals/Source/ITK/Code/IO"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/ITK/Code/Numerics"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/ITK/Code/Numerics/FEM"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/ITK/Code/Numerics/NeuralNetworks"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/ITK/Code/Numerics/Statistics"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/ITK/Code/Patented"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/ITK/Code/Review"
	"${MITK_BINARY_DIR}/../CMakeExternals/Source/ITK/Code/Review/Statistics"
	"${MITK_BINARY_DIR}/../CMakeExternals/Source/ITK/Code/SpatialObject"
	"${MITK_BINARY_DIR}/../CMakeExternals/Source/ITK/Utilities"
	"${MITK_BINARY_DIR}/../CMakeExternals/Source/ITK/Utilities/vxl/vcl"
	"${MITK_BINARY_DIR}/../CMakeExternals/Source/ITK/Utilities/vxl/core"
	"${MITK_BINARY_DIR}/../ITK-build"
	"${MITK_BINARY_DIR}/../ITK-build/Utilities/"
	"${MITK_BINARY_DIR}/../ITK-build/Utilities/vxl/vcl"
	"${MITK_BINARY_DIR}/../ITK-build/Utilities/vxl/core"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/ex-ITK1235/InsightToolkit-3.20.0/Wrapping/WrapITK/ExternalProjects/ItkVtkGlue/src"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/ITK/Wrapping/WrapITK/ExternalProjects/ItkVtkGlue/src"
)
set(ITK_LIBDIRS
	"${MITK_BINARY_DIR}/../ITK-build/bin/${MITK_INSTALL_CONFIG_DIR}" # CMAKE_CFG_INTDIR
	"${MITK_BINARY_DIR}/../GDCM-build/bin/${MITK_INSTALL_CONFIG_DIR}" # CMAKE_CFG_INTDIR
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
	"${MITK_BINARY_DIR}/../CMakeExternals/Source/VTK/Common"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/VTK/Imaging"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/VTK/Graphics"
	"${MITK_BINARY_DIR}/../CMakeExternals/Source/VTK/Filtering"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/VTK/GUISupport/Qt"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/VTK/Hybrid"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/VTK/IO"
	"${MITK_BINARY_DIR}/../CMakeExternals/Source/VTK/Rendering"
	"${MITK_BINARY_DIR}/../VTK-build"
	#"${MITK_BINARY_DIR}/../VTK-build/Rendering"
	#MOD
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/VTK/VolumeRendering"
)
set(VTK_LIBDIRS
	"${MITK_BINARY_DIR}/../VTK-build/bin/${MITK_INSTALL_CONFIG_DIR}" #CMAKE_CFG_INTDIR
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
	#"${MITK_SOURCE_DIR}/Core/Code/Algorithms"
	#"${MITK_SOURCE_DIR}/Core/Code/Controllers"
	#"${MITK_SOURCE_DIR}/Core/Code/DataManagement"
	#"${MITK_SOURCE_DIR}/Core/Code/Interactions"
	#"${MITK_SOURCE_DIR}/Core/Code/IO"
	#"${MITK_SOURCE_DIR}/Core/Code/Rendering"
	#"${MITK_SOURCE_DIR}/CoreUI/Qmitk"
	"${MITK_SOURCE_DIR}/Modules/QmitkExt"
	"${MITK_SOURCE_DIR}/Utilities/"
	"${MITK_SOURCE_DIR}/Utilities/mbilog"
	"${MITK_SOURCE_DIR}/Utilities/ipPic"
	"${MITK_BINARY_DIR}/"
	#"${MITK_BINARY_DIR}/CoreUI/Qmitk"
	#"${MITK_BINARY_DIR}/Modules/QmitkExt"
	"${MITK_BINARY_DIR}/modulesConf"
	"${MITK_BINARY_DIR}/Utilities/mbilog"

	#"${MITK_BINARY_DIR}/../GDCM-build/Source/Common"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/GDCM/Source/Common"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/GDCM/Source/DataDictionary"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/GDCM/Source/DataStructureAndEncodingDefinition"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/GDCM/Source/MediaStorageAndFileFormat"
	#"${MITK_BINARY_DIR}/../CMakeExternals/Source/GDCM/Utilities/C99"
	
	#"${MITK_SOURCE_DIR}/Modules/SceneSerialization"
	#"${MITK_SOURCE_DIR}/Utilities/Poco/Zip/include"
	#"${MITK_SOURCE_DIR}/Utilities/Poco/Foundation/include"
	#"${MITK_SOURCE_DIR}/CoreUI/Bundles/org.mitk.gui.common/src"
	#"${MITK_SOURCE_DIR}/Modules"
	"${MITK_SOURCE_DIR}/Modules/MitkExt/DataManagement"
	"${MITK_SOURCE_DIR}/Modules/MitkExt/Controllers"
	"${MITK_SOURCE_DIR}/Modules/MitkExt/Interactions"
	#"${MITK_SOURCE_DIR}/Modules/MitkExt/Rendering"
	"${MITK_SOURCE_DIR}/Modules/MitkExt/Algorithms"
	#"${MITK_SOURCE_DIR}/Modules/Bundles/org.mitk.gui.qt.datamanager/src/internal"
)
set(MITK_LIBDIRS
	"${MITK_BINARY_DIR}/bin/${MITK_INSTALL_CONFIG_DIR}" #CMAKE_CFG_INTDIR
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
set(ALL_INCLUDE_DIRECTORIES ${ITK_INCLUDES} ${VTK_INCLUDES} ${MITK_INCLUDES})# ${CGAL_INCLUDES})
include_directories(${ALL_INCLUDE_DIRECTORIES})

set(ALL_LIBDIRS ${ITK_LIBDIRS} ${VTK_LIBDIRS} ${MITK_LIBDIRS} )#${CGAL_LIBDIRS})
set(ALL_LIBRARIES ${ITK_LIBS} ${VTK_LIBS} ${MITK_LIBS} ${QT_LIBRARIES} )#${CGAL_LIBS})

#link_directories(${ALL_LIBDIRS})

# Fill in all tool GUI files (implementations) here.
# The macro later will process the file names and
# expect that these files contain a class of the
# same name as the file, i.e QmitkComplexExternalToolGUI.cpp
# is expected to contain a class QmitkComplexExternalToolGUI.
# The macro will create a factory for this tool GUI, which
# is constructed for these expected names, in this case
# the factory name would be QmitkComplexExternalToolGUIFactory.
#SET( TOOL_QT3GUI_FILES
#     QmitkComplexExternalToolGUI.cpp
#   )
SET( TOOL_QT4GUI_FILES
     QmitkAddThreeDToolGUI.cpp
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
     mitkAddThreeDTool.cpp
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

# Create a library called libmitkExternalTools.so/DLL.
# The macro will create this library and add all tools,
# tool GUIs and their respective factories to it.
MITK_GENERATE_TOOLS_LIBRARY(mitkExternalTools)

ADD_CUSTOM_TARGET(run)
ADD_CUSTOM_COMMAND(TARGET run POST_BUILD COMMAND pdp.exe WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../../../../../PDP-MITK/release/pdp-inst/bin/)


