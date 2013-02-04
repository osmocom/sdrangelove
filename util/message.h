#ifndef INCLUDE_MESSAGE_H
#define INCLUDE_MESSAGE_H

#include <QAtomicInt>

class MessageQueue;
class QWaitCondition;
class QMutex;

class Message {
public:
	Message();
	virtual ~Message();

	void submit(MessageQueue* queue);
	int execute(MessageQueue* queue);

	void completed(int result = 0);

	virtual int type() const = 0;
	virtual const char* name() const = 0;

protected:
	// stuff for synchronous messages
	bool m_synchronous;
	QWaitCondition* m_waitCondition;
	QMutex* m_mutex;
	QAtomicInt m_complete;
	int m_result;
};

#endif // INCLUDE_MESSAGE_H
