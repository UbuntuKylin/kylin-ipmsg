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

/*
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
*/

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

    /*lock file*/
    QtSingleApplication app(id, argc, argv);
    app.setApplicationVersion("1.1.21");

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

#ifndef QT_NO_TRANSLATION
    QString translatorFileName = QLatin1String("qt_");
    translatorFileName += QLocale::system().name();
    QTranslator *translator = new QTranslator();
    if (translator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(translator);
    else
        qDebug() << "Failed to load Chinese translation file.";
#endif

#if 0
{
    /*add file lock*/
    struct flock s_lock;
    s_lock.l_type = F_WRLCK;
    s_lock.l_whence = SEEK_SET;
    s_lock.l_start = 0;
    s_lock.l_len = 0;
    s_lock.l_pid = getpid();

    char p_lock_file_dir[512];
    char p_lock_file_path[1024];
    char *p_home = NULL;
    int i_ret = -1;
    int fd = -1;

    memset(p_lock_file_dir , 0x00 , sizeof(p_lock_file_dir));
    memset(p_lock_file_path , 0x00 , sizeof(p_lock_file_path));

    p_home = getenv("HOME");
    if(p_home == NULL) {
        printf("get env var HOME fail\n");
        return -1;
    }

    sprintf(p_lock_file_dir , "%s/.kylin-ipmsg" , p_home);
    sprintf(p_lock_file_path , "%s/kylin-ipmsg.lock" , p_lock_file_dir);

    i_ret = access(p_lock_file_dir , F_OK);
    if (i_ret == -1) {
        printf("lock file path is not exits\n");
        i_ret = mkdir(p_lock_file_dir , 0777);
        if(i_ret == -1) {
            printf("create dir fail\n");
            return -1;
        }
        printf("create dir success\n");
    }

    fd = open(p_lock_file_path , O_CREAT |  O_RDWR | O_TRUNC , 0777);
    if(fd == -1) {
        printf("get file fd fail\n");
        return -1;
    }
    i_ret = fcntl(fd , F_SETLK , &s_lock);
    if (i_ret == -1) {
        printf("lock file fail . errno is %s\n" , strerror(errno));
        return -1;
    }
}
#endif

    // 单例判断
    /*check file whether locked*/
    if (app.isRunning()) {
        app.sendMessage("FOREGROUND");
        return 0;
    }
//    app.setWindowIcon(QIcon("/usr/share/pixmaps/kylin-ipmsg.png"));
    app.setWindowIcon(QIcon::fromTheme("kylin-ipmsg"));

    /*inherit qmlapplicationviewer and link dbus open kylin guide manual*/
    DuktoWindow viewer;

//    app.setActivationWindow(&viewer, true);

    /*init and udp , tcp listing and online broadcast*/
    /*udp broadcast is one byte and systemflag*/
    /*udp read is create item*/
    /*tcp active link is example ksocket in a thread*/
    /*tcp passive link is example ksocket in a thread*/
    /*ksocket class maintain two socket*/
    /*one ksocket example is a chatwidget*/
    /*tcp passive link is create second tcp server*/
    /*real socket in ksocket*/
    GuiBehind gb(&viewer);

    viewer.showExpanded();
    viewer.raise();
    app.installEventFilter(&gb);

    return app.exec();
}
