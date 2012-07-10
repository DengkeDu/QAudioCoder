#include <qcodingchaincomponent.h>

#define CHUNK_SIZE 8192

/**********************************************************
QCodingChainComponent
**********************************************************/

QCodingChainComponent::QCodingChainComponent()
	: QObject()
{
	mNext = NULL;
}

void QCodingChainComponent::setNext(QCodingChainComponent *next)
{
	mNext = next;
}

int QCodingChainComponent::chunkSize()
{
	return CHUNK_SIZE;
}

void QCodingChainComponent::changeFormat(QExtendedAudioFormat format)
{
}

void QCodingChainComponent::addData(QSampleArray *data)
{
	mData.enqueue(data);
	execute();
}

/**********************************************************
QCodingChainInput
**********************************************************/

QCodingChainInput::QCodingChainInput()
	: QCodingChainComponent()
{
}

/**********************************************************
QCodingChainFileInput
**********************************************************/

QCodingChainFileInput::QCodingChainFileInput()
	: QCodingChainInput()
{
	mFilePath = "";
}

void QCodingChainFileInput::setFilePath(QString filePath)
{
	mFilePath = filePath;
}

bool QCodingChainFileInput::hasData()
{
	return !mFile.atEnd();
}

int QCodingChainFileInput::size()
{
	return mFile.size();
}

void QCodingChainFileInput::initialize()
{
	mFile.setFileName(mFilePath);
	if(!mFile.open(QIODevice::ReadOnly))
	{
		return;
	}
}

void QCodingChainFileInput::execute()
{
	char *data = new char[CHUNK_SIZE];
	int size = mFile.read(data, CHUNK_SIZE);
	mNext->addData(new QSampleArray(data, size));
}

void QCodingChainFileInput::finalize()
{
	mFile.close();
}

/**********************************************************
QCodingChainCoder
**********************************************************/

QCodingChainCoder::QCodingChainCoder()
	: QCodingChainComponent()
{
	mCoder = NULL;
}

void QCodingChainCoder::setCoder(QAbstractCoder *coder)
{
	mCoder = coder;
}

/**********************************************************
QCodingChainDecoder
**********************************************************/

QCodingChainDecoder::QCodingChainDecoder()
	: QCodingChainCoder()
{
}

void QCodingChainDecoder::setCoder(QAbstractCoder *coder)
{
	QCodingChainCoder::setCoder(coder);
	QObject::connect(coder, SIGNAL(formatChanged(QExtendedAudioFormat)), mNext, SLOT(changeFormat(QExtendedAudioFormat)), Qt::DirectConnection);
}

void QCodingChainDecoder::initialize()
{
	if(mCoder != NULL && mCoder->initializeDecode())
	{
		QObject::connect(mCoder, SIGNAL(decoded(QSampleArray*)), mNext, SLOT(addData(QSampleArray*)), Qt::DirectConnection);
	}
}

void QCodingChainDecoder::finalize()
{
	if(mCoder != NULL && mCoder->finalizeDecode())
	{
		QObject::disconnect(mCoder, SIGNAL(decoded(QSampleArray*)));
	}
}

void QCodingChainDecoder::execute()
{
	QSampleArray *array = mData.dequeue();
	mCoder->decode(array->data(), array->size());
	delete array;
}

/**********************************************************
QCodingChainEncoder
**********************************************************/

QCodingChainEncoder::QCodingChainEncoder()
	: QCodingChainCoder()
{
}

void QCodingChainEncoder::changeFormat(QExtendedAudioFormat format)
{
	if(mCoder != NULL)
	{
		mCoder->setFormat(QAudio::AudioInput, format);
		if(mCoder->initializeEncode())
		{
			QObject::connect(mCoder, SIGNAL(encoded(QSampleArray*)), mNext, SLOT(addData(QSampleArray*)), Qt::DirectConnection);
		}
	}
}

void QCodingChainEncoder::initialize()
{
}

void QCodingChainEncoder::finalize()
{
	if(mCoder != NULL && mCoder->finalizeEncode())
	{
		QObject::disconnect(mCoder, SIGNAL(encoded(QSampleArray*)));
	}
}

void QCodingChainEncoder::execute()
{
	QSampleArray *array = mData.dequeue();
	mCoder->encode(array->data(), array->samples());
	delete array;
}

/**********************************************************
QCodingChainOutput
**********************************************************/

QCodingChainOutput::QCodingChainOutput()
	: QCodingChainComponent()
{
}

void QCodingChainOutput::setHeader(QByteArray data)
{
	mHeader = data;
}

/**********************************************************
QCodingChainFileOutput
**********************************************************/

QCodingChainFileOutput::QCodingChainFileOutput()
	: QCodingChainOutput()
{
	mFilePath = "";
}

void QCodingChainFileOutput::setFilePath(QString filePath)
{
	mFilePath = filePath;
}

void QCodingChainFileOutput::initialize()
{
	mFile.setFileName(mFilePath);
	mFile.open(QIODevice::WriteOnly);
}

void QCodingChainFileOutput::finalize()
{
cout<<"size: "<<mFile.size()<<endl;
	mFile.seek(0);
	mFile.write(mHeader.data(), 44);
	mFile.close();
cout<<"size: "<<mFile.size()<<endl;
}

void QCodingChainFileOutput::execute()
{
	QSampleArray *array = mData.dequeue();
	mFile.write(array->charData(), array->size());
	delete array;
}
