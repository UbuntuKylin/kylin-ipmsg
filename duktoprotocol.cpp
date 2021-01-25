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
#include "platform.h"


DuktoProtocol::DuktoProtocol(GuiBehind *gb)
    : mSocket(NULL), mTcpServer(NULL)
{
    this->gbehind = gb;
    this->pSystemSignature = this->getSystemSignature();
}

DuktoProtocol::~DuktoProtocol()
{
    qDebug() << "~DuktoProtocol";
    if (mSocket) {
//        delete mSocket;
        mSocket->deleteLater();
        mSocket = NULL;
    }
    if (mTcpServer) {
        mTcpServer->close();
//        delete mTcpServer;
        mTcpServer->deleteLater();
        mTcpServer = NULL;
    }

    qDebug() << "discardSockets";
    // 释放废弃的socket类
    for(int i = 0; i < this->discardSockets.size(); i ++){
        KSocket *t_ = discardSockets[i];
//        delete t_;
        t_->deleteLater();
        t_ = NULL;
    }
}

/*
* Summary: init
* Return :
*/
void DuktoProtocol::initialize()
{
    mSocket = new QUdpSocket(this);
    mSocket->bind(QHostAddress::AnyIPv4, NETWORK_PORT, QUdpSocket::ReuseAddressHint);
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(newUdpData()));

    mTcpServer = new KTcpServer();
    qDebug() << "mTcpServer->listen" <<mTcpServer->listen(QHostAddress::AnyIPv4, NETWORK_PORT);
    connect(mTcpServer, SIGNAL(newConn(qintptr)), this, SLOT(newIncomingConnection(qintptr)));
}

// 生成本机标识
QString DuktoProtocol::getSystemSignature()
{
    static QString signature = "";
    if (signature != "") return signature;

    signature = "kylin-ipmsg " + Platform::getSystemUsername()
              + " " + Platform::getHostname()
              + " " + this->getMac()
              + " " + Platform::getPlatformName();
    return signature;
}

// 获取本机MAC地址
QString DuktoProtocol::getMac(){
//    foreach(QNetworkInterface ni, QNetworkInterface::allInterfaces()){
//        if(ni.name() != "lo"){
//            return ni.hardwareAddress();
//            break;
//        }
//    }

//    return "NOMAC";

    // caoliang 获取正确的本机MAC地址
    // 获取所有网络接口列表
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();
    int netCnt = nets.count();
    QString strMacAddr = "";

    for (int i = 0; i < netCnt; i++) {
        // 如果此网络接口被激活并且正在运行并且不是回环地址，则就是我们需要找的Mac地址
        if(nets[i].flags().testFlag(QNetworkInterface::IsUp) && nets[i].flags().testFlag(QNetworkInterface::IsRunning) && !nets[i].flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            strMacAddr = nets[i].hardwareAddress();
            return strMacAddr;
            break;
        }
    }
    return "NOMAC";
}

// 上线广播
void DuktoProtocol::sayHello(QHostAddress dest)
{
    QByteArray *packet = new QByteArray();

    // 广播
    if (dest == QHostAddress::Broadcast){
        packet->append(UDP_BROADCAST);
        packet->append(this->pSystemSignature);
        this->sendToAllBroadcast(packet);

    // 单播
    }else{
        packet->append(UDP_UNICAST);
        packet->append(this->pSystemSignature);
        this->mSocket->writeDatagram(packet->data(), packet->length(), dest, NETWORK_PORT);
    }

    delete packet;
    packet = NULL;
}

// 下线广播
void DuktoProtocol::sayGoodbye()
{
    QByteArray *packet = new QByteArray();
    packet->append(UDP_GOODBYE);
    packet->append(this->pSystemSignature);
    sendToAllBroadcast(packet);

    delete packet;
    packet = NULL;
}

