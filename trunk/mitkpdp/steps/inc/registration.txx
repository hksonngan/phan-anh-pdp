#include "app/itk_filter_progress_watcher.h"
#include <itkAbsoluteValueDifferenceImageFilter.h>
#include <itkGradientDescentOptimizer.h>


template <class PixelType, int dim>
typename itk::Image<float, 3>::Pointer pdp::registration::doIt(typename itk::Image<PixelType, dim>::Pointer fImg, typename itk::Image<PixelType, dim>::Pointer mImg)
{
    typedef itk::Image<PixelType, dim> InputImageType;

    //optimizer 
    typedef itk::RegularStepGradientDescentOptimizer OptimizerType;
         
    //transform 
    typedef itk::TranslationTransform< double, dim > TransformType;
     
      
    //metric 
    typedef itk::MeanSquaresImageToImageMetric<InputImageType,InputImageType > MetricType;
     
    //interpolator 
    typedef itk:: LinearInterpolateImageFunction<InputImageType,double> InterpolatorType;
     
    //registration
    typedef itk::ImageRegistrationMethod<InputImageType,InputImageType > RegistrationType;
     
    MetricType::Pointer         metric        = MetricType::New();
    TransformType::Pointer      transform     = TransformType::New();
    OptimizerType::Pointer      optimizer     = OptimizerType::New();
    InterpolatorType::Pointer   interpolator  = InterpolatorType::New();
    RegistrationType::Pointer   registration  = RegistrationType::New();
     
    registration->SetMetric(        metric        );
    registration->SetOptimizer(     optimizer     );
    registration->SetTransform(     transform     );
    registration->SetInterpolator(  interpolator  );

    registration->SetFixedImage(fImg);
    registration->SetMovingImage(mImg);

    //we're taking under consideration only part of the image
    InputImageType::SizeType sizeS = fImg->GetLargestPossibleRegion().GetSize();
    sizeS[0]=sizeS[0]/2; //currently only half of each size
    sizeS[0]=sizeS[0]/2;
    sizeS[0]=sizeS[0]/2;

    
    InputImageType::IndexType start;
    start[0] = 0;
    start[1] = 0;
    start[2] = 0;
    
    //evtl. to define the area more precisly we can use fixed points
    /*
    InputImageType::SizeType size;
    size[0] = 400;
    size[1] = 400;
    size[2] = 50;
    */
    InputImageType::RegionType region;
     
    region.SetIndex(start);
    region.SetSize(sizeS);

    //registration->SetFixedImageRegion(fImg->GetLargestPossibleRegion() ); //if we want to consider the whole image
    registration->SetFixedImageRegion(region );     //or just part of it
      
    //  Initialize the transform
    typedef RegistrationType::ParametersType ParametersType;
    ParametersType initialParameters( transform->GetNumberOfParameters() );

    initialParameters[0] = 0;  // Initial offset along X
    initialParameters[1] = 0.0;  // Initial offset along Y

    registration->SetInitialTransformParameters( initialParameters );

    optimizer->SetMaximumStepLength( 4.00 ); //best max 1.0; min 0.0
    optimizer->SetMinimumStepLength( 0.01 ); //up to 0.03 considering the whole image, part too, but we don't have to be so fast
    //stopping criterion
    optimizer->SetNumberOfIterations( 200 );

    itkFilterProgressWatcher watcher(registration, this, 0.1f, 0.95f);

    try
    {
        registration->Update();
    }
    catch( itk::ExceptionObject & err )
    {
        std::cerr << "ExceptionObject caught !" << std::endl;
        std::cerr << err << std::endl;
    }
    
    //printing results
    ParametersType finalParameters = registration->GetLastTransformParameters();

    const double TranslationAlongX = finalParameters[0];
    const double TranslationAlongY = finalParameters[1];
    const double TranslationAlongZ = finalParameters[2];
    const unsigned int numberOfIterations = optimizer->GetCurrentIteration();
    const double bestValue = optimizer->GetValue();

    std::cout << "Result = " << std::endl;
    std::cout << " Translation X = " << TranslationAlongX  << std::endl;
    std::cout << " Translation Y = " << TranslationAlongY  << std::endl;
    std::cout << " Translation Z = " << TranslationAlongZ  << std::endl;
    std::cout << " Iterations    = " << numberOfIterations << std::endl;
    std::cout << " Metric value  = " << bestValue          << std::endl;
    std::cout << " maxStepLenght  = " << optimizer->GetMaximumStepLength() << std::endl;
    std::cout << " minStepLenght  = " << optimizer->GetMinimumStepLength() << std::endl;
    //printing done

    //----TRANSLATING---- 
    typedef itk::ResampleImageFilter<InputImageType,InputImageType > ResampleFilterType;

    ResampleFilterType::Pointer resampler = ResampleFilterType::New();
    resampler->SetInput(mImg);
 
    //Note the use of the
    //  methods \code{GetOutput()} and \code{Get()}. This combination is needed
    //  here because the registration method acts as a filter whose output is a
    //  transform decorated in the form of a \doxygen{DataObject}. For details in
    //  this construction you may want to read the documentation of the
    //  \doxygen{DataObjectDecorator}.

    resampler->SetTransform( registration->GetOutput()->Get() );

    resampler->SetSize( fImg->GetLargestPossibleRegion().GetSize() );
    resampler->SetOutputOrigin(  fImg->GetOrigin() );
    resampler->SetOutputSpacing( fImg->GetSpacing() );
    resampler->SetOutputDirection( fImg->GetDirection() );
    resampler->SetDefaultPixelValue( 0 );
    resampler->Update();
    mImg=resampler->GetOutput();
    //----TRANSLATING DONE----
    
    
    //difference of the imgs
    /*
    typedef itk::AbsoluteValueDifferenceImageFilter<InputImageType, InputImageType,InputImageType >  difT;
    difT::Pointer dif=difT::New();
    dif->SetInput1(fImg);
    dif->SetInput2(mImg);
    dif->Update();
    return dif->GetOutput();
    */

    //return translated moving image
    return mImg;
}
