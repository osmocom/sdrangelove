#include <QPainter>
#include "scope.h"

Scope::Scope(QWidget* parent) :
	QWidget(parent),
	m_changed(false)
{
	setAttribute(Qt::WA_OpaquePaintEvent);
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
	m_timer.start(50);
}

Scope::~Scope()
{
}

void Scope::newTrace(const std::vector<Real>& trace)
{
	if(!m_mutex.tryLock(2))
		return;

	m_trace = trace;
	m_changed = true;

	m_mutex.unlock();
}

void Scope::paintEvent(QPaintEvent* event)
{
	if(!m_mutex.tryLock(2))
		return;

	QPainter painter(this);
	int margin = 10;
	int gridHeight = height() - 2 * margin;
	int gridWidth = width() - 2 * margin;

	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::black);
	painter.drawRect(rect());

	painter.setPen(QColor(0x19, 0x19, 0x19));
	for(int i = 1; i < 10; i++) {
		painter.drawLine(QLineF(
			margin, margin + (gridHeight * i) / 10.0,
			margin + gridWidth, margin + (gridHeight * i) / 10.0
		));
	}
	for(int i = 1; i < 10; i++) {
		painter.drawLine(QLineF(
			margin + (gridWidth * i) / 10.0, margin,
			margin + (gridWidth * i) / 10.0, margin + gridHeight
		));
	}
	painter.setPen(QColor(0x8c, 0x8c, 0x8c));
	painter.setBrush(Qt::NoBrush);
	painter.drawRect(QRectF(margin, margin, gridWidth, gridHeight));

	m_changed = false;

	m_mutex.unlock();
}

void Scope::tick()
{
	if(m_changed)
		repaint();
}
