/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-09-12 15:46:48 +0200 (vie, 12 sep 2008) $
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkTestTool_h_Included
#define mitkTestTool_h_Included

#include "mitkDataStorage.h"
#include "mitkSurface.h"
#include <mitkTool.h>

namespace mitk
{
	/**
	  \warning Only to be instantiated by mitk::ToolManager.
	*/
	class TestTool : public mitk::Tool
	{
		public:

			/**
			* Needed because we use itk::SmartPointers and like
			* to have some typedefs just as most itk::Objects
			*/
			mitkClassMacro(TestTool, mitk::Tool);
			/**
			* Created through itk::ObjectFactory, so we need a
			* ::New() method, which is created by this macro.
			*/
			itkNewMacro(TestTool);
			/**
			* \brief Returns an icon for this tool.
			*
			* Saved in the XPM format, can be written e.g. by The Gimp.
			*/
			const char** GetXPM() const;
			/**
			* \brief Returns a (short) name for this tool.
			*
			* E.g. in InteractiveSegmentation, this string
			* would be displayed in a toolbutton right below
			* the tool's icon.
			*/
			const char* GetName() const;
			/**
			* \brief Returns a group name for this tool.
			* Tools can be grouped, and each group is described
			* by a string.
			*/
			const char* GetGroup() const;

			// Tool extension interface
			void StartAuswertung();
			void ConvertData();
			double CalculateVolume(std::string);
			double CalculateVolumeViaPixel(std::string);
			void Closing();
			mitk::Surface::Pointer ConvertAndSmoothData(mitk::Image::Pointer);
			void LoadConvertAndSave(std::string, std::string, std::string);
			void AuswertungPixel();

			std::string GetDirectory(){return m_Directory;};
			bool GetActiveSurfaceBool(){return m_ActiveSurface;};
			bool GetAddSubstract1Bool(){return m_AddSubstract1;};
			bool GetAddSubstract2Bool(){return m_AddSubstract2;};
			bool GetBulgeBool(){return m_Bulge;};
			bool GetDragBool(){return m_Drag;};
			bool GetLiveWireBool(){return m_LiveWire;};
			bool GetPaintWipe1Bool(){return m_PaintWipe1;};
			bool GetPaintWipe2Bool(){return m_PaintWipe2;};
			std::vector<std::string> GetNames(){return m_Names;};
			DataStorage* GetStorage(){return m_Storage;};

		protected:
			TestTool(); // purposely hidden, should always be done with itk::Objects that are create by New()
			~TestTool();
			/**
			* \brief Called when tool becomes active.
			*/
			void Activated();
			/**
			* \brief Called when tool is deactivated.
			*/
			void Deactivated();
			/**
			* \brief Overloaded to process mouse move events.
			*/
			//bool OnMouseMoved   (Action*, const StateEvent*);
		private:
			// Tool extension interface
			DataStorage* m_Storage;
			std::vector<std::string> m_Names;
			std::string m_Directory; 
			bool m_ActiveSurface;
			bool m_AddSubstract1;
			bool m_AddSubstract2;
			bool m_Bulge;
			bool m_Drag;
			bool m_LiveWire;
			bool m_PaintWipe1;
			bool m_PaintWipe2;
	};

} // namespace

#endif


