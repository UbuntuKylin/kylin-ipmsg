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

    this->setTitle(tr("Messages"));
//    this->setIcon(QIcon("/usr/share/pixmaps/kylin-ipmsg.png"));
    // this->setIcon(QIcon::fromTheme("kylin-ipmsg"));

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

//    this->setIcon(QIcon(iconPath));
    this->setIcon(QIcon::fromTheme("kylin-ipmsg"));

    setOrientation(QmlApplicationViewer::ScreenOrientationLockPortrait);

    // 固定窗口大小
    int fixedWidth  = 335;
    int fixedHeight = 605;
    setMaximumWidth(fixedWidth);
    setMinimumWidth(fixedWidth);
    setMaximumHeight(fixedHeight);
    setMinimumHeight(fixedHeight);

    // 固定初始窗口位置到屏幕中央
    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    this->setGeometry((availableGeometry.width() - fixedWidth)/2, 
                      (availableGeometry.height() - fixedHeight)/2,
                      fixedWidth, fixedHeight);
    
    if (QGSettings::isSchemaInstalled(THEME_QT_SCHEMA)) {

        themeSettings = new QGSettings(THEME_QT_SCHEMA);

        connect(themeSettings, &QGSettings::changed, this, [=](const QString &key) {

            qDebug() << "key: " << key;

            if (key == ICON_QT_KEY) {
                //获取当前主题
                QString currentIconTheme = themeSettings->get(key).toString();
                qDebug() << "currentIconTheme: " << currentIconTheme;

                this->setIcon(QIcon::fromTheme("kylin-ipmsg"));
                
                // icon图标路径
                QString iconPath = "/usr/share/icons/"+ currentIconTheme + "/64x64/apps/kylin-ipmsg.png";
                QFileInfo fi(iconPath);
                if (fi.exists()) {
                    mGuiBehind->setIconPath(iconPath);
                }
            }
        });
    }
}

// 判断是否有其他用户正在运行传书
void DuktoWindow::judgeSingleUser()
{
    // 判断是否有其他用户正在运行传书
    QProcess process;
    // process.start("sh", QStringList() << "-c" << "ps -ef |grep -w kylin-ipmsg |grep -v grep | wc -l");
    process.start("sh", QStringList() << "-c" << "netstat -pan|grep -w 9695 |wc -l");
    process.waitForFinished();
    QString res = process.readAllStandardOutput();
    res = res.remove("\n");
    // qDebug() << "res" << res;
    if (res != "0") {
        msgBox = new QMessageBox(QMessageBox::Warning,this->title(),tr("Messages has been opened by other user!"),QMessageBox::Yes);
        msgBox->button(QMessageBox::Yes)->setText(tr("Confirm"));
        qApp->setApplicationDisplayName(this->title());
        msgBox->exec();

        exit(0);
    }
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
void DuktoWindow::closeEvent(QCloseEvent *event){
//    mGuiBehind->settings()->saveWindowGeometry(saveGeometry());
    mGuiBehind->settings()->saveWindowGeometry(frameGeometry());
    mGuiBehind->close();

    qDebug() << "exit(0)exit(0)exit(0)exit(0)exit(0)";
    event->accept();
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
