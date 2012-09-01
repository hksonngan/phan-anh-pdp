#ifndef MYTIMER_H
#define MYTIMER_H

#include <QThread>
#include <QTimer>

//#include "ThreeDEditing.h"

namespace pdp {

	/*class ThreeDEditing;*/

class MyTimer : public QObject
{
	Q_OBJECT
	public:
		MyTimer(/*ThreeDEditing* editing*/);
		~MyTimer();
	public slots:
		void start();
		//void stop();
		void TimerEvent();
	private:
		/*ThreeDEditing* m_ThreeDEditing;*/
		QTimer *m_Timer;
		int m_ElapsedTime;
};

} // namespace pdp

#endif // PDP_THREEDEDITING_H