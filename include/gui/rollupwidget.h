#ifndef INCLUDE_DEMODWIDGET_H
#define INCLUDE_DEMODWIDGET_H

#include <QWidget>

class RollupWidget : public QWidget {
	Q_OBJECT

public:
	RollupWidget(QWidget* parent = NULL);

	void addRollup(QWidget* rollup);

protected:
	//QWidgetList m_rollups;

	int arrangeRollups();

	void paintEvent(QPaintEvent*);
	int paintRollup(QWidget* rollup, int pos, QPainter* p, bool last, const QColor& frame);

	void resizeEvent(QResizeEvent* size);
	void mousePressEvent(QMouseEvent* event);

	bool eventFilter(QObject* object, QEvent* event);
};

#endif // INCLUDE_DEMODWIDGET_H
