#include "dicom_dir.h"

#include <QIcon>
#include <QDir>
#include <QFileInfo>
#include <QStandardItemModel>
#include <QStandardItem>

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4396)
#endif
#include "gdcmReader.h"
#include "gdcmMediaStorage.h"
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

static QString valueToStr(const gdcm::DataElement& elem)
{
    std::stringstream str;
    elem.GetValue().Print(str);
    return QString(str.str().c_str()).trimmed();
}

pdp::DicomDir::DicomDir(const QString &dicomFile)
{
    // First we do a series of check to see if it is a valid dicomdir file...
    gdcm::Reader reader;
    reader.SetFileName(dicomFile.toAscii());
    if(!reader.Read()) {
		throw std::runtime_error(QObject::tr("Fehler beim lesen der DICOM Daten. Eventuell sind die Daten korrupt.").toStdString());
    }

    gdcm::MediaStorage ms;
    ms.SetFromFile(reader.GetFile());
    if(ms != gdcm::MediaStorage::MediaStorageDirectoryStorage) {
		throw std::runtime_error(QObject::tr("Fehler beim lesen der DICOM Daten. Eventuell ist die DICOMDIR Datei kaputt.").toStdString());
    }

    gdcm::FileMetaInformation &metainfo = reader.GetFile().GetHeader();
    if(metainfo.FindDataElement(gdcm::Tag(0x0002, 0x0002))) {
        QString tagID = valueToStr(metainfo.GetDataElement(gdcm::Tag(0x0002, 0x0002)));
        if(tagID != "1.2.840.10008.1.3.10") {
			throw std::runtime_error(QObject::tr("Fehler beim lesen der DICOM Daten. Eventuell ist die DICOMDIR Datei kaputt. (Falsche TAG-ID: %1)").arg(tagID).toStdString());
        }
        //std::cerr << "Media Storage Sop Class UID: " << strm << std::endl;
    } else {
		throw std::runtime_error(QObject::tr("Fehler beim lesen der DICOM Daten. Eventuell ist die DICOMDIR Datei kaputt. (Keine Sop Class UID verfügbar.)").toStdString());
    }

    // Now that we are pretty confident that it is at least a dicomdir file, walk through it.

    // There is no real hierarchy in the dicomdir file. The hierarchy is only logical in the sense
    // that it's a flat list of items and we give them meaning depending on what has been seen before:
    // patient
    // study
    // series
    // image
    // image
    // image
    // series
    // image
    // image
    // ...
    //
    // Best way to read such a file is using a pseudo-state machine. (A real one would be "better", but I'm bored.)
    // (Where the state is my datastructure/list of patients).
    //
    // Say "hi" to Demeter.

    const gdcm::DataSet::DataElementSet& elementSet = reader.GetFile().GetDataSet().GetDES();
    for(std::set<gdcm::DataElement>::const_iterator iDataElement = elementSet.begin() ; iDataElement != elementSet.end(); ++iDataElement) {
        if(iDataElement->GetTag() != gdcm::Tag(0x0004, 0x1220)) // DirectoryRecordSequence : SQ
            continue;
		
		// "Cast" current Item to Sequence-Type
        gdcm::SmartPointer<gdcm::SequenceOfItems> sqi = iDataElement->GetValueAsSQ();

        for(unsigned int curritemID = 1 ; curritemID <= sqi->GetNumberOfItems() ; ++curritemID) {
            gdcm::Item& currItem = sqi->GetItem(curritemID);

            gdcm::Tag typenameTag(0x0004, 0x1430); // Directory Record Type
            Item item(currItem);

            if(item.tagValue(typenameTag).toUpper() == "PATIENT") {
                m_patients.append(new Patient(currItem));
            } else if(item.tagValue(typenameTag).toUpper() == "STUDY") {
                if(m_patients.size() == 0) {
                    std::cerr << "Warning: skipping orphan study" << std::endl;
                    continue;
                }

                m_patients.last()->studies.append(new Study(currItem));
            } else if(item.tagValue(typenameTag).toUpper() == "SERIES") {
                if(m_patients.size() == 0 || m_patients.last()->studies.size() == 0) {
                    std::cerr << "Warning: skipping orphan series" << std::endl;
                    continue;
                }

                m_patients.last()->studies.last()->series.append(new Series(currItem));
            } else if(item.tagValue(typenameTag).toUpper() == "IMAGE") {
                if(m_patients.size() == 0 || m_patients.last()->studies.size() == 0 || m_patients.last()->studies.last()->series.size() == 0) {
                    std::cerr << "Warning: skipping orphan image" << std::endl;
                    continue;
                }

                m_patients.last()->studies.last()->series.last()->images.append(Image(currItem));

                // In addition, we need to prefix the path to the DICOMDIR file to have a full image path.
                Image& img = m_patients.last()->studies.last()->series.last()->images.last();
                
				QFileInfo dicomFileInfo(dicomFile);
				img.path = dicomFileInfo.absoluteDir().absoluteFilePath(img.path);
            } else {
                // Skip unknown/unnecessary items.
            }

        } // While data element got more items
    } // For data element in set

    this->cleanup();
}

