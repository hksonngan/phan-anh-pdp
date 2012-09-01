#include "steps/inc/registration.h"

#include <itkImageDuplicator.h>
#include <itkTranslationTransform.h>
#include <itkResampleImageFilter.h>
#include <itkCenteredAffineTransform.h>
#include <itkNearestNeighborInterpolateImageFunction.h>

/*//////
////////
Still some details to be corrected, like the names and types of the input/outputs
Also need to be tested on real input data
////////
*///////
pdp::registration::registration(QString info)
    : info(info)
{ }

QString pdp::registration::name() const
{
    return "Matching_registration: " + info;
}

float pdp::registration::relativePercent() const
{
    return 1.0;
}

void pdp::registration::work(LungDataset& input, LungDataset& output)
{
    std::cout << "performing the REGISTRATION step!" << std::endl;
    
    mitk::Image::Pointer mitkImage = input.getImage(inputStepName);
    mitkImage->DisconnectPipeline();
	itk::Image<float, 3>::Pointer mImg; //moving image 
    itk::Image<float, 3>::Pointer fImg; //fixed image
    mitk::CastToItkImage(mitkImage, mImg);
    
    //currently we're using duplicated imgs, just for TESTING
    //cloning itk image
    std::cout << "cloning starts!" << std::endl;
	typedef itk::ImageDuplicator< itk::Image<float, 3> > DuplicatorType;
    DuplicatorType::Pointer duplicator = DuplicatorType::New();
    duplicator->SetInputImage(mImg);
    duplicator->Update();
    fImg = duplicator->GetOutput();
    std::cout << "cloning done!" << std::endl;
    //cloning DONE
    
    
    /*
    itk::Image<float, 3>::IndexType pixelIndex;
    itk::Image<float, 3>::PixelType pixelVal;

    //mImg->SetPixel(pixelIndex, pixelVal+255);

    //debuggin: pixel values of the original
    for(int i=0;i<60;i++){
        pixelIndex[0] = 240+i;
        pixelIndex[1] = 200;
        pixelIndex[2] = 40;
        pixelVal=fImg->GetPixel(pixelIndex);
        std::cout << "pixel ("<<240+i<<",200,40)fImgbefore: " <<pixelVal<< std::endl;
    }*/

    //shifting, just for TESTING
    //typedef itk::ChangeInformationImageFilter<itk::Image<float, 3>> originChange; //for origin change, not used here
    
    typedef itk::ResampleImageFilter<itk::Image<float, 3>,itk::Image<float, 3>> FilterType;
    FilterType::Pointer filter = FilterType::New();

    typedef itk::CenteredAffineTransform<double,3> tt;
    tt::Pointer trans=tt::New();
    
    tt::OutputVectorType translation;
    translation[0] = -10; // X translation in millimeters
    translation[1] = 5; // Y translation in millimeters
    translation[2] = 2;
    trans->Translate( translation );

    filter->SetTransform( trans );

    typedef itk::NearestNeighborInterpolateImageFunction<itk::Image<float, 3>, double > InterpolatorType;

    InterpolatorType::Pointer interpolator = InterpolatorType::New();
    filter->SetInterpolator( interpolator );
    filter->SetDefaultPixelValue( 0 );

    filter->SetOutputSpacing( fImg->GetSpacing() );
    filter->SetOutputOrigin( fImg->GetOrigin() );

    itk::Image<float, 3>::SizeType size = fImg->GetLargestPossibleRegion().GetSize();;

    filter->SetSize( size );
    filter->SetInput(fImg);
    filter->Update();
    mImg=filter->GetOutput();

    std::cout << "shifting done!" << std::endl;
    //shifting DONE   
    
    emit stepProgress(0.1f);

    //registration itself
    itk::Image<float, 3>::Pointer imgOut = doIt<float, 3>(fImg, mImg);

    emit stepProgress(0.95f);
    mitk::Image::Pointer mitkOut = mitk::Image::New();
	mitk::CastToMitkImage(imgOut, mitkOut);
    output.addImage(mitkOut, "Registration");
    emit stepProgress(1.0f);
    
}
