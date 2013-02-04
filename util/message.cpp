#include <QWaitCondition>
#include <QMutex>
#include "message.h"
#include "messagequeue.h"

Message::Message() :
	m_synchronous(false),
	m_waitCondition(NULL),
	m_mutex(NULL),
	m_complete(0)
{
}

Message::~Message()
{
	if(m_waitCondition != NULL)
		delete m_waitCondition;
	if(m_mutex != NULL)
		delete m_mutex;
}

void Message::submit(MessageQueue* queue)
{
	m_synchronous = false;
	queue->submit(this);
}

int Message::execute(MessageQueue* queue)
{
	m_synchronous = true;

	if(m_waitCondition == NULL)
		m_waitCondition = new QWaitCondition;
	if(m_mutex == NULL)
		m_mutex = new QMutex;

	m_mutex->lock();
	m_complete.testAndSetAcquire(0, 1);
	queue->submit(this);
	while(!m_complete.testAndSetAcquire(0, 1))
		m_waitCondition->wait(m_mutex, 100);
	m_complete = 0;
	int result = m_result;
	m_mutex->unlock();
	return result;
}

void Message::completed(int result)
{
	if(m_synchronous) {
		m_result = result;
		m_complete = 0;
		m_waitCondition->wakeAll();
	} else {
		delete this;
	}
}
