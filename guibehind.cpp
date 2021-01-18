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

#include "guibehind.h"
#include "settings.h"
#include "miniwebserver.h"
#include "duktowindow.h"
#include "platform.h"

#include <QQuickView>
#include <QQmlContext>
#include <QQmlProperty>
#include <QTimer>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QGraphicsObject>
#include <QRegExp>
#include <QThread>
#include <QTemporaryFile>
#include <QDesktopWidget>
#include <QScreen>
#include <QTime>

GuiBehind::GuiBehind(DuktoWindow* view) :
    QObject(NULL), mView(view), mPeriodicHelloTimer(NULL),
    mMiniWebServer(NULL), mSettings(NULL)
{
    // 通信管理中心
    mDuktoProtocol = new DuktoProtocol(this);
    // 头像web服务器
    mMiniWebServer = new MiniWebServer();
    // 设置中心
    mSettings = new Settings(this);
    // 添加 "本机" 到好友列表strip
    mBuddiesList.addMeElement(tr(" (You)"));
    // 添加 "使用IP地址" 到好友列表
    mBuddiesList.addIpElement(tr("Use IP Address"), tr("Contact with parent network"));
    // 响应用户手册
    mDaemonIpcDbus = new DaemonIpcDbus();

    mView->setGuiBehindReference(this);
    QDir::setCurrent(mSettings->currentPath());
    mTheme.setThemeColor(mSettings->themeColor());

    // qml属性填充
    view->rootContext()->setContextProperty("buddiesListData", &mBuddiesList);
    view->rootContext()->setContextProperty("recentListData", &mRecentList);
    view->rootContext()->setContextProperty("ipAddressesData", &mIpAddresses);
    view->rootContext()->setContextProperty("guiBehind", this);
    view->rootContext()->setContextProperty("theme", &mTheme);

    connect(mDuktoProtocol, SIGNAL(peerListAdded(Peer)), this, SLOT(peerListAdded(Peer)));
    connect(mDuktoProtocol, SIGNAL(peerListRemoved(Peer)), this, SLOT(peerListRemoved(Peer)));

    // 上线发广播
    /*init udp server and tcp server*/
    mDuktoProtocol->initialize();
    mDuktoProtocol->sayHello(QHostAddress::Broadcast);

    // 定时发广播
    mPeriodicHelloTimer = new QTimer(this);
    connect(mPeriodicHelloTimer, SIGNAL(timeout()), this, SLOT(periodicHello()));
    mPeriodicHelloTimer->start(60000);

    // 新消息闪烁提示相关
    this->alertTimer = new QTimer(this);
    this->alertTimer->setInterval(300);
    this->connect(this->alertTimer, SIGNAL(timeout()), this, SLOT(msgAlert()));
//    this->iconDukto = QIcon("/usr/share/pixmaps/kylin-ipmsg.png");
//    mSettings->saveIconPath("");
    QString iconPath = mSettings->iconPath();
    QFileInfo fi(iconPath);
    if (iconPath != "" && fi.exists()) {
//        this->iconDukto = QIcon(iconPath);
        this->iconDukto = QIcon::fromTheme("kylin-ipmsg");
    }
    else {
        // SP1的icon图标路径
        iconPath = "/usr/share/icons/ukui-icon-theme-default/64x64/apps/kylin-ipmsg.png";
        fi.setFile(iconPath);
        if (fi.exists()) {
            mSettings->saveIconPath(iconPath);
//            this->iconDukto = QIcon(iconPath);
            this->iconDukto = QIcon::fromTheme("kylin-ipmsg");
        }
        else {
            // V10的icon图标路径
            iconPath = "/usr/share/icons/kylin-icon-theme/64x64/apps/kylin-ipmsg.png";
            fi.setFile(iconPath);
            if (fi.exists()) {
                mSettings->saveIconPath(iconPath);
//                this->iconDukto = QIcon(iconPath);
                this->iconDukto = QIcon::fromTheme("kylin-ipmsg");
            }
            else {
                // 保险措施
                iconPath = "/usr/share/pixmaps/kylin-ipmsg.png";
                mSettings->saveIconPath(iconPath);
//                this->iconDukto = QIcon(iconPath);
                this->iconDukto = QIcon::fromTheme("kylin-ipmsg");
            }
        }
    }
    // qml访问本地图片需使用绝对路径
    this->setIconPath(iconPath);

    // 加载 GUI
    view->setSource(QUrl("qrc:/qml/dukto/Dukto.qml"));
    //TODO: check QtQuick 2 restoreGeometry
//    view->restoreGeometry(mSettings->windowGeometry());
//    view->setGeometry(mSettings->windowGeometry());

    this->iconBlank = QIcon(":/qml/dukto/Blank.png");

    this->cwx = -1;
    this->cwy = -1;
    this->timeFlag = 0;
}

