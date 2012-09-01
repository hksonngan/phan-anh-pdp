#include "lung_dataset.h"
#include "dicom_dir.h"

#include <QMessageBox>

// Reading DICOM files trials.
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <mitkDicomSeriesReader.h>
#include <mitkRenderingManager.h>

const char* pdp::LungDataset::OriginalLungCTName = "Original Lung CT";

pdp::LungDataset::LungDataset()
    : m_ds(mitk::StandaloneDataStorage::New())
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;
    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

pdp::LungDataset::LungDataset(const LungDataset& other)
    : m_ds(other.m_ds)
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;
    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

pdp::LungDataset::~LungDataset()
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;
    // Do not call clear here!
    // The datastore will clear automatically when its reference count is 0.
    // The destructor may be called several times in the meantime, for example
    // when switching to another tab.
    //this->clear();
    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::LungDataset::clear()
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

    // Remove all the nodes from the datatree.
    while(!m_ds->GetAll()->empty()) {
        m_ds->Remove(m_ds->GetAll()->front());
    }

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::LungDataset::clearAllButOriginal()
{
    std::cout << "hi from " << __FUNCSIG__ << std::endl;

    // Remove all the nodes from the datatree, except for the original data.
    // Note: you can't delete while iterating, thus first collect the ones to remove, then remove them.
    typedef std::set<const mitk::DataNode*> NodeSet;
    NodeSet nodesToDelete;
    mitk::StandaloneDataStorage::SetOfObjects::ConstPointer allNodes = m_ds->GetAll();
    for(mitk::StandaloneDataStorage::SetOfObjects::const_iterator iNode = allNodes->begin() ; iNode != allNodes->end() ; ++iNode) {
        if((*iNode)->GetName() != OriginalLungCTName) {
            nodesToDelete.insert(*iNode);
        }
    }

    for(NodeSet::iterator iNode = nodesToDelete.begin() ; iNode != nodesToDelete.end() ; ++iNode) {
        m_ds->Remove(*iNode);
    }

    std::cout << "ciao from " << __FUNCSIG__ << std::endl;
}

void pdp::LungDataset::loadFromDicom(const DicomDir::Series* series, mitk::DicomSeriesReader::UpdateCallBackMethod callback)
{
    mitk::DicomSeriesReader::StringContainer seriesToLoad;
    for(QList<pdp::DicomDir::Image>::const_iterator i = series->images.begin() ; i != series->images.end() ; ++i) {
        seriesToLoad.push_back(i->path.toStdString());
    }

    // final step: load into DataNode (can result in 3D+t image)
	mitk::DataNode::Pointer node = mitk::DicomSeriesReader::LoadDicomSeries(seriesToLoad, true, false, callback);
    if(node) {
        node->SetName(OriginalLungCTName);
        m_ds->Add(node);
    }

    this->updateGeometry();
}

mitk::Image::Pointer pdp::LungDataset::getOriginalLungCTImage()
{
    mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_ds->GetNamedNode(OriginalLungCTName)->GetData());
    if(!img)
        throw std::runtime_error("There is no Original Lung CT image node!");

    return img;
}

mitk::Image::Pointer pdp::LungDataset::getImage(const QString& name)
{
    return dynamic_cast<mitk::Image*>(this->getDataStore()->GetNamedNode(name.toStdString().c_str())->GetData());
}

void pdp::LungDataset::addImage(mitk::Image::Pointer image, const QString& name)
{
	mitk::DataNode::Pointer node = mitk::DataNode::New();
	node->SetData(image);
    node->SetName(name.toStdString());
	this->getDataStore()->Add(node);
}

mitk::DataStorage::Pointer pdp::LungDataset::getDataStore()
{
    return mitk::DataStorage::Pointer(m_ds);
}

void pdp::LungDataset::updateGeometry()
{
    // Update (zoomfit) the views
    mitk::TimeSlicedGeometry::Pointer geo = m_ds->ComputeBoundingGeometry3D(m_ds->GetAll());
    mitk::RenderingManager::GetInstance()->InitializeViews(geo);
}

