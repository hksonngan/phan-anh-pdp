#include "thickening_detector.h"

#include "app/thickening_detection_step_base.h"

// Add the include for your steps here.
#include "steps/inc/itk_slice_extractor.h"
#include "steps/inc/otsu_step.h"
#include "steps/inc/lungs_detection.h"
#include "steps/inc/contour_extractor.h"
#include "steps/inc/visualize_data.h"
#include "steps/inc/convex_hull.h"
#include "steps/inc/thickening_identification.h"
#include "steps/inc/em_classification.h"

#include <list>

pdp::ThickeningDetector::ThickeningDetector(LungDataset& input, LungDataset& output)
    : m_input(input)
    , m_output(output)
{
}

void pdp::ThickeningDetector::run()
{
    struct StepInfo {
        float realPercent;
        ThickeningDetectionStep* step;

        StepInfo(ThickeningDetectionStep* step) : step(step) {};
    };

    typedef std::list<StepInfo> StepList;
    StepList steps;

    // Add your steps here in a similar way.

	//using threshold to detect the lungs
	//the first and end slices containing a large enough amount of lungs are found
	LungsDetection* startEnd = new LungsDetection("detect start and end of the lungs");
	startEnd->setInputStepName("Original Lung CT");
	steps.push_back(startEnd);

	OtsuStep* otsu = new OtsuStep("detect start and end of the lungs");
	otsu->setInputStepName("Lungs");
	steps.push_back(otsu);

	//EMClassification* emClassifier = new EMClassification("detect start and end of the lungs");
	//emClassifier->setInputStepName("Otsu");
	//steps.push_back(emClassifier);
	
	//contourExtractor<double>* contour = new contourExtractor<double>("Extracting contours");
 //   contour->setInputStepName("Lungs");
 //   steps.push_back(contour);

 //   convexHull* convex = new convexHull("Computing the convex hull");
 //   steps.push_back(convex);
	//
	//thickeningsIdentification* thickIdent = new thickeningsIdentification("Identifying 3d thicks");
 //   steps.push_back(thickIdent);

	//visualizeData<double>* visualize = new visualizeData<double>("Visualizing data");
 //   steps.push_back(visualize);

    // Done adding steps, let's go!

    // Normalize the progress percents.
    float totalPercent = 0.0f;
    for(StepList::iterator iStep = steps.begin() ; iStep != steps.end() ; ++iStep) {
        totalPercent += iStep->step->relativePercent();
    }
    for(StepList::iterator iStep = steps.begin() ; iStep != steps.end() ; ++iStep) {
        iStep->realPercent = iStep->step->relativePercent() / totalPercent;
    }

    // Execute the steps.
    for(StepList::iterator iStep = steps.begin() ; iStep != steps.end() ; ++iStep) {
        QObject::connect(iStep->step, SIGNAL(stepProgress(float)),
                         this, SLOT(onStepProgress(float)));

        emit startStep(iStep->step->name(), iStep->realPercent);
		//! \todo exceptionhandling
		iStep->step->work(m_input, m_output);

        emit stepDone();

        // Cleanup
        QObject::disconnect(iStep->step, SIGNAL(stepProgress(float)),
                            this, SLOT(onStepProgress(float)));
        delete iStep->step;
    }

    this->exit();
}
 
void pdp::ThickeningDetector::exit()
{
    emit done();
}

void pdp::ThickeningDetector::onStepProgress(float totalpercent)
{
    emit stepProgress(totalpercent);
}