GuiBehind::~GuiBehind(){
    mDuktoProtocol->sayGoodbye();

    if (mMiniWebServer) mMiniWebServer->deleteLater();
    if (mPeriodicHelloTimer) mPeriodicHelloTimer->deleteLater();
    if (mView) mView->deleteLater();
    if (mSettings) mSettings->deleteLater();
}

// 在列表中添加新好友
void GuiBehind::peerListAdded(Peer peer){
    mBuddiesList.addBuddy(peer);
}

// 收到goodbye消息，在列表中删除对应好友
void GuiBehind::peerListRemoved(Peer peer){
    mBuddiesList.removeBuddy(peer.mac);
}

// 一次文件接收完成
/*
* Parameters:
*   files: receive files
*   totalsize: total size
*   dir: dir
*   mac: mac address
* Return :
*/
void GuiBehind::receiveFileComplete(QStringList *files, qint64 totalSize, QString dir, QString mac){
    QDir d(".");
    QStandardItem *buddy = mBuddiesList.buddyByMac(mac);
    QString userName = buddy->data(BuddyListItemModel::Username).toString();

    // 时间头
    QDateTime time = QDateTime::currentDateTime();
    QString tHead = "<font color=" + Theme::DEFAULT_THEME_COLOR + ">";
    tHead += userName;
    tHead += " (";
    tHead += time.toString("hh:mm:ss");
    tHead += ")</font>";

    QString msg = "";
    QStringList tmpList;
    // 收到的是文件
    if (dir == ""){
//        msg = "<font color=" + mTheme.color4() + ">收到文件 ";
        msg = "<font color=" + mTheme.color4() + ">"+tr("Received file" );
        if(files->size() == 1){
            msg += "'";
            msg += files->at(0);
//            msg += "' 大小 (";
            msg += "' "+tr("size")+" (";
            msg += this->sizeHuman(totalSize);
            msg += ")</font><br/>";
//            msg += "<a href='" + d.absoluteFilePath(files->at(0)) + "'>打开</a>";
            msg += "<a href='" + d.absoluteFilePath(files->at(0)) + "'>"+tr("Open")+"</a>";
//            msg += "&nbsp;&nbsp;<a href='" + d.absolutePath() + "'>打开目录</a>";
            msg += "&nbsp;&nbsp;<a href='" + d.absolutePath() + "'>"+tr("Open directory")+"</a>";
        }else{
            msg += QString::number(files->size());
//            msg += " 个";
            msg += " "+tr("files");
//            msg += " 共计大小 (";
            msg += " "+tr("Total size")+" (";
            msg += this->sizeHuman(totalSize);
            msg += ")<br/>";
            for(int i = 0; i < files->size(); i ++){
                msg += "'";
                msg += files->at(i);
                msg += "' ";
            }
            msg += "</font><br/><a href='" + d.absolutePath() + "'>"+tr("Open directory")+"</a>";
        }

    // 收到的是文件夹
    }else{
//        msg = "<font color=" + mTheme.color4() + ">收到文件夹 '" + dir  + "'  共计大小 (";
        msg = "<font color=" + mTheme.color4() + ">"+tr("Received folder")+" '" + dir  + "'  "+tr("Total size")+" (";
        msg += this->sizeHuman(totalSize);
        msg += ")</font><br/>";
        msg += "<a href='" + d.absolutePath() + "'>"+tr("Open directory")+"</a>";
    }

    // 已有该对话框，直接append
    if(this->cws.contains(mac)){
        ChatWidget *cw = this->cws.value(mac);
        cw->addLogDirect(tHead);
        cw->addLogDirect(msg);
        cw->stopTransfer(false);

    // 没有该对话框，先保存在tmpMsgs中
    }else{
        //  已有该tmpMsg，取出继续使用
        if(this->tmpMsgs.contains(mac)){
            tmpList = this->tmpMsgs.value(mac);
        }
        tmpList.append(tHead);
        tmpList.append(msg);
        this->tmpMsgs.insert(mac, tmpList);
    }

    // 将同名的历史聊天对象删除，再执行下面的addRecent，确保每一个对象只在列表中出现一个
    this->removeRecentItem(mac);

    // 添加到最近聊天列表
    QString ip_mac = buddy->data(BuddyListItemModel::Ip).toString();
    if (files->size() == 1)
        mRecentList.addRecent(files->at(0), d.absoluteFilePath(files->at(0)), "file", userName, totalSize, ip_mac);
    else
        mRecentList.addRecent("Files and folders", d.absolutePath(), "misc", userName, totalSize, ip_mac);

    delete files;
    files = NULL;

    // 界面新消息提醒
    newMsgAlert(mac);
}

