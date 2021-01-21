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

#include "ksocket.h"


// 被动连接端初始化
KSocket::KSocket(qintptr s, QString systemSignature , QObject *parent):QObject(parent){
    qDebug() << "服务端被动连接初始化 : KSocket";
    qDebug() << "服务端被动链接初始化参数 : " << s;
    this->socketDescriptor = s;
    this->pSystemSignature = systemSignature;
    this->isConnected = false;
}

// 主动连接端初始化
/*
* Parameters:
*   targetip: target ip
*   systemsignature: system sig
*   premoteid: remote id
*   parent: parent object
* Return :
*/
KSocket::KSocket(QString targetIP, QString systemSignature, QString pRemoteID, QObject *parent):QObject(parent){
    qDebug() << "客户端主动连接初始化 : KSocket";
    qDebug() << "客户端主动链接初始化参数" <<targetIP << systemSignature << pRemoteID;
    this->pTargetIP = targetIP;
    this->pSystemSignature = systemSignature;
    this->pRemoteID = pRemoteID;

    this->timeoutCount = 0;
    this->isConnected = false;
}

KSocket::~KSocket(){
    qDebug() << "~KSocket";
    if(pReceivedFiles){
        delete pReceivedFiles;
        pReceivedFiles = NULL;
    }
}

// 被动端开始工作
void KSocket::imReady(){
    this->socket = new QTcpSocket();
    this->socket->setSocketDescriptor(this->socketDescriptor);
    connect(socket, SIGNAL(readyRead()), this, SLOT(handleMsg()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(finishThread()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)), Qt::DirectConnection);

    // 监听附socket连接
    this->pTcpServer = new QTcpServer();
    bool rtn = this->pTcpServer->listen(QHostAddress::AnyIPv4, NETWORK_PORT - 1);
    connect(pTcpServer, SIGNAL(newConnection()), this, SLOT(newSecondaryConn()));

    this->pTargetIP = socket->peerAddress().toString();
    this->pTotalLenToReceive = 0;
    this->pTotalReceivedLen = 0;
    this->pReadType = MSGTYPE;
    this->pReceivedFiles = new QStringList();
    this->isConnected = true;
    this->isInitiative = false;

    qDebug()<<"服务端socket被动连接成功, thread: "<<QThread::currentThreadId()<<" 附tcpserver启动: "<<rtn;

    // 朋友,请开始你的表演.
    /*modify by jsj at 2021-01-18 11:12*/
    QString system_flag;
    system_flag.clear();
    system_flag = this->pSystemSignature;

    std::string std_system_flag = system_flag.toStdString();
    const char *p_system_flag = std_system_flag.c_str();

    socket->write(S_IAMREADY);
    socket->write(p_system_flag);
    socket->waitForBytesWritten();
    socket->flush();
}

// 被动附socket连接
void KSocket::newSecondaryConn(){
    if (!pTcpServer->hasPendingConnections()) return;

    this->socketSecondary = pTcpServer->nextPendingConnection();

    connect(socketSecondary, SIGNAL(readyRead()), this, SLOT(handleMsgSecondary()));
    connect(socketSecondary, SIGNAL(disconnected()), this, SLOT(finishThread()));

    qDebug()<<"被动附socket已经启动.";
}

// 主动端开始工作
void KSocket::imStart(){
    qDebug() << "客户端socket主动链接";
    //qDebug() << "KSocket::imStart()";
    socket = new QTcpSocket();
    connect(socket, SIGNAL(connected()), this, SLOT(imStart_()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(handleMsg()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(finishThread()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)), Qt::DirectConnection);
    socket->connectToHost(this->pTargetIP, NETWORK_PORT);

    // 超时计时器
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timoutOnce()));
    timer->start(3000);
}

