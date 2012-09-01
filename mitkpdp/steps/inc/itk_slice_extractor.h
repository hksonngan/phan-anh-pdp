//extract each slice from an itk image
//it also support the assembly of 2D slices into a 3D image
//
#pragma once

#include "app/thickening_detection_step_base.h"
#include "steps/inc/itk_trace_path_on_image_filter.h"

//#include "itkPathToImageFilter.h"
#include <itkContourExtractor2DImageFilter.h>
#include <itkTileImageFilter.h>
#include <itkExtractImageFilter.h>
#include <vector>
#include <itkImage.h>
#include <itkPolyLineParametricPath.h>
#include <QString>

namespace pdp {

	//slices can be modified outside of this class
	template <class PixelType> class pdpItkSliceExtractor : public ThickeningDetectionStep
	{

	public:
	
		pdpItkSliceExtractor(QString info);

		typedef itk::Image<PixelType, 3>  ImageType;
		typedef itk::Image<PixelType, 2>  ImageType2D;
		typedef itk::PolyLineParametricPath<2> PathType;
		
		void extractSlice(typename itk::Image<PixelType, 3>::Pointer img);
		
		//assemble whatever in the slice list
		//even the slice data have been changed
		typename ImageType::Pointer assembleSlice();

		typename ImageType::Pointer assembleSlice(int start, int end);

		//void demoConvexHull(LungDataset& input);

		int getNumberOfSlices()
		{
			return sliceList.size();
		}

		typename ImageType2D::Pointer getSlice(int sliceNumber);

		void setSlice(int sliceNumber, typename ImageType2D::Pointer slice);
		
		//not used, one below uses it
		typename ImageType2D::Pointer drawLine(typename PathType::Pointer bbox, typename ImageType2D::Pointer img);
		void setPathToImageParam(LungDataset& input);
		//currently not used
		typename ImageType2D::Pointer itkPathToImage(std::vector<typename PathType::Pointer> paths);
		
		//typename ImageType2D::Pointer manualPathToImage(std::vector<typename itk::PolyLineParametricPath<2>::Pointer> paths, typename ImageType::SpacingType spacing3 = spacing, typename ImageType::PointType origin3 = origin, typename ImageType2D::RegionType region2 = region);
		typename ImageType2D::Pointer manualPathToImage(std::vector<typename itk::PolyLineParametricPath<2>::Pointer> paths);
		
		virtual QString name() const;
		virtual float relativePercent() const;
		virtual void work(LungDataset& input, LungDataset& output);
		
		void setInputStepName(QString input)
		{
			inputStepName = input;
		}

		std::vector<typename ImageType2D::Pointer> sliceList;
	private:
		

		
		//chose index = slice number, size = 0
		typename ImageType::SizeType size;
		typename ImageType::IndexType index;
		typename ImageType2D::SizeType size2D;
		typename ImageType::SpacingType spacing;
		typename ImageType::PointType origin;
		typename ImageType2D::RegionType region;
		
		
		QString info;
		QString inputStepName;

		//! This image created to hold the meta information of the original image
		//! or we can use the changeImageInformationFilter to manually adjust the image information

		typename ImageType::Pointer referencedImage;
		
	};

}
	
#include "steps/inc/itk_slice_extractor.txx"