#ifndef INCLUDE_CHANNELMARKER_H
#define INCLUDE_CHANNELMARKER_H

#include <QObject>
#include <QColor>

class ChannelMarker : public QObject {
	Q_OBJECT

public:
	ChannelMarker(QObject* parent = NULL);

	void setTitle(const QString& title);
	const QString& getTitle() const { return m_title; }

	void setCenterFrequency(int centerFrequency);
	int getCenterFrequency() const { return m_centerFrequency; }

	void setBandwidth(int bandwidth);
	int getBandwidth() const { return m_bandwidth; }

	void setVisible(bool visible);
	bool getVisible() const { return m_visible; }

	void setColor(const QColor& color);
	const QColor& getColor() const { return m_color; }

protected:
	QString m_title;
	int m_centerFrequency;
	int m_bandwidth;
	bool m_visible;
	QColor m_color;

signals:
	void changed();
};

#endif // INCLUDE_CHANNELMARKER_H