// 主动连接单次超时处理
void KSocket::timoutOnce(){
    timeoutCount ++;
    qDebug()<<"发起Tcp连接超时 "<<timeoutCount<<" 次";

    // 超时次数到，向主界面发送超时信号
    if(timeoutCount > 3){
        disconnect(timer, SIGNAL(timeout()), this, SLOT(timoutOnce()));
        isConnected = false;
        emit transferMsgSignal(CONN_TIMEOUT);
        qDebug()<<"超时次数过多，连接失败...";

    }else{
        socket->abort();
        qDebug() << "this->pTargetIP" << this->pTargetIP;
        socket->connectToHost(this->pTargetIP, NETWORK_PORT);
    }
}

// 主动连接成功，取消超时计时器
void KSocket::imStart_(){
    qDebug() << "客户端socket动链接成功";
    qDebug() << "KSocket::imStart_()";
    disconnect(timer, SIGNAL(timeout()), this, SLOT(timoutOnce()));
    timer->stop();
    isConnected = true;
    isInitiative = true;

    this->pTotalLenToReceive = 0;
    this->pTotalReceivedLen = 0;
    this->pReadType = MSGTYPE;
    this->pReceivedFiles = new QStringList();

    // 添加上层好友
    //qDebug() << this->pTargetIP << "   " << this->pRemoteID;

#if 0
    /*发送建立链接成功后的第一次消息*/
    char p_first_send[11];
    memset(p_first_send , 0x00 , sizeof(p_first_send));
    sprintf(p_first_send , "first_send");
    socket->write(p_first_send , strlen(p_first_send));
#endif

    //emit addUpBuddy(this->pTargetIP, this->pRemoteID);

    qDebug()<<"socket主动连接成功, thread: "<<QThread::currentThreadId()<<" socket: "<<socket;
}

/*
 * 消息处理函数
 * 消息头为16进制，一次信息由两类消息构成：头，数据
 * 接收方每收到一个文件，发送ENDONE告知发送方
 *
 * 文字: 第一条消息：TEXT；第二条消息：DATA
 * 文件: (第一条消息：FILE 或 DIR；第二条消息：DATA) * N；第三条消息：END
 *
 * TEXT: A0A1 + 9位总长度
 * FILE: A0A2 + 9位总长度 + 9位本条长度 + 9位文件名长度 + 文件名
 * DIR：A0A3 + 9位文件夹名长度 + 文件夹名
 * DATA：A0A0 + 变长数据
 * END：A0A9
 * ENDONE：A0A8
 *
 * WHOAMI：A0B1 + 变长数据
 *
 */
