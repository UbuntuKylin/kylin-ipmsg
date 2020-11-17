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

#ifndef DAEMONIPCDBUS_H
#define DAEMONIPCDBUS_H

#define KYLIN_USER_GUIDE_PATH "/"

#define KYLIN_USER_GUIDE_SERVICE "com.kylinUserGuide.hotel"

#define KYLIN_USER_GUIDE_INTERFACE "com.guide.hotel"

#define SERVICE_NAME_SIZE 30

#include <QObject>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <unistd.h>
#include <sys/types.h>

class DaemonIpcDbus : public QObject
{
    Q_OBJECT
    // 定义Interface名称为com.scorpio.test.value
//    Q_CLASSINFO("D-Bus Interface", "com.scorpio.test.value")
public:

    DaemonIpcDbus() {}

public slots:

    int daemonIsNotRunning();
    void showGuide(QString appName);

};

/*
// 使用方法
DaemonIpcDbus *mDaemonIpcDbus;
mDaemonIpcDbus = new DaemonIpcDbus();
if (!mDaemonIpcDbus->daemonIsNotRunning()){
    //增加标题栏帮助菜单、F1快捷键打开用户手册
    mDaemonIpcDbus->showGuide("kylin-ipmsg");
}
*/

#endif // DAEMONIPCDBUS_H
