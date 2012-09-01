#pragma once

#include "app/thickening_detection_step_base.h"

#include "itkVector.h"
#include "itkListSample.h"
#include "itkGaussianMixtureModelComponent.h"
#include "itkExpectationMaximizationMixtureModelEstimator.h"
#include "itkMaximumRatioDecisionRule.h"
#include <mitkImageCast.h>
#include "itkMaximumRatioDecisionRule.h"
#include "itkGaussianDensityFunction.h"
#include <itkMembershipFunctionBase.h>
#include <vector>
#include <QString>

namespace pdp {

class EMClassification : public ThickeningDetectionStep {
public:
    EMClassification(QString info);

    virtual QString name() const;
    virtual float relativePercent() const;
    virtual void work(LungDataset& input, LungDataset& output);

	void setInputStepName(QString input)
	{
		inputStepName = input;
	}

	void setNumberOfClasses(unsigned char numberOfComponents)
	{
		numberOfClasses = numberOfComponents;
	}

	typedef itk::Vector< double, 1 > MeasurementVectorType;
	typedef itk::Statistics::ListSample< MeasurementVectorType > SampleType;
	typedef itk::Image<unsigned char, 3> MaskType;
	typedef itk::Image<float, 3> ImageType;
	typedef itk::Statistics::GaussianMixtureModelComponent< SampleType > ComponentType;
	typedef itk::Statistics::ExpectationMaximizationMixtureModelEstimator< SampleType > EstimatorType;

	/**
	 * Prepare sample input data by picking the masked pixel values. 
	 *
	 * \param	img		CT image. 
	 * \param	mask	Lungs mask. 
	 *
	 * \return	sample input data for the expectation maximization estimator. 
	**/
	SampleType::Pointer prepareSample(ImageType::Pointer img, MaskType::Pointer mask);

	/**
	 * Estimate components' parameters using Expectation maximization algorithm. 
	 * This will update the variable components which contain the correct parameters.
	 * The final proportions are taken from estimator->GetProportions().
	 *
	 * \param	initProportion	The initial proportion for each component. 
	**/
	void expectationMaximization(std::vector< itk::Array< double > > initialParameters, itk::Array< double > initProportion, SampleType::Pointer sample);

	/**
	 * Generate label for a data point based on MaximumRatioDecisionRule using estimated parameters. 
	 * See page 640 and 683 of the ITK software guide.
	 *
	 * \param	dataPoint	voxel value. 
	 *
	 * \return	class label of the data point. 
	**/
	unsigned char genLabel(MeasurementVectorType dataPoint);

	/**
	 * Classification of non-lungs tissues based on EM and Bayes decision rule. 
	 *
	 * \param	img		The image. 
	 * \param	mask	The mask of initial segmented lungs. 
	 *
	 * \return	mask of non-lungs tissues. 
	**/
	MaskType::Pointer classify(ImageType::Pointer img, MaskType::Pointer mask);

private:
    QString info;
	QString inputStepName;

	unsigned char numberOfClasses;

	std::vector< ComponentType::Pointer > components;
	EstimatorType::Pointer estimator;

};

} // namespace pdp