void KSocket::handleMsg(){
    qDebug() << "KSocket::handleMsg()";
    while (socket->bytesAvailable() > 0){

        if(this->pReadType == MSGTYPE){

            if(socket->bytesAvailable() < MSG_SIZE_MIN){
                qDebug()<<"\n本条消息长度错误 wrong msg size: "<<socket->bytesAvailable();
                return;
            }

            // 读取msgType
            QByteArray msgType = socket->read(4);
            socket->flush();
            QString mt = QString::fromUtf8(msgType);

            // 被动端ready消息
            if(mt == S_IAMREADY){
                /*read passive system flag*/
                qDebug() << "---clien receive server iamready flag";
                QByteArray system_falg = socket->readAll();
                QString tmp(system_falg);
                QStringList flag = tmp.split(" ");
                if (flag.count() > 4) {
                    QString mac = flag.at(3);
                    QString ip = socket->peerAddress().toString();
                    QString user_name = flag.at(1);
                    QString system = flag.at(2);
                    QString platfrom = flag.at(4);

                    qDebug() << "s_iamready : " << "\n" << "ip is " << ip << "\n" << "user_name = " << user_name << "\n" << "system = " << system << "\n" << "mac = " << mac << "\n" << "platfrom = " << platfrom;

                    emit addUpBuddy(ip , user_name , system , mac , platfrom);
                }


                // 主动附socket连接
                socketSecondary = new QTcpSocket();
                connect(socketSecondary, SIGNAL(readyRead()), this, SLOT(handleMsgSecondary()));
                connect(socketSecondary, SIGNAL(disconnected()), this, SLOT(finishThread()));
                socketSecondary->connectToHost(this->pTargetIP, NETWORK_PORT - 1);
#if 0
                QString msg = C_WHOAMI;
                QString pRemoteID = this->pSystemSignature.split(" ")[3];
                msg.append(this->comLen(pRemoteID));
                msg.append(pRemoteID);
#endif
                /*modify by jsj at 2021-01-18 11:12*/
                QString system_flag;
                system_flag.clear();
                system_flag = this->pSystemSignature;

                std::string std_system_flag = system_flag.toStdString();
                const char *p_system_flag = std_system_flag.c_str();

                socket->write(C_WHOAMI);
                socket->write(p_system_flag);
                socket->waitForBytesWritten();
                socket->flush();
 /*
                QString msg = C_WHOAMI;
                QString pRemoteID = this->pSystemSignature.split(" ")[3];
                msg.append(this->comLen(pRemoteID));
                msg.append(pRemoteID);

                socket->write(msg.toUtf8().data());
                socket->flush();
*/
                emit transferMsgSignal(CONN_SUCCESS);
            }

            // 主动端自报姓名消息
            else if(mt == C_WHOAMI){
                qDebug() << "---server receive clien c_whoami flag";
                QByteArray system_falg = socket->readAll();
                QString tmp(system_falg);
                QStringList flag = tmp.split(" ");
                if (flag.count() > 4) {
                    QString mac = flag.at(3);
                    QString ip = socket->peerAddress().toString();
                    QString user_name = flag.at(1);
                    QString system = flag.at(2);
                    QString platfrom = flag.at(4);

                    qDebug() << "c_whoami" << "\n" << "ip is " << ip << "\n" << "user_name = " << user_name << "\n" << "system = " << system << "\n" << "mac = " << mac << "\n" << "platfrom = " << platfrom;

                   // emit addUpBuddy(ip , user_name , system , mac , platfrom);
                    emit updateRemoteID(ip , user_name , system , mac , platfrom , this);
                }

#if 0
                int whoamiLen = socket->read(9).toInt(NULL, 16);
                this->pRemoteID = QString::fromUtf8(socket->read(whoamiLen));
                emit updateRemoteID(this->pRemoteID, this);
#endif
                emit transferMsgSignal(CONN_SUCCESS);
            }

            // 文字消息头
            else if(mt == C_TEXT){
                this->pCurrentMsgType = C_TEXT;
                this->pReadType = DATA;

                this->pTotalLenToReceive = socket->read(9).toInt(NULL, 16);
                this->pReceiveText = "";
            }

            // 文件消息头
            else if(mt == C_FILE){
                this->pCurrentMsgType = C_FILE;
                this->pReadType = DATA;

                this->pCurrentMsgReceivedLen = 0;

                this->pTotalLenToReceive = socket->read(9).toLongLong(NULL, 16);
                this->pCurrentMsgLen = socket->read(9).toLongLong(NULL, 16);
                int fileNameLen = socket->read(9).toInt(NULL, 16);
                QString name = QString::fromUtf8(socket->read(fileNameLen));

                // 本文件为发送文件夹中的文件，将根目录改为重命名之后的
                if ((name.indexOf('/') != -1) && (name.section("/", 0, 0) == pRootDir)){
                    name = name.replace(0, name.indexOf('/'), pRootDirRename);
                }

                // 如果文件重名，则重命名
                int i = 2;
                QString originalName = name;
                while (QFile::exists(name)) {
                    QFileInfo fi(originalName);
                    name = fi.baseName() + " (" + QString::number(i) + ")." + fi.completeSuffix();
                    i++;
                }

                this->pReceiveFile = new QFile(name);

                bool ret = this->pReceiveFile->open(QIODevice::WriteOnly);
                if (!ret){
                    emit transferMsgSignal(7);
                }

                // 该文件大小为0，只创建，不进入DATA接收步骤
                if(pCurrentMsgLen == 0){
                    this->pReceivedFiles->append(pReceiveFile->fileName());
                    pReceiveFile->close();
//                    delete pReceiveFile;
                    pReceiveFile->deleteLater();
                    pReceiveFile = NULL;

                    // 使用附socket告知对方，单个文件接收完成，可以发送下一个文件
                    QString endOne = C_END_ONEFILE;
                    socketSecondary->write(endOne.toUtf8().data());
                    socketSecondary->flush();

                    this->pReadType = MSGTYPE;
                }

                emit startTransfer(false);
            }

            // 文件夹消息头
            else if(mt == C_DIR){
                this->pCurrentMsgType = C_DIR;

                int dirNameLen = socket->read(9).toInt(NULL, 16);
                QString name = QString::fromUtf8(socket->read(dirNameLen));

                // 根目录
                if(name.indexOf("/") == -1){
                    // 如果文件夹重名，则重命名
                    int i = 2;
                    QString originalName = name;
                    while (QFile::exists(name))
                        name = originalName + " (" + QString::number(i++) + ")";

                    this->pRootDir = originalName;
                    this->pRootDirRename = name;
                }
                // 子目录
                else{
                   name = name.mid(this->pRootDir.length());
                   name = this->pRootDirRename + name;
                }

                // 创建文件夹
                QDir dir(".");
                bool ret = dir.mkpath(name);
                if (!ret){
                    emit transferMsgSignal(6);
                }

                this->pReceivedFiles->append(name);

                // 使用附socket告知对方，单个文件夹接收完成，可以发送下一个
                QString endOne = C_END_ONEDIR;
                socketSecondary->write(endOne.toUtf8().data());
                socketSecondary->flush();
            }

            // 本次信息结束符
            else if(mt == C_END){
                if(this->pCurrentMsgType == C_TEXT){
                    qDebug()<<"一次文本信息接收完毕: "<<QString::fromUtf8(this->pReceiveText);

                    emit receiveTextComplete(QString::fromUtf8(this->pReceiveText), this->pRemoteID);

                    this->pTotalReceivedLen = 0;

                }else if(this->pCurrentMsgType == C_FILE || this->pCurrentMsgType == C_DIR){
                    qDebug()<<"一次文件传输接收完毕: "<<this->pTotalLenToReceive<<" "<<this->pTotalReceivedLen;

                    QStringList *listToShow = new QStringList();
                    listToShow->append(*pReceivedFiles);
                    emit receiveFileComplete(listToShow, this->pTotalReceivedLen, this->pRootDirRename, this->pRemoteID);

                    this->pRootDir = "";
                    this->pRootDirRename = "";
                    this->pTotalReceivedLen = 0;
                    this->pReceivedFiles->clear();

                }else{
                    qDebug()<<"\n1本条数据结束符消息类型错误 wrong end msg type: "<<this->pCurrentMsgType;
                    emit transferMsgSignal(7);
                }
            }

            else{
//                qDebug()<<"\n2本条消息头类型错误 wrong head msg type: "<<mt;
                emit transferMsgSignal(8);
                break;
            }
        }

        // 实际数据传输
        else{
            if(this->pCurrentMsgType == C_TEXT){
                this->receiveText();
            }else if(this->pCurrentMsgType == C_FILE){
                this->receiveFile();
            }else{
                qDebug()<<"\n3本条数据消息类型错误 wrong data msg type: "<<this->pCurrentMsgType;
                emit transferMsgSignal(9);
            }
        }
    }
}

