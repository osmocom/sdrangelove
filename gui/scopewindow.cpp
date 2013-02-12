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

#include "scopewindow.h"
#include "ui_scopewindow.h"

ScopeWindow::ScopeWindow(DSPEngine* dspEngine, QWidget* parent) :
	QWidget(parent),
	ui(new Ui::ScopeWindow),
	m_sampleRate(0),
	m_timeStep(1)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	ui->scope->setDSPEngine(dspEngine);
}

ScopeWindow::~ScopeWindow()
{
	delete ui;
}

void ScopeWindow::setSampleRate(int sampleRate)
{
	m_sampleRate = sampleRate;
	on_scope_traceSizeChanged(0);
}

void ScopeWindow::on_amp_valueChanged(int value)
{
	static qreal amps[11] = { 0.2, 0.1, 0.05, 0.02, 0.01, 0.005, 0.002, 0.001, 0.0005, 0.0002, 0.0001 };
	ui->ampText->setText(tr("%1\n/div").arg(amps[value], 0, 'f', 4));
	ui->scope->setAmp(0.2 / amps[value]);
}

void ScopeWindow::on_scope_traceSizeChanged(int)
{
	qreal t = (ui->scope->getTraceSize() * 0.1 / m_sampleRate) / (qreal)m_timeStep;
	if(t < 0.000001)
		ui->timeText->setText(tr("%1\nns/div").arg(t * 1000000000.0));
	else if(t < 0.001)
		ui->timeText->setText(tr("%1\nµs/div").arg(t * 1000000.0));
	else if(t < 1.0)
		ui->timeText->setText(tr("%1\nms/div").arg(t * 1000.0));
	else ui->timeText->setText(tr("%1\ns/div").arg(t * 1.0));
}

void ScopeWindow::on_time_valueChanged(int value)
{
	m_timeStep = value;
	on_scope_traceSizeChanged(0);
	ui->scope->setTimeStep(m_timeStep);
}

void ScopeWindow::on_timeOfs_valueChanged(int value)
{
	ui->scope->setTimeOfsProMill(value);
}

void ScopeWindow::on_displayMode_currentIndexChanged(int index)
{
	switch(index) {
		case 0: // i+q
			ui->scope->setMode(GLScope::ModeIQ);
			break;
		case 1: // mag+pha
			ui->scope->setMode(GLScope::ModeMagPha);
			break;
		case 2: // derived1+derived2
			ui->scope->setMode(GLScope::ModeDerived12);
			break;
		default:
			break;
	}
}
