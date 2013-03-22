#include "dsp/channelmarker.h"

ChannelMarker::ChannelMarker(QObject* parent) :
	QObject(parent),
	m_centerFrequency(0),
	m_bandwidth(0),
	m_visible(false),
	m_color(Qt::red)
{
}

void ChannelMarker::setTitle(const QString& title)
{
	m_title = title;
	emit changed();
}

void ChannelMarker::setCenterFrequency(int centerFrequency)
{
	m_centerFrequency = centerFrequency;
	emit changed();
}

void ChannelMarker::setBandwidth(int bandwidth)
{
	m_bandwidth = bandwidth;
	emit changed();
}

void ChannelMarker::setVisible(bool visible)
{
	m_visible = visible;
	emit changed();
}

void ChannelMarker::setColor(const QColor& color)
{
	m_color = color;
	emit changed();
}
