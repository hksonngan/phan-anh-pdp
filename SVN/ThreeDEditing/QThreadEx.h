class QThreadEx : public QThread
{
	protected:
		void run() { exec(); while(1){;};};
};