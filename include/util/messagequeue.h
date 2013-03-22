#ifndef INCLUDE_MESSAGEQUEUE_H
#define INCLUDE_MESSAGEQUEUE_H

#include <QObject>
#include <QQueue>
#include "spinlock.h"

class Message;

class MessageQueue : public QObject {
	Q_OBJECT

public:
	MessageQueue(QObject* parent = NULL);
	~MessageQueue();

	void submit(Message* message);
	Message* accept();

	int countPending();

signals:
	void messageEnqueued();

private:
	Spinlock m_lock;
	QQueue<Message*> m_queue;
};

#endif // INCLUDE_MESSAGEQUEUE_H
