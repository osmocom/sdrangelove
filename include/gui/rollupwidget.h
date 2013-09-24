#ifndef INCLUDE_ROLLUPWIDGET_H
#define INCLUDE_ROLLUPWIDGET_H

#include <QWidget>
#include "util/export.h"

class SDRANGELOVE_API RollupWidget : public QWidget {
	Q_OBJECT

public:
	RollupWidget(QWidget* parent = NULL);

protected:
	int arrangeRollups();

	void paintEvent(QPaintEvent*);
	int paintRollup(QWidget* rollup, int pos, QPainter* p, bool last, const QColor& frame);

	void resizeEvent(QResizeEvent* size);
	void mousePressEvent(QMouseEvent* event);

	bool eventFilter(QObject* object, QEvent* event);
};

#endif // INCLUDE_ROLLUPWIDGET_H
