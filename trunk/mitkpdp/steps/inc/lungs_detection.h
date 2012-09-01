/*!
 * <This filter detect start and end slices between which the lungs are contained!!>
 * 
 * This uses the fixed HU of -550 for applying binary thresholding
 * to seperate the lungs from other tissue
 * 
 * the first and end slices containing a large enough amount of lungs are found
 * Copyright (c) 2011 by <Phan-Anh/PDP>
 */

#pragma once

#include "app/thickening_detection_step_base.h"
#include <itkBinaryThresholdImageFilter.h>

#include <QString>

namespace pdp {

	class LungsDetection : public ThickeningDetectionStep
	{

	public:
		
		LungsDetection(QString info);


		template <class PixelType, int dim>
		typename itk::Image<unsigned char, dim>::Pointer binaryThreshold(typename itk::Image<PixelType, dim>::Pointer img);

		virtual QString name() const;
		virtual float relativePercent() const;
		virtual void work(LungDataset& input, LungDataset& output);
		virtual void computeMinLungArea(itk::Image<unsigned char, 2>::Pointer img);

		/**
		* Calculates the throax region. 
		*
		* \param	img			The image. 
		* \param	sliceNo		The slice no. 
		* \param	pixelValue	The pixel value. 
		*
		* \return	The calculated throax region. 
		**/

		virtual itk::Image<unsigned char, 2>::RegionType computeThroaxRegion(itk::Image<unsigned char, 2>::Pointer img, float sliceNo, unsigned char pixelValue);

		virtual void GetSliceLungNorm( int nSliceLungStart, int nSliceLungEnd, int nSlice, float& nSliceNorm, QString& cLungField );


		int getLungStartSlice()
		{
			return m_LungStartSlice;
		}



		int getLungEndSlice()
		{
			return m_LungEndSlice;
		}

		void setInputStepName(QString input)
		{
			inputStepName = input;
		}
		
	private:
		
		QString info;
		QString inputStepName;
		int m_LungStartSlice, m_LungEndSlice, m_Slices;
		int minLungAreaUp; //e.g. 150000
		int minLungAreaDown;
	};
	
} // namespace pdp

#include "steps/inc/lungs_detection.txx"