void pdp::DicomDir::cleanup()
{
	foreach(Patient* patient, m_patients) {
		patient->cleanup();
        // Remove all pations which have no studies.
		if(patient->studies.isEmpty()) {
			m_patients.removeOne(patient);
        }
    }
}

pdp::DicomDir::Patient::Patient(const gdcm::Item &item)
    : Item(item)
{
    this->id = this->tagValue(gdcm::Tag(0x0010, 0x0020));
	this->name = this->tagValue(gdcm::Tag(0x0010, 0x0010));
}

void pdp::DicomDir::Patient::cleanup()
{
   	foreach(Study* study, studies) {
		study->cleanup();
        // Remove all pations which have no studies.
		if(study->series.isEmpty()) {
            studies.removeOne(study);
        }
    }
}

pdp::DicomDir::Study::Study(const gdcm::Item &item)
    : Item(item)
{
    this->uid = this->tagValue(gdcm::Tag(0x0020, 0x000d));
    this->date = this->tagValue(gdcm::Tag(0x0008, 0x0020));
    this->description = this->tagValue(gdcm::Tag(0x0008, 0x1030));
}

void pdp::DicomDir::Study::cleanup()
{
	foreach(Series* serie, series) {
        // Remove all pations which have no studies.
		if(serie->images.count() < 20) {
            series.removeOne(serie);
        }
    }
}

pdp::DicomDir::Series::Series(const gdcm::Item& item)
    : Item(item)
{
    this->uid = this->tagValue(gdcm::Tag(0x0020, 0x000e));
    this->modality = this->tagValue(gdcm::Tag(0x0008, 0x0060));
    this->description = this->tagValue(gdcm::Tag(0x0008, 0x103e));
}

pdp::DicomDir::Image::Image(const gdcm::Item &item)
    : Item(item)
{
    this->uid = this->tagValue(gdcm::Tag(0x0004, 0x1511));
    this->path = this->tagValue(gdcm::Tag(0x0004, 0x1500));
}

pdp::DicomDir::Item::Item(const gdcm::Item& item)
    : m_item(item)
{ }


QString pdp::DicomDir::Item::tagValue(const gdcm::Tag &tag) const
{
    if(!m_item.FindDataElement(tag))
        return "";

    return valueToStr(m_item.GetDataElement(tag));
}

void pdp::DicomDir::fillTreeModel (QStandardItemModel* const model) {
	// Swept old data
	model->removeRows(0, model->rowCount());

	QStandardItem *parentItem = model->invisibleRootItem();

	foreach(Patient* currentPatient, m_patients) {
		//: Referes to Patient Name in Treeview
		QStandardItem* patientItem = new QStandardItem(QIcon(":/common/res/common/user.png"), QString("%1").arg(currentPatient->name));
		patientItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

		// Bind Data to Tree Item (evaluated in OnSelect Event)
		patientItem->setData(QVariant(DICOM_PATIENT_ROLE), DICOM_DATA_ROLE);
		patientItem->setData(QVariant::fromValue<void*>(currentPatient), DICOM_PATIENT_ROLE);

		foreach(Study* currentStudy, currentPatient->studies) {
			QStandardItem* studyItem = new QStandardItem(QIcon(), QString("%1").arg(currentStudy->date));
			studyItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

			studyItem->setData(QVariant(DICOM_STUDY_ROLE), DICOM_DATA_ROLE);
			studyItem->setData(QVariant::fromValue<void *>(currentStudy), DICOM_STUDY_ROLE);
			
			foreach(Series* currentSeries, currentStudy->series) {
				QStandardItem* seriesItem = new QStandardItem(QIcon(), QString("%1").arg(currentSeries->modality));
				seriesItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

				seriesItem->setData(QVariant(DICOM_SERIES_ROLE), DICOM_DATA_ROLE);
				seriesItem->setData(QVariant::fromValue<void*>(currentSeries), DICOM_SERIES_ROLE);
				studyItem->appendRow(seriesItem);
			}

			patientItem->appendRow(studyItem);
		}

		parentItem->appendRow(patientItem);
	}
}

pdp::DicomDir& pdp::DicomDir::operator =(const DicomDir& other)
{
    if(this == &other)
        return *this;

    m_patients = other.m_patients;

    return *this;
}