#include "messagequeue.h"
#include "message.h"

MessageQueue::MessageQueue(QObject* parent) :
	QObject(parent),
	m_lock(),
	m_queue()
{
}

MessageQueue::~MessageQueue()
{
	SpinlockHolder spinlockHolder(&m_lock);

	while(!m_queue.isEmpty())
		delete m_queue.takeFirst();
}

void MessageQueue::submit(Message* message)
{
	m_lock.lock();
	m_queue.append(message);
	m_lock.unlock();
	emit messageEnqueued();
}

Message* MessageQueue::accept()
{
	SpinlockHolder spinlockHolder(&m_lock);

	if(m_queue.isEmpty())
		return NULL;
	else return m_queue.takeFirst();
}

int MessageQueue::countPending()
{
	SpinlockHolder spinlockHolder(&m_lock);

	return m_queue.size();
}
