#ifndef INCLUDE_MESSAGE_H
#define INCLUDE_MESSAGE_H

#include <QAtomicInt>
#include <QStringList>
#include "util/export.h"

class MessageQueue;
class QWaitCondition;
class QMutex;

class SDRANGELOVE_API MessageRegistrator {
public:
	MessageRegistrator(const char* name);
	int operator()() const { return m_registeredID; }

	const char* name() const;

private:
	int m_registeredID;
};

class SDRANGELOVE_API Message {
public:
	virtual ~Message();

	void submit(MessageQueue* queue, void* destination = NULL);
	int execute(MessageQueue* queue, void* destination = NULL);

	void completed(int result = 0);

	int id() const { return m_id; }
	const char* name() const;
	void* destination() const { return m_destination; }

protected:
	Message(int id);

	// adressing
	int m_id;
	void* m_destination;

	// stuff for synchronous messages
	bool m_synchronous;
	QWaitCondition* m_waitCondition;
	QMutex* m_mutex;
	QAtomicInt m_complete;
	int m_result;
};

#endif // INCLUDE_MESSAGE_H
