#pragma once

#include "app/lung_dataset.h"
#include <QObject>
#include <QString>
#include <boost/thread.hpp>
#include "omp.h"

namespace pdp {

class ThickeningDetectionStep : public QObject {
Q_OBJECT
public:
    virtual ~ThickeningDetectionStep() = 0 {};

    virtual QString name() const = 0;
    virtual float relativePercent() const = 0;
    virtual void work(LungDataset& input, LungDataset& output) = 0;

    inline void emitStepProgress(float totalpercent)
    {
        emit stepProgress(totalpercent);
    }

	int computeNumberOfLogicalProcessors()
	{
		int numberOfThreads = boost::thread::hardware_concurrency();
		omp_set_num_threads(numberOfThreads);
		std::cout << "number of threads set " << numberOfThreads <<std::endl;
		return numberOfThreads;
	}

signals:;
    void stepProgress(float totalpercent);

protected:
    ThickeningDetectionStep() {};
};

} // namespace pdp
