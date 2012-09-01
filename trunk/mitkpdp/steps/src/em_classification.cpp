#include "steps/inc/em_classification.h"

pdp::EMClassification::EMClassification(QString info)
    : info(info)
{
	numberOfClasses = 4;
}

QString pdp::EMClassification::name() const
{
    return "Example step: " + info;
}

float pdp::EMClassification::relativePercent() const
{
    return 1.0;
}

void pdp::EMClassification::work(LungDataset& input, LungDataset& output)
{
    std::cout << "performing the expectation maximization step!" << std::endl;
	typedef itk::Image<float, 3> ImageType;
	typedef itk::Image<unsigned char, 3> MaskType;

	mitk::Image::Pointer mitkImage = input.getImage("Lungs");
	mitkImage->DisconnectPipeline();
	ImageType::Pointer img;
	mitk::CastToItkImage(mitkImage, img);

	mitk::Image::Pointer mitkMask = input.getImage("Otsu");
	mitkMask->DisconnectPipeline();
	MaskType::Pointer mask;
	mitk::CastToItkImage(mitkMask, mask);

	typedef itk::Vector< double, 1 > MeasurementVectorType;
	typedef itk::Statistics::ListSample< MeasurementVectorType > SampleType;

	SampleType::Pointer sample = prepareSample(img, mask);

    emit stepProgress(0.33f);

	//numberOfClasses = 4;

	typedef itk::Array< double > ParametersType;
	ParametersType params( 2 );
	std::vector< ParametersType > initialParameters( numberOfClasses );
	params[0] = -920.0;
	params[1] = 2500.0;//std = 50
	initialParameters[0] = params;
	params[0] = -835.0;
	params[1] = 1600.0;//std = 40
	initialParameters[1] = params;
	params[0] = -720.0;
	params[1] = 900.0;//std = 30
	initialParameters[2] = params;
	params[0] = -570.0;
	params[1] = 900.0;//std = 30
	initialParameters[3] = params;

	itk::Array< double > initialProportions(numberOfClasses);
	initialProportions[0] = 0.8;
	initialProportions[1] = 0.1;
	initialProportions[2] = 0.05;
	initialProportions[3] = 0.05;

	expectationMaximization(initialParameters, initialProportions, sample);

    emit stepProgress(0.66f);

	MaskType::Pointer nonLungTissueMask = classify(img, mask);

	// Cast the ITK -> MITK image and add it to the datatree.
	mitk::Image::Pointer mitkOut = mitk::Image::New();
	mitk::CastToMitkImage(nonLungTissueMask, mitkOut);
	output.addImage(mitkOut, "nonLungTissueMask");
	output.getDataStore()->Remove(output.getDataStore()->GetNamedNode("Otsu"));

    emit stepProgress(1.00f);
}


itk::Image<unsigned char, 3>::Pointer pdp::EMClassification::classify(itk::Image<float, 3>::Pointer img, itk::Image<unsigned char, 3>::Pointer mask)
{
	typedef itk::Vector< double, 1 > MeasurementVectorType;
	typedef itk::Image<unsigned char, 3> MaskType;
	typedef itk::Image<float, 3> ImageType;

	typedef itk::ImageRegionConstIterator< ImageType > ImageIteratorType;
	ImageIteratorType imgIt(img, img->GetLargestPossibleRegion());

	typedef itk::ImageRegionConstIterator< MaskType> MaskIteratorType;
	MaskIteratorType maskIt( mask, mask->GetLargestPossibleRegion());

	MaskType::Pointer correctImage = MaskType::New();
	correctImage->CopyInformation(mask);
	MaskType::RegionType outputRegion = mask->GetLargestPossibleRegion();
	correctImage->SetRegions( outputRegion );
	correctImage->Allocate();
	typedef itk::ImageRegionIterator<MaskType> IteratorType;
	IteratorType outputIt( correctImage, outputRegion);

	MeasurementVectorType mv;
	for ( imgIt.GoToBegin(), maskIt.GoToBegin(), outputIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt, ++imgIt, ++outputIt)
	{

		if (maskIt.Get() == 255)
		{
			mv[0] = imgIt.Get();
			if (genLabel(mv) == 3)
			{
				outputIt.Set(255);
			}
		}
	}

	return correctImage;
}