// 附属 socket 消息处理
void KSocket::handleMsgSecondary(){
    while (socketSecondary->bytesAvailable() > 0){
        if(socketSecondary->bytesAvailable() < MSG_SIZE_MIN){
            qDebug()<<"\nsocketSecondary本条消息长度错误 wrong msg size: "<<socketSecondary->bytesAvailable();
            break;
        }

        // 读取msgType
        QByteArray msgType = socketSecondary->read(4);
        socketSecondary->flush();
        QString mt = QString::fromUtf8(msgType);

        // 单个文件接收完成响应
        if(mt == C_END_ONEFILE){
            this->sendOneFile();
        }

        // 单个文件夹接收完成响应
        else if(mt == C_END_ONEDIR){
            this->sendDirs();
        }

        // 文件传输取消响应
        else if(mt == C_CANCEL){
            // 告知聊天界面是否主动连接
            emit isInitiativeConn(this->isInitiative);
            emit receiveFileCancelled();
            // 断开连接，解绑废弃ksocket
            qDebug() << "handleMsgSecondary";
            this->finishThread();
            break;
        }

        else{
            qDebug()<<"\n2socketSecondary本条消息头类型错误 wrong head msg type: "<<mt;
            emit transferMsgSignal(8);
        }
    }
}

// 接收文字
void KSocket::receiveText(){
    // 计算本次接收长度
    qint64 leftLen = pTotalLenToReceive - pTotalReceivedLen;
    qint64 readLen = (socket->bytesAvailable() > leftLen) ? leftLen : socket->bytesAvailable();

    QByteArray data = socket->read(readLen);
    pTotalReceivedLen += readLen;
    pReceiveText.append(data);

    // 约定长度已经全部读取完成，将接收模式置为MSGTYPE，等待END消息
    if(pTotalReceivedLen == pTotalLenToReceive){
        this->pReadType = MSGTYPE;
    }
}

