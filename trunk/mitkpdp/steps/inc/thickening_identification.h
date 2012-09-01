#pragma once

#include "app/thickening_detection_step_base.h"
#include "steps/inc/Thickening3d.h"
#include "app/itk_filter_progress_watcher.h"

#include <QString>
#include <itkImage.h>
#include <mitkImage.h>
#include <mitkImageCast.h>

//! \todo remove?
//#include "steps/inc/pdpItkSliceExtractor.h"
//#include "steps/inc/ContourFilter.h"

namespace pdp {

	class thickeningsIdentification : public ThickeningDetectionStep
	{

	public:
		
		thickeningsIdentification(QString info);
		
		void connectThickeningsBoundingBox(LungDataset& input, Thickening& thick, int slice, int thickNum, bool isLeft);
		void connectThickeningsPolygon(LungDataset& input, Thickening& thick, int slice, int thickNum, bool isLeft);
		/**
		* Checks HU values of the 3d thickenings's tissue. Then eliminates the wrong thickenings. 
		*
		* \param [in,out]	input	The whole dataset. 
		**/
		void checkTissue(LungDataset& input);
		virtual QString name() const;
		virtual float relativePercent() const;
		virtual void work(LungDataset& input, LungDataset& output);

		void setInputStepName(QString input)
		{
			inputStepName = input;
		}
	private:
		
		QString info;
		QString inputStepName;
		int thickRemovalSize; //minimum size of the area of the thickening to be removed
	};

	
	
} // namespace pdp

#include "steps/inc/thickening_identification.txx"