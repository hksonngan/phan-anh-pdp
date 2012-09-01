#ifndef TIMERTHREAD_H
#define TIMERTHREAD_H

#include <QThread>
#include <QTimer>

//#include "ThreeDEditing.h"

namespace pdp {

	/*class ThreeDEditing;*/

class TimerThread : public QThread
{
	Q_OBJECT
	public:
		TimerThread(/*ThreeDEditing* editing*/);
		~TimerThread();
		void run();
	public slots:
		//void start();
		//void stop();
		void TimerEvent();
	private:
		/*ThreeDEditing* m_ThreeDEditing;*/
		QTimer *m_Timer;
		int m_ElapsedTime;
};

} // namespace pdp

#endif // PDP_THREEDEDITING_H