// 接收单个文件
void KSocket::receiveFile(){
    // 计算本次接收长度
    qint64 leftLen = pCurrentMsgLen - pCurrentMsgReceivedLen;
    qint64 readLen = (socket->bytesAvailable() > leftLen) ? leftLen : socket->bytesAvailable();

    QByteArray data = socket->read(readLen);
    pCurrentMsgReceivedLen += readLen;
    pTotalReceivedLen += readLen;
    pReceiveFile->write(data);

    // 更新进度条
    int percent = this->pTotalReceivedLen * 100.0 / this->pTotalLenToReceive;
    QString text = this->sizeHuman(this->pTotalReceivedLen) + " / " + this->sizeHuman(this->pTotalLenToReceive);
    emit updateTransferStatus(percent, text, false);

    // 单个文件已经全部读取完成
    if(pCurrentMsgReceivedLen == pCurrentMsgLen){
        this->pReceivedFiles->append(pReceiveFile->fileName());
        pReceiveFile->close();
//        delete pReceiveFile;
        pReceiveFile->deleteLater();
        pReceiveFile = NULL;

        // 使用附socket告知对方，单个文件接收完成，可以发送下一个文件
        QString endOne = C_END_ONEFILE;
        socketSecondary->write(endOne.toUtf8().data());
        socketSecondary->flush();

        this->pReadType = MSGTYPE;
    }
}

// 发送文字消息
void KSocket::sendText(QString text){
    if(this->isConnected == true){
        QString head = C_TEXT;
        head.append(this->comLen(text));
        socket->write(head.toUtf8().data());

        socket->write(text.toUtf8().data());

        QString end = C_END;
        socket->write(end.toUtf8().data());
        socket->flush();

        qDebug() << "发送成功";
        emit sendTextComplete();
    }
    else{
        emit transferMsgSignal(DISCONN);
    }
}

// 发送文件夹预处理
void KSocket::sendDir(QString dir){
    if(this->isConnected == true){
        this->tmpFiles.clear();
        this->pDirsToSend.clear();
        this->pRootPath = dir.left(dir.lastIndexOf("/") + 1);

        this->listAllFiles(dir);

        this->sendDirs();
    }
    else{
        emit transferMsgSignal(DISCONN);
    }
}

