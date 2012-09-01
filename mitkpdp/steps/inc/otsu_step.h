//Otsu step
#pragma once

#include "app/thickening_detection_step_base.h"
#include <QString>
#include "steps/inc/itk_otsu_threshold_image_filter.h"
#include "vtkFloatArray.h"
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkContourFilter.h>

namespace pdp {

	//volumetric range constraints for otsu thresholding
	//const float THORAX_LOWER_BOUND = 0.62;
	//const float THORAX_UPPER_BOUND = 0.68;

	const float THORAX_LOWER_BOUND = 0.55;
	const float THORAX_UPPER_BOUND = 0.68;

	//const float LUNGS_LOWER_BOUND = 0.22;
	//const float LUNGS_UPPER_BOUND = 0.38;

	const float LUNGS_LOWER_BOUND = 0.20;
	const float LUNGS_UPPER_BOUND = 0.40;

	class OtsuStep : public ThickeningDetectionStep
	{

	public:
		
		OtsuStep(QString info);

		template <class PixelType, int dim>
		typename itk::Image<unsigned char, dim>::Pointer SVRC_Otsu(typename itk::Image<PixelType, dim>::Pointer img, float lowerBound, float upperBound);
		template <class PixelType, int dim>
		typename itk::Image<unsigned char, dim>::Pointer SVRC_OtsuWithMask(typename itk::Image<PixelType, dim>::Pointer img, float lowerBound, float upperBound, typename itk::Image<unsigned char, 3>::Pointer mask);

		virtual QString name() const;
		virtual float relativePercent() const;
		virtual void work(LungDataset& input, LungDataset& output);
		/*for the 2D segmentation, we have to establish a look up table
		*to find the lower and upper bounds for the supervised range-constrained otsu
		*with 3D segmentation, we build the histogram from all slices
		*and just use one volumetric parameter for the thorax
		*and another volumetric parameter for the lungs
		*see thesis of Eduard Angelats Company for the parameter table at the end of the thesis
		*/
		
		typedef itk::Image<unsigned char, 3> BinImgType;
		typedef BinImgType::Pointer BinImgPointer;
		virtual BinImgPointer Morphologies(BinImgPointer img);

		itk::PolyLineParametricPath<2>::Pointer pdp::OtsuStep::throaxContour(itk::Image<unsigned char, 2>::Pointer img);

		vtkSmartPointer<vtkPolyData> pdp::OtsuStep::vtkThroaxContour(itk::Image<unsigned char, 2>::Pointer img);

		/*
		* this function get called inside the Morpholigies function
		*/
		itk::Image<unsigned char, 2>::Pointer pdp::OtsuStep::fillThroax(itk::Image<unsigned char, 2>::Pointer img);

		void setInputStepName(QString input)
		{
			inputStepName = input;
		}

		vtkSmartPointer<vtkPolyData> pdp::OtsuStep::itkContourToVtkContour(itk::PolyLineParametricPath<2>::Pointer contour, bool closeTheLoop);

		double pdp::OtsuStep::GetLength2 (double Bounds[6]);

		/**
		 * Produce throax. This can only be run after thresholding the lungs.
		 * this is the cleaning up procedure
		 *
		 * \param	lungs	The lungs. 
		 * \param	mask	The mask. 
		 * 
		**/
		void pdp::OtsuStep::produceThroax(itk::Image<unsigned char, 3>::Pointer img, itk::Image<unsigned char, 3>::Pointer mask);

		/**
		 * Labellings 3D connected components.
		 *
		 * \param	img	The thresholded image. 
		 *
		 * \return	the largest component as an 3d image. 
		**/

		itk::Image<unsigned char, 3>::Pointer pdp::OtsuStep::labelling(itk::Image<unsigned char, 3>::Pointer img);

	private:
		
		QString info;
		QString inputStepName;
		
	};

	
	
} // namespace pdp

#include "steps/inc/otsu_step.txx"