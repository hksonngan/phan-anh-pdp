#pragma once

#include "app/thickening_detection_step_base.h"

#include <QString>

//! \todo remove?
//#include "steps/pdpItkSliceExtractor.h"

namespace pdp {

template <class PixelType> class visualizeData : public ThickeningDetectionStep {
public:
	typedef itk::Image<PixelType, 3>  ImageType;
	typedef itk::Image<PixelType, 2>  ImageType2D;
	typedef itk::PolyLineParametricPath<2> PathType;

    visualizeData(QString info);

    virtual QString name() const;
    virtual float relativePercent() const;
    virtual void work(LungDataset& input, LungDataset& output);
	typename ImageType::Pointer createImage(LungDataset& input, bool lungContours = 1, bool convexHull = 1, bool thickenings = 1, bool bboxes = 1, bool thicks3d = 1);
	void setInputStepName(QString input)
		{
			inputStepName = input;
		}

private:
	QString inputStepName;
    QString info;
};

} // namespace pdp
#include "steps/inc/visualize_data.txx"