// 一次文字接收完成
/*
* Parameters:
*   text: text msg
*   mac: mac address
* Return :
*/
void GuiBehind::receiveTextComplete(QString text, QString mac){
    QStandardItem *buddy = mBuddiesList.buddyByMac(mac);
    QString userName = buddy->data(BuddyListItemModel::Username).toString();

    // 已有该对话框，直接append
    if(this->cws.contains(mac)){
        ChatWidget *cw = this->cws.value(mac);
        cw->addTextLog(text);

    // 没有该对话框，先保存在tmpMsgs中
    }else{
        QDateTime time = QDateTime::currentDateTime();
        QString tHead= "<font color=" + Theme::DEFAULT_THEME_COLOR + ">";
        tHead += userName;
        tHead += " (";
        tHead += time.toString("hh:mm:ss");
        tHead += ")</font>";

        if(this->tmpMsgs.contains(mac)){
            QStringList tmpList = this->tmpMsgs.value(mac);
            tmpList.append(tHead);
            tmpList.append(text);
            this->tmpMsgs.insert(mac, tmpList);
        }else{
            QStringList tmpList;
            tmpList.append(tHead);
            tmpList.append(text);
            this->tmpMsgs.insert(mac, tmpList);
        }
    }

    //  将同名的历史聊天对象删除，再执行下面的addRecent，确保每一个对象只在列表中出现一个
    this->removeRecentItem(mac);

    // 添加到最近聊天列表
    QString ip_mac = buddy->data(BuddyListItemModel::Ip).toString();
    mRecentList.addRecent("Text snippet", text, "text", userName, 0, ip_mac);

    // 界面新消息提醒
    newMsgAlert(mac);
}

// 删除最近聊天列表中的指定条目
/*
* Parameters:
*   mac: mac address
* Return :
*/
void GuiBehind::removeRecentItem(QString mac){
    for(int i = 0; i < mRecentList.rowCount(); i ++){
        QStandardItem *oneItem = mRecentList.item(0);
        QVariant sender = oneItem->data(RecentListItemModel::Mac);
        QString mac_ = sender.toString().split(" ")[1];
        if(mac == mac_){
            mRecentList.removeRow(i);
            break;
        }
    }
}

// 显示聊天对话窗口
/*
* Parameters:
*   ip_mac: ip and mac address string
* Return :
*/
void GuiBehind::showSendPage(QString ip_mac)
{
    qDebug()<<"打开聊天窗口："<<ip_mac;

    if (ip_mac == "") {
        return ;
    }
    QStringList sections = ip_mac.split(" ");
    QString mac = "";
    if(sections.size() == 1){
        mac = ip_mac;
    }else{
        mac = sections[1];
    }
    QStandardItem *buddy = mBuddiesList.buddyByMac(mac);

    if (buddy == NULL) return;

    // 已经有实例
    if(this->cws.contains(mac)){
        foreach (ChatWidget *cw_, this->cws){
            cw_->focusOut();
        }

        ChatWidget *cw = this->cws.value(mac);
        cw->showme();
        cw->show();
        cw->raise();
        cw->focusIn();

        // 使用wmctrl命令切换聊天窗口
        QString wmcmd = "wmctrl -a " + cw->windowTitle();
        system(wmcmd.toUtf8().data());

    // 创建新实例
    }else{
        ChatWidget *cw = createCW(mac);
        cw->showme();
        cw->show();
    }

    // 停止头像动画和任务栏动画
    buddy->setData(false, BuddyListItemModel::ShowMsgAnim);
    this->alertTimer->stop();
//    this->mView->setWindowIcon(this->iconDukto);
}

// 创建新的对话框实例
/*
* Parameters:
*   mac: mac address
* Return :
*/
ChatWidget * GuiBehind::createCW(QString mac){
    QStandardItem *buddy = mBuddiesList.buddyByMac(mac);
    ChatWidget *cw = new ChatWidget();
    cw->theme = &mTheme;
    cw->buddies = &(mBuddiesList.mItemsMap);
    cw->dbuddy->fillFromItem(buddy);
    mIpAddresses.refreshIpList();
    cw->myIp = mIpAddresses.myIp;
    cw->firstStyle();
    connect(cw, SIGNAL(reSaveCw(QString)), this, SLOT(reSaveCw(QString)));
    connect(cw, SIGNAL(reBindSocket(ChatWidget*)), mDuktoProtocol, SLOT(connectSocketAndChatWidget(ChatWidget*)));

    this->cws.insert(QString(mac), cw);

    // 有tmp消息，全部append到对话框中
    if(this->tmpMsgs.contains(mac)){
        QStringList tmpList = this->tmpMsgs.value(mac);
        for(int i = 0; i < tmpList.length(); i ++){
            QString msg = tmpList[i];
            cw->addLogDirect(msg);
        }
        this->tmpMsgs.remove(mac);
    }

    // 多对话框初始位置偏移
    if(this->cwx == -1){
        QDesktopWidget* desktop = QApplication::desktop();
        this->cwx = (desktop->width() - cw->width()) / 2;
        this->cwy = (desktop->height() - cw->height()) / 2;
    }else{
        this->cwx += 25;
        this->cwy += 25;
    }
    cw->move(this->cwx, this->cwy);

    return cw;
}

