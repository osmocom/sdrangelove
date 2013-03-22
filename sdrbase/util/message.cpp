#include <vector>
#include <QWaitCondition>
#include <QMutex>
#include "util/message.h"
#include "util/messagequeue.h"
#include "util/spinlock.h"

class Registry {
public:
	static Registry* getInstance()
	{
		SpinlockHolder spinlockHolder(&m_registryLock);
		if(m_instance != NULL) {
			return m_instance;
		} else {
			m_instance = new Registry;
			return m_instance;
		}
	}

	int registerMessage(const char* name)
	{
		SpinlockHolder spinlockHolder(&m_registryLock);
		m_registry.push_back(name);
		return m_registry.size() - 1;
	}

	const char* name(int id) const
	{
		return m_registry[id];
	}

private:
	static Registry* m_instance;
	static Spinlock m_registryLock;
	std::vector<const char*> m_registry;

	Registry() :
		m_registry()
	{ }
};

Registry* Registry::m_instance = NULL;
Spinlock Registry::m_registryLock;

MessageRegistrator::MessageRegistrator(const char* name)
{
	Registry* registry = Registry::getInstance();
	m_registeredID = registry->registerMessage(name);
	qDebug("%s registered as %d", name, m_registeredID);
}

Message::~Message()
{
	if(m_waitCondition != NULL)
		delete m_waitCondition;
	if(m_mutex != NULL)
		delete m_mutex;
}

void Message::submit(MessageQueue* queue, void* destination)
{
	m_destination = destination;
	m_synchronous = false;
	queue->submit(this);
}

int Message::execute(MessageQueue* queue, void* destination)
{
	m_destination = destination;
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

const char* Message::name() const
{
	return Registry::getInstance()->name(m_id);
}

Message::Message(int id) :
	m_id(id),
	m_destination(NULL),
	m_synchronous(false),
	m_waitCondition(NULL),
	m_mutex(NULL),
	m_complete(0)
{
}
