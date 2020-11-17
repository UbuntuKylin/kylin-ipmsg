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

#include "duktowindow.h"
#include "guibehind.h"
#include "platform.h"
#include "settings.h"

DuktoWindow::DuktoWindow(QWindow *parent) :
    QmlApplicationViewer(parent), mGuiBehind(NULL){

    this->setTitle(tr("Kylin Ipmsg"));
    this->setIcon(QIcon("/usr/share/pixmaps/kylin-ipmsg.png"));

    // 响应用户手册
    mDaemonIpcDbus = new DaemonIpcDbus();

//    this->setWindowTitle(tr("Kylin Ipmsg"));
//    setWindowTitle("麒麟传书");
//    setWindowIcon(QIcon("/usr/share/pixmaps/kylin-ipmsg.png"));
    QString iconPath = "/usr/share/icons/ukui-icon-theme-default/64x64/apps/kylin-ipmsg.png";
    QFileInfo fi(iconPath);
    if (!fi.exists()) {
        iconPath = "/usr/share/icons/kylin-icon-theme/64x64/apps/kylin-ipmsg.png";
        fi.setFile(iconPath);
        if (!fi.exists()) {
            iconPath = "/usr/share/pixmaps/kylin-ipmsg.png";
        }
    }

    this->setIcon(QIcon(iconPath));

    setOrientation(QmlApplicationViewer::ScreenOrientationLockPortrait);

    setMaximumWidth(420);
    setMinimumWidth(330);
}

/*
* Summary: set gui reference
* Parameters:
*   ref: gui behind object
* Return :
*/
void DuktoWindow::setGuiBehindReference(GuiBehind* ref){
    mGuiBehind = ref;
}

/*
* Summary:  close event
* Parameters:
*   qt close event
* Return :
*/
void DuktoWindow::closeEvent(QCloseEvent *){
//    mGuiBehind->settings()->saveWindowGeometry(saveGeometry());
    mGuiBehind->settings()->saveWindowGeometry(frameGeometry());
    mGuiBehind->close();

    exit(0);
}

void DuktoWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {

    case Qt::Key_F1:
//        qDebug() << "DuktoWindow keyPressEvent Key_F1" << mDaemonIpcDbus->daemonIsNotRunning();

        if (!mDaemonIpcDbus->daemonIsNotRunning()){
            //增加标题栏帮助菜单、F1快捷键打开用户手册
            mDaemonIpcDbus->showGuide("kylin-ipmsg");
        }
        break;

    default:
        break;
    }
}