// 广播发送
/*
* Parameters:
*   packet: broadcast msg packet
* Return :
*/
void DuktoProtocol::sendToAllBroadcast(QByteArray *packet)
{
    // 所有网络接口
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();

    for (int i = 0; i < ifaces.size(); i++)
    {
        // 每一个网络接口的所有IP地址
        QList<QNetworkAddressEntry> addrs = ifaces[i].addressEntries();

        for (int j = 0; j < addrs.size(); j++){
            // 如果该网络接口的该IP地址是IPV4，并且有广播地址，则发送广播
            if ((addrs[j].ip().protocol() == QAbstractSocket::IPv4Protocol) && (addrs[j].broadcast().toString() != ""))
            {
                mSocket->writeDatagram(packet->data(), packet->length(), addrs[j].broadcast(), NETWORK_PORT);
                mSocket->flush();
            }
        }
    }
}

// 收到广播
void DuktoProtocol::newUdpData()
{
    qDebug() << "收到广播newUdpData";
    while (mSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        const int maxLength = 65536;
        datagram.resize(maxLength);
        QHostAddress sender;
        int size = mSocket->readDatagram(datagram.data(), datagram.size(), &sender);
        datagram.resize(size);
        handleMessage(datagram, sender);
     }
}

// 处理广播消息
/*
* Parameters:
*   data: msg data
*   sender: msg sender
* Return :
*/
void DuktoProtocol::handleMessage(QByteArray &data, QHostAddress &sender)
{
    char msgtype = data.at(0);
    data.remove(0, 1);
    QString dataStr = QString::fromUtf8(data);
    switch(msgtype)
    {
        case UDP_BROADCAST:
        case UDP_UNICAST:

            if(dataStr != this->pSystemSignature && dataStr.startsWith("kylin-ipmsg ") == true){
                mPeers[dataStr] = Peer(sender, dataStr, NETWORK_PORT);

                /*print Qhash*/
                QHash<QString , Peer>::iterator it_begin = mPeers.begin();
                QHash<QString , Peer>::iterator it_end = mPeers.end();
                while (it_begin != it_end) {
                       qDebug() << "QHash<QString , Peer>" << it_begin.key() << it_begin.value().address;
                       it_begin++;
                }

                if(msgtype == UDP_BROADCAST){
                    sayHello(sender);
                }

                emit peerListAdded(mPeers[dataStr]);
            }
            break;

        case UDP_GOODBYE:
            emit peerListRemoved(mPeers[dataStr]);
            mPeers.remove(sender.toString());
            break;
    }
}

// 主动连接
/*
* Parameters:
*   targetip: target ip address
*   remoteid: remote id
*   cw: chat widget object
* Return :
*/
void DuktoProtocol::newOutgoingConnection(QString targetIP, QString remoteID, ChatWidget *cw){
    qDebug() << "客户端主动连接函数 : newOutgoingConnection";
    qDebug() << "客户端主动链接函数参数 : " << targetIP << remoteID;
    QThread *qthread = new QThread();
    KSocket *ks = new KSocket(targetIP, this->pSystemSignature, remoteID);
    ks->moveToThread(qthread);

    connect(cw, SIGNAL(sendMsg(QString)), ks, SLOT(sendText(QString)));
    connect(ks, SIGNAL(sendTextComplete()), cw, SLOT(onSendCompleted()));
    connect(ks , SIGNAL(sendTextComplete_add_recentlist(QString , QString)) , this->gbehind , SLOT(sendTextComplete_add_recentlist(QString , QString)));
    connect(ks, SIGNAL(receiveTextComplete(QString, QString)), this->gbehind, SLOT(receiveTextComplete(QString, QString)));

    connect(cw, SIGNAL(sendFiles(QStringList)), ks, SLOT(sendFiles(QStringList)));
    connect(cw, SIGNAL(sendDir(QString)), ks, SLOT(sendDir(QString)));
    connect(cw, SIGNAL(sendFileCanceled()), ks, SLOT(sendCancel()));
    connect(ks, SIGNAL(startTransfer(bool)), cw, SLOT(startTransfer(bool)));
    connect(ks, SIGNAL(updateTransferStatus(int, QString, bool)), cw, SLOT(updateTransferStatus(int, QString, bool)));
    connect(ks, SIGNAL(sendFileComplete()), cw, SLOT(onSendCompleted()));
    connect(ks, SIGNAL(receiveFileComplete(QStringList*, qint64, QString, QString)), this->gbehind, SLOT(receiveFileComplete(QStringList*, qint64, QString, QString)));
    connect(ks, SIGNAL(receiveFileCancelled()), cw, SLOT(recvCancel()));
    connect(ks, SIGNAL(isInitiativeConn(bool)), cw, SLOT(getIsInitiativeConn(bool)));
    connect(ks, SIGNAL(addUpBuddy(QString , QString , QString , QString , QString)), this, SLOT(addUpBuddy(QString , QString , QString , QString , QString)));

    connect(ks, SIGNAL(transferMsgSignal(int)), cw, SLOT(slotTransferMsg(int)));

    connect(qthread, SIGNAL(started()), ks, SLOT(imStart()));

    connect(ks, SIGNAL(finished()), qthread, SLOT(quit()));
    connect(qthread, SIGNAL(finished()), qthread, SLOT(deleteLater()));
    connect(ks, SIGNAL(finished()), ks, SLOT(deleteLater()));
    connect(ks, SIGNAL(finished(QString)), this, SLOT(updateSockets(QString)));
    connect(ks, SIGNAL(finished(bool)), cw, SLOT(setOnLine(bool)));

    this->gbehind->sockets.insert(remoteID, ks);
    qthread->start();
}

