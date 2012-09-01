#pragma once

#include <QtGui>
#include <QList>
#include <QString>
#include <QDir>

class QStandardItemModel;

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4396)
#endif
#include "gdcmItem.h"
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

namespace pdp {

/**
 * Maps DICOMDIR File to Memory, makes it accessible. 
 *
 * Our DICOMDIR File is structured like this:
 *	- Patient
 *		- Study
 *			- Series
 *				- Image
 * More information about Information Entries (IE) and Tags here:
 * http://www.medinfo-wiki.de/wiki/DICOM
**/
struct DicomDir {
	/**
	 * Used as Dataroles in Qt's QStandardTreeWidget.
	 * \see fillTreeModel
	**/
	enum DICOM_TREEVIEW_ROLES {
		DICOM_DATA_ROLE	= Qt::UserRole,
		DICOM_PATIENT_ROLE = Qt::UserRole + 1,
		DICOM_STUDY_ROLE = Qt::UserRole + 2,
		DICOM_SERIES_ROLE = Qt::UserRole + 3
	};

	//! Generic DICOMDIR IE defined by a tag
    struct Item {
        Item(const gdcm::Item& item);
        virtual ~Item() {};

        QString tagValue(const gdcm::Tag& tag) const;

    protected:
         const gdcm::Item& m_item;
    };

	//! Image IE
    struct Image : public Item {
        Image(const gdcm::Item& item);

        QString uid;
        QString path;
    };

	//! Series IE
    struct Series : public Item {
        Series(const gdcm::Item& item);
        QString uid;
        QString modality;
        QString description;

        QList<Image> images;
    };

	//! Study IE
    struct Study : public Item {
        Study(const gdcm::Item& item);
        void cleanup();

        QString uid;
        QString date;
        QString description;

        QList<Series*> series;
    };

	//! Patient IE
    struct Patient : public Item {
        Patient(const gdcm::Item& item);
        void cleanup();

        QString id;
        QString name;

        QList<Study*> studies;
    };

	DicomDir() {};
    DicomDir(const QString& dicomFile);
    DicomDir& operator =(const DicomDir& other);

	/**
	 * Fills a QT QStandardItemModel with our tree data.
	 *
	 * \param [in]	model 
	**/
	void fillTreeModel( QStandardItemModel* const);

    /**
     * Checks for useless data (empty studys etc.) and wipes that stuff away.
    **/
    void cleanup();

private:
	//! Contains Patients
    QList<Patient*> m_patients;
};

} // namespace pdp