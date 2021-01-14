/* DUKTO - A simple, fast and multi-platform file transfer tool for LAN users
 * Copyright (C) 2011 Emanuele Colombo
 *               2020 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <qglobal.h>
#include <QtWidgets/QApplication>
#include <QTranslator>
#include <QLocale>
#include <QDebug>

#include "qtsingleapplication.h"
#include "qmlapplicationviewer.h"
#include "guibehind.h"
#include "duktowindow.h"


int main(int argc, char *argv[])
{
//    QApplication::setGraphicsSystem("raster");

    // 适配4K屏
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    #endif

    // 需要给QtSingleApplication的传入参数id加DISPLAY标识
    QString id = QString("kylin-ipmsg"+ QLatin1String(getenv("DISPLAY")));
    QtSingleApplication app(id, argc, argv);
//    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
//    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
//    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    // 国际化
    QString locale = QLocale::system().name();
    QTranslator trans_global, trans_menu;
    if(locale == "zh_CN"){
        trans_global.load(":/qml/kylin-ipmsg_zh_CN.qm");
//        trans_global.load(":/qml/kylin-ipmsg_bo_CN.qm");
        trans_menu.load(":/qml/qt_zh_CN.qm");
        app.installTranslator(&trans_global);
        app.installTranslator(&trans_menu);
    }

    // 单例判断
    if (app.isRunning()) {
        app.sendMessage("FOREGROUND");
        return 0;
    }

//    app.setWindowIcon(QIcon("/usr/share/pixmaps/kylin-ipmsg.png"));
    app.setWindowIcon(QIcon::fromTheme("kylin-ipmsg"));

    DuktoWindow viewer;
//    app.setActivationWindow(&viewer, true);

    GuiBehind gb(&viewer);

    viewer.showExpanded();
    viewer.raise();
    app.installEventFilter(&gb);

    return app.exec();
}