// 被动连接
/*
* Parameters:
*   socketdescriptor: incoming connection socket
* Return :
*/
void DuktoProtocol::newIncomingConnection(qintptr socketDescriptor)
{
    qDebug() << "服务端被动连接newIncomingConnection";
    QThread *qthread = new QThread();
    KSocket *ks = new KSocket(socketDescriptor , this->pSystemSignature);
    ks->moveToThread(qthread);

    connect(ks, SIGNAL(updateRemoteID(QString, QString , QString , QString , QString ,KSocket*)), this, SLOT(updateRemoteID(QString, QString , QString , QString , QString , KSocket*)));

    connect(ks, SIGNAL(receiveTextComplete(QString, QString)), this->gbehind, SLOT(receiveTextComplete(QString, QString)));

    connect(ks, SIGNAL(receiveFileComplete(QStringList*, qint64, QString, QString)), this->gbehind, SLOT(receiveFileComplete(QStringList*, qint64, QString, QString)));

    connect(qthread, SIGNAL(started()), ks, SLOT(imReady()));
    connect(qthread, SIGNAL(finished()), qthread, SLOT(deleteLater()));
    connect(ks, SIGNAL(finished()), ks, SLOT(deleteLater()));
    connect(ks, SIGNAL(finished(QString)), this, SLOT(updateSockets(QString)));

    qthread->start();
}

