#ifndef QCODINGCHAIN_H
#define QCODINGCHAIN_H

#include <qcodingchaincomponent.h>
#include <qcodecmanager.h>

class QCodingChain : public QThread
{

	Q_OBJECT

	signals:

		void finished();

	private slots:

		void inputFinished();
		void checkFinished();

	public:

		QCodingChain();

		void setInputFilePath(QString filePath);
		void setOutputFilePath(QString filePath);

		void run();

	protected:

		QAbstractCoder* detectCoder(QCodecContent &content);

	private:

		QString mInputFilePath;
		QString mOutputFilePath;

		QCodecManager mCoderManager;
		QAbstractCoder *mInputCoder;
		QAbstractCoder *mOutputCoder;

		QCodingChainInput *mInput;
		QCodingChainOutput *mOutput;

		QCodingChainFileInput mFileInput;
		QCodingChainFileOutput mFileOutput;

		QCodingChainDecoder mDecoder;
		QCodingChainEncoder mEncoder;

		QSharedBuffer mBuffer1;
		QSharedBuffer mBuffer2;
		QSharedBuffer mBuffer3;

		bool mInputAtEnd;
		bool mIsFinished;

int t;

};

#endif
