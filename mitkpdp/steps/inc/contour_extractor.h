//thickenings identification
#pragma once

#include "app/thickening_detection_step_base.h"
#include "steps/inc/itk_trace_path_on_image_filter.h"

#include <QString>
#include <itkContourExtractor2DImageFilter.h>
#include <itkTileImageFilter.h>
#include <itkExtractImageFilter.h>
#include <vector>
#include <itkImage.h>
#include <itkPolyLineParametricPath.h>

namespace pdp {

	template <class PixelType> class contourExtractor : public ThickeningDetectionStep
	{

	public:
		typedef itk::Image<PixelType, 3>  ImageType;
		typedef itk::Image<PixelType, 2>  ImageType2D;
		typedef itk::PolyLineParametricPath<2> PathType;

		contourExtractor(QString info);
		
		void extractContour(typename ImageType2D::Pointer img, int part, int firstOrLast, sliceContour& sliceCont);
		void Contour(const typename ImageType::Pointer img, LungDataset& input);

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

		int boxBottom[2][4]; //top>right>bottom>left //boxBottom[0]-left lung
		int boxUp[2][4];

		static const int middlePercentage = 20; //if 15, then middle slices are 15% - 85%
		static const int maxNum = 10; //number of the biggest contours in the first step
		static const int border = 5; //thickness of the border of the image
		
	};

	
	
} // namespace pdp

#include "steps/inc/contour_extractor.txx"