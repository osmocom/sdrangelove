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

#include <QApplication>
#include <QTextCodec>
#include <QProxyStyle>
#include <QStyleFactory>
#include "mainwindow.h"

static int runQtApplication(int argc, char* argv[])
{
	QApplication a(argc, argv);

	/*QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));*/

	QCoreApplication::setOrganizationName("osmocom");
	QCoreApplication::setApplicationName("SDRangelove");

	//QApplication::setStyle(new QProxyStyle());
#if 0
	qApp->setStyle(QStyleFactory::create("fusion"));

	QPalette palette;
	palette.setColor(QPalette::Window, QColor(53,53,53));
	palette.setColor(QPalette::WindowText, Qt::white);
	palette.setColor(QPalette::Base, QColor(15,15,15));
	palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
	palette.setColor(QPalette::ToolTipBase, Qt::white);
	palette.setColor(QPalette::ToolTipText, Qt::white);
	palette.setColor(QPalette::Text, Qt::white);
	palette.setColor(QPalette::Button, QColor(53,53,53));
	palette.setColor(QPalette::ButtonText, Qt::white);
	palette.setColor(QPalette::BrightText, Qt::red);

	palette.setColor(QPalette::Highlight, QColor(142,45,197).lighter());
	palette.setColor(QPalette::HighlightedText, Qt::black);
	qApp->setPalette(palette);
#endif
	MainWindow w;
	w.show();

	return a.exec();
}

int main(int argc, char* argv[])
{
	return runQtApplication(argc, argv);
}
