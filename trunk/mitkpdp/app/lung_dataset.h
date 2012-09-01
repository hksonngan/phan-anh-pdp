#pragma once

#include "dicom_dir.h"
#include "steps/inc/slice_contour.h" 
#include "steps/inc/slice_thickenings.h" 
#include "steps/inc/thickening3d.h" 

#include <mitkStandaloneDataStorage.h>
#include <mitkDicomSeriesReader.h>
#include <QString>
#include <QDir>

namespace pdp {

/**
 * Container for all Lung Data and Results.
**/
class LungDataset
{
public:
    LungDataset();
    LungDataset(const LungDataset& other);
    virtual ~LungDataset();

    void clear();
    void clearAllButOriginal();
	void loadFromDicom(const DicomDir::Series* series, mitk::DicomSeriesReader::UpdateCallBackMethod callback = NULL);

    mitk::Image::Pointer getOriginalLungCTImage();
    mitk::Image::Pointer getImage(const QString& name);

    void addImage(mitk::Image::Pointer image, const QString& name);

    mitk::DataStorage::Pointer getDataStore();

	std::vector<sliceContour> slicesContours;
	std::vector<sliceThickenings> slicesThickenings;
	std::vector<Thickening3d> thickenings3d;

    // Call this whenever you added data to the store by accessing it directly using getDataStore.
    // It will update all the views.
    // Note that you don't need to call it in a step as it's called after all steps are done anyways.
    void updateGeometry();

    static const char* OriginalLungCTName;

public:
    mitk::StandaloneDataStorage::Pointer m_ds;
};

} // namespace pdp