// 被动连接后，收到对方remoteID，更新信息
/*
* Parameters:
*   premoteid: remote id in
*   ks: kylin socket object
* Return :
*/
void DuktoProtocol::updateRemoteID(QString ip, QString user_name , QString system , QString pRemoteID , QString Platform , KSocket* ks){
    // 添加下级网络好友，如果已有该好友，addBuddy()中不会重复添加
    qDebug() << "服务端更新信息：" << "ip :" << ip << "\n" << "user_name :" << user_name << "\n" << "system :" << system << "\n" << "mac :" << pRemoteID << "\n" << "Platform :" << Platform << "\n";

    this->gbehind->mBuddiesList.addBuddy(ip, NETWORK_PORT, user_name , system , pRemoteID, Platform , QUrl(""));

    // 保存到map中供界面使用
    if(this->gbehind->sockets.contains(pRemoteID) == false){
        this->gbehind->sockets.insert(pRemoteID, ks);
    }else{
        KSocket *ks_ = this->gbehind->sockets.value(pRemoteID);
        if((unsigned long)ks != (unsigned long)ks_){
            this->discardSockets.append(ks_);
            this->gbehind->sockets.remove(pRemoteID);
            this->gbehind->sockets.insert(pRemoteID, ks);
        }
    }

    // 被动连接，绑定信号
    ChatWidget *cw = NULL;
    foreach (ChatWidget *cw_, this->gbehind->cws) {
        QString oneMac = cw_->dbuddy->ip().split(" ")[1];
        if(oneMac == pRemoteID){
            cw = cw_;
        }
    }
    if(cw == NULL){
        cw = this->gbehind->createCW(pRemoteID);
    }
    cw->setOnLine(true);

    connect(cw, SIGNAL(sendMsg(QString)), ks, SLOT(sendText(QString)));
    connect(ks, SIGNAL(sendTextComplete()), cw, SLOT(onSendCompleted()));

    connect(cw, SIGNAL(sendFiles(QStringList)), ks, SLOT(sendFiles(QStringList)));
    connect(cw, SIGNAL(sendDir(QString)), ks, SLOT(sendDir(QString)));
    connect(cw, SIGNAL(sendFileCanceled()), ks, SLOT(sendCancel()));
    connect(ks, SIGNAL(startTransfer(bool)), cw, SLOT(startTransfer(bool)));
    connect(ks, SIGNAL(updateTransferStatus(int, QString, bool)), cw, SLOT(updateTransferStatus(int, QString, bool)));
    connect(ks, SIGNAL(sendFileComplete()), cw, SLOT(onSendCompleted()));
    connect(ks, SIGNAL(receiveFileCancelled()), cw, SLOT(recvCancel()));
    connect(ks, SIGNAL(isInitiativeConn(bool)), cw, SLOT(getIsInitiativeConn(bool)));

    connect(ks, SIGNAL(transferMsgSignal(int)), cw, SLOT(slotTransferMsg(int)));

    connect(ks, SIGNAL(finished(bool)), cw, SLOT(setOnLine(bool)));

    qDebug()<<"活跃sockets: "<<this->gbehind->sockets.size()<<" 废弃sockets: "<<this->discardSockets.size();
}

// 清除sockets中的指定socket
/*
* Parameters:
*   premoteid: remote id
* Return :
*/
void DuktoProtocol::updateSockets(QString pRemoteID){
    qDebug() << "DuktoProtocol::updateSockets";
    if(this->gbehind->sockets.contains(pRemoteID) == true){
        KSocket *ks_ = this->gbehind->sockets.value(pRemoteID);
        qDebug() << "KSocket *ks_";
        this->discardSockets.append(ks_);
        qDebug() << "discardSockets.append" << pRemoteID ;
        this->gbehind->sockets.remove(pRemoteID);
        qDebug() << "sockets.remove(pRemoteID)";
    }
}

// 绑定聊天窗口和socket
/*
* Parameters:
*   cw: chat widget object
* Return :
*/
void DuktoProtocol::connectSocketAndChatWidget(ChatWidget *cw){
    qDebug() << "active link tcp , create chatwidget and socket contact";

    QString ip = cw->dbuddy->ip().split(" ")[0];
    QString remoteID = cw->dbuddy->ip().split(" ")[1];

    // 主动连接，为聊天窗口建立新的socket连接
    if(this->gbehind->sockets.contains(remoteID) == false){
        this->newOutgoingConnection(ip, remoteID, cw);

    }else{
        KSocket *ks_ = this->gbehind->sockets.value(remoteID);
        // 已有连接已断开，但还没来得及销毁。销毁并建立主动连接
        if(ks_->isConnected == false){
            qDebug() << "connectSocketAndChatWidget";
            this->updateSockets(remoteID);
            this->newOutgoingConnection(ip, remoteID, cw);
        }
    }
}

// 添加上层网络好友
/*
* Parameters:
*   ip: friend ip address
*   mac: friend mac address
* Return :
*/
void DuktoProtocol::addUpBuddy(QString ip, QString user_name , QString system , QString mac , QString Platform){
    // 如果已有该好友，addBuddy()中不会重复添加
    this->gbehind->mBuddiesList.addBuddy(ip, NETWORK_PORT, user_name , system , mac, Platform , QUrl(""));
}