unsigned char pdp::EMClassification::genLabel(itk::Vector< double, 1 > dataPoint)
{
	typedef itk::Vector< double, 1 > MeasurementVectorType;
	//typedef itk::Statistics::GaussianDensityFunction< MeasurementVectorType > MembershipFunctionType;
	typedef itk::Statistics::MembershipFunctionBase< MeasurementVectorType > MembershipFunctionType;
	typedef itk::Statistics::ListSample< MeasurementVectorType > SampleType;
	typedef itk::Statistics::GaussianMixtureModelComponent< SampleType > ComponentType;
	typedef itk::MaximumRatioDecisionRule DecisionRuleType;

	std::vector< double > discriminantScores;
	DecisionRuleType::APrioriVectorType aPrioris;
	const itk::Array< double >* estimatedProportions = estimator->GetProportions();
	DecisionRuleType::Pointer decisionRule = DecisionRuleType::New();

	for (int i = 0; i < numberOfClasses; i++)
	{
		MembershipFunctionType * likelihood = components[i]->GetMembershipFunction ();
		discriminantScores.push_back(likelihood->Evaluate(dataPoint));
		aPrioris.push_back(estimatedProportions[0][i]);
	}

	decisionRule->SetAPriori( aPrioris );
	return decisionRule->Evaluate( discriminantScores );

}


itk::Statistics::ListSample< itk::Vector< double, 1 > >::Pointer pdp::EMClassification::prepareSample(itk::Image<float, 3>::Pointer img, itk::Image<unsigned char, 3>::Pointer mask)
{

	typedef itk::Vector< double, 1 > MeasurementVectorType;
	typedef itk::Statistics::ListSample< MeasurementVectorType > SampleType;
	typedef itk::Image<unsigned char, 3> MaskType;
	typedef itk::Image<float, 3> ImageType;

	SampleType::Pointer sample = SampleType::New();

	typedef itk::ImageRegionConstIterator< ImageType > ImageIteratorType;
	ImageIteratorType imgIt(img, img->GetLargestPossibleRegion());

	typedef itk::ImageRegionConstIterator< MaskType> MaskIteratorType;
	MaskIteratorType maskIt( mask, mask->GetLargestPossibleRegion());

	MeasurementVectorType mv;
	for ( imgIt.GoToBegin(), maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt, ++imgIt)
	{

		if (maskIt.Get() == 255)
		{
			mv[0] = imgIt.Get();
			sample->PushBack( mv );
		}
	}

	return sample;
}

void pdp::EMClassification::expectationMaximization(std::vector< itk::Array< double > > initialParameters, itk::Array< double > initProportion, itk::Statistics::ListSample< itk::Vector< double, 1 > >::Pointer sample)
{

	typedef itk::Vector< double, 1 > MeasurementVectorType;
	typedef itk::Statistics::ListSample< MeasurementVectorType > SampleType;
	typedef itk::Statistics::GaussianMixtureModelComponent< SampleType > ComponentType;

	for ( unsigned int i = 0 ; i < numberOfClasses ; i++ )
	{
		components.push_back( ComponentType::New() );
		(components[i])->SetSample( sample );
		(components[i])->SetParameters( initialParameters[i] );
	}

	typedef itk::Statistics::ExpectationMaximizationMixtureModelEstimator< SampleType > EstimatorType;
	estimator = EstimatorType::New();

	estimator->SetSample( sample );
	estimator->SetMaximumIteration( 30 );

	estimator->SetInitialProportions( initProportion );
	for ( unsigned int i = 0 ; i < numberOfClasses ; i++)
	{
		estimator->AddComponent( (ComponentType::Superclass*)
			(components[i]).GetPointer() );
	}

	std::cout << "Start estimating..." << std::endl;
	estimator->Update();
	std::cout << "End of estimation!" << std::endl;

	const itk::Array< double >* estimatedProportions = estimator->GetProportions();
	std::cout << "    Proportion: ";
	std::cout << "         " << estimatedProportions[0] << std::endl;
	//std::cout << "         " << estimator->GetProportions()[0] << std::endl;

	// Output the results
	for ( unsigned int i = 0 ; i < numberOfClasses ; i++ )
	{
		std::cout << "Cluster[" << i << "]" << std::endl;
		std::cout << "    Parameters:" << std::endl;
		std::cout << "         " << (components[i])->GetFullParameters() << std::endl;
	}

}