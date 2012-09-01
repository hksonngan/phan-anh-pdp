#pragma once

#include "app/thickening_detection_step_base.h"

#include <mitkImageCast.h>
#include <QString>
#include <itkImageRegistrationMethod.h> 
#include <itkTranslationTransform.h>
#include <itkMeanSquaresImageToImageMetric.h> 
#include <itkLinearInterpolateImageFunction.h> 
#include <itkRegularStepGradientDescentOptimizer.h>
#include <itkCastImageFilter.h>
#include <itkEllipseSpatialObject.h>
#include <itkImage.h>
#include <itkImageRegistrationMethod.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkMeanSquaresImageToImageMetric.h>
#include <itkRegularStepGradientDescentOptimizer.h>
#include <itkResampleImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkSpatialObjectToImageFilter.h>
#include <itkTranslationTransform.h>

namespace pdp {

class registration : public ThickeningDetectionStep {
    public:
        registration(QString info);
        
        template <class PixelType, int dim>
        typename itk::Image<float, 3>::Pointer doIt(typename itk::Image<PixelType, dim>::Pointer fImg, typename itk::Image<PixelType, dim>::Pointer mImg);

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
};

} // namespace pdp

#include "steps/inc/registration.txx"