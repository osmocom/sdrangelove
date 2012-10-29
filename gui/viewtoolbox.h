///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 maintech GmbH, Otto-Hahn-Str. 15, 97204 Hoechberg, Germany //
// written by Christian Daniel                                                   //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDE_VIEWTOOLBOX_H
#define INCLUDE_VIEWTOOLBOX_H

#include <QWidget>

namespace Ui {
	class ViewToolBox;
}

class ViewToolBox : public QWidget {
	Q_OBJECT

public:
	explicit ViewToolBox(QWidget* parent = NULL);
	~ViewToolBox();

	void setViewWaterfall(bool checked);
	void setWaterfallUpward(bool checked);
	void setViewHistogram(bool checked);
	void setViewLiveSpectrum(bool checked);

signals:
	void closed();
	void viewWaterfall(bool checked);
	void waterfallUpward(bool checked);
	void viewHistogram(bool checked);
	void viewLiveSpectrum(bool checked);

private slots:
	void on_viewWaterfall_toggled(bool checked);
	void on_waterfallUpward_currentIndexChanged(int index);
	void on_viewHistogram_toggled(bool checked);
	void on_viewLiveSpectrum_toggled(bool checked);

private:
	Ui::ViewToolBox* ui;

	void closeEvent(QCloseEvent*);
};

#endif // INCLUDE_VIEWTOOLBOX_H