// 递归获取所有文件夹和文件路径
void KSocket::listAllFiles(QString fileName){
    QFileInfo fi(fileName);

    if(fi.isFile()){
        this->tmpFiles.append(fileName);
    }

    if(fi.isDir()){
        this->pDirsToSend.append(fileName);

        QStringList entries = QDir(fileName).entryList(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
        for (int i = 0; i < entries.count(); i++){
            this->listAllFiles(fileName + "/" + entries.at(i));
        }
    }
}

// 发送文件夹消息
void KSocket::sendDirs(){
    if(this->isConnected == true){
        if(this->pDirsToSend.size() != 0){
            QString head = C_DIR;
            QString dir = this->pDirsToSend.at(0);
            QString dirRalative = dir.mid(this->pRootPath.length());
            head.append(this->comLen(dirRalative));
            head.append(dirRalative);
            this->pDirsToSend.removeFirst();

            socket->write(head.toUtf8().data());
        }
        else{
            // 全部文件夹发送完成，开始发送文件
            this->sendFiles(this->tmpFiles);
        }
    }
    else{
        emit transferMsgSignal(TRANSFERERR);
    }
}

// 发送文件预处理
void KSocket::sendFiles(QStringList files){
    if(this->isConnected == true){
        bool isSkipFile = false;
        this->pFilesToSend.clear();
        this->pTotalLenToSend = 0;
        this->pTotalSendLen = 0;

        for(int i = 0; i < files.size(); i ++){
            QFile *file = new QFile(files.at(i));
            QFileInfo finfo(*file);

            if(finfo.isReadable() == true){
                if(finfo.isSymLink() == false){
                    this->pTotalLenToSend += file->size();
                    this->pFilesToSend.append(file);
                }else{
                    qDebug()<<"该文件是符号链接，跳过: "<<file->fileName();
                    isSkipFile = true;
                }
            }else{
                qDebug()<<"你没有该文件的读权限，跳过: "<<file->fileName();
                isSkipFile = true;
            }
        }

        if(this->pFilesToSend.size() != 0){
            emit startTransfer(true);
            this->sendOneFile();

            // 提示有跳过文件
            if(isSkipFile == true){
                emit transferMsgSignal(SKIPFILE);
            }
        }
    }
    else{
        emit transferMsgSignal(DISCONN);
    }
}

// 发送单个文件
void KSocket::sendOneFile(){
    if(this->isConnected == true){
        if(this->pFilesToSend.size() != 0){
            this->pSendFile = this->pFilesToSend.at(0);
            this->pCurrentFileLenToSend = this->pSendFile->size();
            this->pCurrentFileSendLen = 0;
            this->pFilesToSend.removeFirst();
            this->sendFileHead();
        }
        else{
            // 全部文件发送完成，发送结束消息
            QString end = C_END;
            socket->write(end.toUtf8().data());
            socket->flush();

            this->pRootPath = "";

            emit sendFileComplete();
        }
    }
    else{
        emit transferMsgSignal(TRANSFERERR);
    }
}

// 发送文件消息头
void KSocket::sendFileHead(){
    if(this->isConnected == true){
        this->pSendFile->open(QIODevice::ReadOnly);
        // 知识点: 调用waitForBytesWritten()不会触发bytesWritten()信号
        connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(sendData()));

        QString head = C_FILE;
        head.append(this->formatLen(this->pTotalLenToSend));
        head.append(this->formatLen(this->pCurrentFileLenToSend));

        // 无文件根目录，本次是发送文件ccc.txt
        // /home/kylin/bbb/ccc.txt >> ccc.txt
        if(this->pRootPath == ""){
            head.append(this->comLen(this->pSendFile->fileName().section("/", -1, -1)));
            head.append(this->pSendFile->fileName().section("/", -1, -1));
        }
        // 有文件根目录，本次是发送文件夹bbb
        // /home/kylin/bbb/ccc.txt >> bbb/ccc.txt
        else{
            head.append(this->comLen(this->pSendFile->fileName().mid(this->pRootPath.length())));
            head.append(this->pSendFile->fileName().mid(this->pRootPath.length()));
        }

        //
        qDebug()<<"!!!  "<<head;
        //

        // 通过connect(socket, SIGNAL(bytesWritten(qint64)), ... 触发sendData()
        socket->write(head.toUtf8().data());
    }
    else{
        emit transferMsgSignal(TRANSFERERR);
    }
}