// 使用IP发送消息成功，将map中的"IP"键替换为该mac，此后才能继续打开IP输入窗口
void GuiBehind::reSaveCw(QString mac){
    if(this->cws.contains(mac) == false){
        ChatWidget *cw = this->cws.value("MAC");
        this->cws.insert(mac, cw);
    }

    this->cws.remove("MAC");
}

// 激活头像动画提醒
void GuiBehind::newMsgAlert(QString mac){
    // 没有打开与该用户的对话框
    if(this->cws.contains(mac) == false || this->cws.value(mac)->isHidden() == true){
        QStandardItem *item = mBuddiesList.buddyByMac(mac);
        if(item != NULL){
            // 激活头像提示
            item->setData(true, BuddyListItemModel::ShowMsgAnim);
        }
        // 激活任务栏提示
        this->alertTimer->start();
        //TODO: check QtQuick 2 alert
//        QApplication::alert(mView, 5000);
    }
}

// 更改接收文件目录
void GuiBehind::changeDestinationFolder(){
//    QString dirname = QFileDialog::getExistingDirectory(mView, tr("Change folder"), ".",
//    QString dirname = QFileDialog::getExistingDirectory(mView, "更改目录", ".",
    QString dirname = QFileDialog::getExistingDirectory(nullptr, tr("Change folder"), ".",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dirname == "") return;

    // 将所选路径设置为当前程序环境默认路径
    QDir::setCurrent(dirname);

    // 保存路径
    setCurrentPath(dirname);
}

// 保存路径
void GuiBehind::setCurrentPath(QString path){
    if (path == mSettings->currentPath()) return;
    mSettings->savePath(path);
    emit currentPathChanged();
}

// 使用文件管理器打开文件接收目录
void GuiBehind::openDestinationFolder(){
    QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::currentPath()));
}

// 刷新本机IP
void GuiBehind::refreshIpList(){
    mIpAddresses.refreshIpList();
}

/*
* Summary: change theme color
* Parameters:
*   color: target color
* Return :
*/
void GuiBehind::changeThemeColor(QString color){
    mTheme.setThemeColor(color);
    mSettings->saveThemeColor(color);

    // 将所有的聊天窗口同步主题颜色
     QMap<QString, ChatWidget *>::iterator iter = cws.begin();
     while (iter != cws.end()) {
         iter.value()->showme();
         iter++;
     }
}

/*
* Summary: close app
* Return :
*/
void GuiBehind::close(){
    mDuktoProtocol->sayGoodbye();
}

// 获取图标路径，qml访问本地图片需使用绝对路径
QString GuiBehind::iconPath()
{
    return "file://" + mSettings->iconPath();
}

// 保存图标路径
void GuiBehind::setIconPath(QString path)
{
    if (path == mSettings->iconPath())
        return;
    mSettings->saveIconPath(path);
}

/*
* Summary: say hello
* Return :
*/
void GuiBehind::periodicHello(){
    mDuktoProtocol->sayHello(QHostAddress::Broadcast);
}

QString GuiBehind::currentPath()
{
    return mSettings->currentPath();
}

// 将 byte 转换成 KB 或 MB
/*
* Parameters:
*   size: size in byte
* Return :
*/
QString GuiBehind::sizeHuman(qint64 size){
    QString sizeFormatted;
    if (size < 1000)
        sizeFormatted = QString::number(size) + " B";
    else if (size < 1000000)
        sizeFormatted = QString::number(size * 1.0 / 1000, 'f', 1) + " KB";
    else
        sizeFormatted = QString::number(size * 1.0 / 1000000, 'f', 1) + " MB";

    return sizeFormatted;
}

// 窗口 icon 闪烁
void GuiBehind::msgAlert(){
//    if(this->timeFlag % 2 == 0){
//        this->mView->setIcon(this->iconBlank);
//    }else{
//        this->mView->setIcon(this->iconDukto);
//    }
    this->timeFlag ++;
}

// 查看用户手册
void GuiBehind::viewUserGuide()
{
    if (!mDaemonIpcDbus->daemonIsNotRunning()){
        //增加标题栏帮助菜单、F1快捷键打开用户手册
        mDaemonIpcDbus->showGuide("kylin-ipmsg");
    }
}
