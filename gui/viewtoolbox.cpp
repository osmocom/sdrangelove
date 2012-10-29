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

#include <QCloseEvent>
#include "viewtoolbox.h"
#include "ui_viewtoolbox.h"

ViewToolBox::ViewToolBox(QWidget* parent) :
	QWidget(parent, Qt::Tool),
	ui(new Ui::ViewToolBox)
{
	ui->setupUi(this);

	// align the QComboBox exactly below the QCheckBox
	QStyleOption o;
	ui->wfDirectionSpacer->changeSize(
		style()->subElementRect(QStyle::SE_CheckBoxIndicator, &o, this).width() - style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing, &o, this),
		0,
		QSizePolicy::Fixed,
		QSizePolicy::Fixed);

	setFixedSize(sizeHint());
}

ViewToolBox::~ViewToolBox()
{
	delete ui;
}

void ViewToolBox::setViewWaterfall(bool checked)
{
	ui->viewWaterfall->setChecked(checked);
	if(checked)
		ui->waterfallUpward->setEnabled(true);
	else ui->waterfallUpward->setEnabled(false);
}

void ViewToolBox::setWaterfallUpward(bool checked)
{
	if(checked)
		ui->waterfallUpward->setCurrentIndex(0);
	else ui->waterfallUpward->setCurrentIndex(1);
}

void ViewToolBox::setViewHistogram(bool checked)
{
	ui->viewHistogram->setChecked(checked);
}

void ViewToolBox::setViewLiveSpectrum(bool checked)
{
	ui->viewLiveSpectrum->setChecked(checked);
}

void ViewToolBox::closeEvent(QCloseEvent* event)
{
	emit closed();
	event->accept();
}

void ViewToolBox::on_viewWaterfall_toggled(bool checked)
{
	if(checked)
		ui->waterfallUpward->setEnabled(true);
	else ui->waterfallUpward->setEnabled(false);

	emit viewWaterfall(checked);
}

void ViewToolBox::on_waterfallUpward_currentIndexChanged(int index)
{
	if(index == 0)
		emit waterfallUpward(true);
	else emit waterfallUpward(false);
}

void ViewToolBox::on_viewHistogram_toggled(bool checked)
{
	emit viewHistogram(checked);
}

void ViewToolBox::on_viewLiveSpectrum_toggled(bool checked)
{
	emit viewLiveSpectrum(checked);
}