// 发送单个文件数据
void KSocket::sendData(){
    if(this->isConnected == true){

        if((this->pCurrentFileLenToSend - this->pCurrentFileSendLen) > 0){
            int sendLen = 0;
            if((this->pCurrentFileLenToSend - this->pCurrentFileSendLen) >= 10240){
                sendLen = 10240;
            }else{
                sendLen = this->pCurrentFileLenToSend - this->pCurrentFileSendLen;
            }

            QByteArray data = this->pSendFile->read(sendLen);

            socket->write(data);

            this->pTotalSendLen += data.size();
            this->pCurrentFileSendLen += data.size();

            // 更新进度条
            int percent = this->pTotalSendLen * 100.0 / this->pTotalLenToSend;
            QString text = this->sizeHuman(this->pTotalSendLen) + " / " + this->sizeHuman(this->pTotalLenToSend);
            emit updateTransferStatus(percent, text, true);
        }

        // 单个文件发送完成
        else{
            disconnect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(sendData()));

            this->pSendFile->close();
//            delete this->pSendFile;
            this->pSendFile->deleteLater();
            this->pSendFile = NULL;
        }
    }

    else{
        emit transferMsgSignal(TRANSFERERR);
    }
}

// 发送取消文件传输消息
void KSocket::sendCancel(){
    if(this->isConnected == true){
        QString head = C_CANCEL;
        socketSecondary->write(head.toUtf8().data());
        socketSecondary->flush();

        // 告知聊天界面是否主动连接
        emit isInitiativeConn(this->isInitiative);
        // 断开连接，解绑废弃ksocket
        qDebug() << "sendCancel";
        this->finishThread();
    }
    else{
        emit transferMsgSignal(DISCONN);
    }
}

// 断开连接时的结束操作
void KSocket::finishThread(){
    qDebug() << "KSocket::finishThread()";

    if (socket) {
        disconnect(socket, SIGNAL(disconnected()), this, SLOT(finishThread()));
    }
    if (socketSecondary) {
        disconnect(socketSecondary, SIGNAL(disconnected()), this, SLOT(finishThread()));
    }

//    disconnect(socket, SIGNAL(disconnected()), this, SLOT(finishThread()));
//    disconnect(socketSecondary, SIGNAL(disconnected()), this, SLOT(finishThread()));

    qDebug() << "this->isConnected" << this->isConnected;
    if(this->isConnected == true){
        this->isConnected = false;

        if(socket){
            socket->close();
            socket->deleteLater();
            socket = NULL;
        }

        if(socketSecondary){
            socketSecondary->close();
            socketSecondary->deleteLater();
            socketSecondary = NULL;
        }

        qDebug() << this->isInitiative;
        if(this->isInitiative == false && pTcpServer){
            pTcpServer->close();
            qDebug() << "pTcpServer close";
//            delete pTcpServer;
            // caoliang 防止内存泄露
            pTcpServer->deleteLater();
            qDebug() << "delete pTcpServer";
            pTcpServer = NULL;
        }

//        emit finished();
        qDebug() << "emit finished()";
        emit finished(this->pRemoteID);
        qDebug() << "emit finished(this->pRemoteID)";
        emit finished(false);
        qDebug() << "emit finished(false)";
    }
    qDebug() << "out out KSocket::finishThread()";
}

// socket标准错误处理
void KSocket::socketError(QAbstractSocket::SocketError se){
    qDebug() << "KSocket::socketError()";
    qDebug()<<"socket错误："<<se;
}

// 将byte转换成KB或MB
QString KSocket::sizeHuman(qint64 size){
    QString sizeFormatted;
    if (size < 1000)
        sizeFormatted = QString::number(size) + " B";
    else if (size < 1000000)
        sizeFormatted = QString::number(size * 1.0 / 1000, 'f', 1) + " KB";
    else
        sizeFormatted = QString::number(size * 1.0 / 1000000, 'f', 1) + " MB";

    return sizeFormatted;
}

// 计算消息长度，并转换成9位长的16进制数的字符串
QString KSocket::comLen(QString msg){
    QByteArray msgByte = msg.toUtf8().data();
    return this->formatLen(msgByte.length());
}

// 将10进制数字转换成9位长的16进制数的字符串
QString KSocket::formatLen(qint64 len){
    QString lens = QString::number(len, 16);
    for(int i = lens.length(); i < 9 ; i ++){
        lens = "0" + lens;
    }
    return lens;
}
