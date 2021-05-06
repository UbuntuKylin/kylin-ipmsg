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

#ifndef KSOCKETTHREAD_H
#define KSOCKETTHREAD_H

#include <QObject>
#include <QThread>
#include <QtNetwork>
#include <QDebug>
#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>

#include "settings.h"

// 消息类型
#define C_WHOAMI "A0B1"
#define C_TEXT "A0A1"
#define C_FILE "A0A2"
#define C_DIR "A0A3"
#define C_END_ONEFILE "A0A7"
#define C_END_ONEDIR "A0A8"
#define C_END "A0A9"
#define C_CANCEL "A0FF"
#define C_CANCEL_READY "A0FG"
#define S_IAMREADY "B0B1"
#define S_ERROR "B0FF"

#define NETWORK_PORT 9696
#define UDP_NETWORK_PORT 9695
#define MSG_SIZE_MIN 4


class KSocket : public QObject
{
    Q_OBJECT
public:
    KSocket(qintptr s, QString systemSignature, QObject *parent = 0);
    KSocket(QString targetIP, QString systemSignature, QString pRemoteID, QObject *parent = 0);
    ~KSocket();

    // 本机系统标识
    QString pSystemSignature;
    // 对方标识
    QString pRemoteID;
    // 目标IP
    QString pTargetIP;
    // 连接超时次数
    int timeoutCount;
    // socket是否在线
    bool isConnected;
    // 主动连接或被动连接
    bool isInitiative;

    // 被告知的本次信息总长度
    qint64 pTotalLenToReceive;
    // 当前信息已接收的总长度
    qint64 pTotalReceivedLen;
    // 被告知的当前单条消息长度
    qint64 pCurrentMsgLen;
    // 当前消息已接收部分的长度
    qint64 pCurrentMsgReceivedLen;
    // 接收的文字
    QByteArray pReceiveText;
    // 当前接收的文件夹的根目录
    QString pRootDir;
    // 文件夹已存在，重命名后的根目录
    QString pRootDirRename;
    // 当前接收的文件
    QFile *pReceiveFile;
    // 本次接收的所有文件列表
    QStringList *pReceivedFiles;

    // 当前发送的文件
    QFile *pSendFile;
    // 根文件夹所在目录
    QString pRootPath;
    // 当前要发送的文件夹列表
    QStringList pDirsToSend;
    // 预处理文件列表
    QStringList tmpFiles;
    // 当前要发送的文件列表
    QList<QFile *> pFilesToSend;
    // 当前要发送的所有文件总长度
    qint64 pTotalLenToSend;
    // 当前已发送的所有文件总长度
    qint64 pTotalSendLen;
    // 当前发送的单个文件长度
    qint64 pCurrentFileLenToSend;
    // 当前已发送的单个文件长度
    qint64 pCurrentFileSendLen;

    // 当前读取内容类型
    enum readType {
        MSGTYPE,
        DATA
    } pReadType;
    // 当前消息类型
    QString pCurrentMsgType;

    // socket相关
    QTcpSocket *socket;
    QTcpSocket *socketSecondary;
    QTcpServer *pTcpServer;
    qintptr socketDescriptor;
    // 连接超时计时器
    QTimer *timer;

    // 上次消息类型
    QString lastMsgType;

    // 是否已重连
    bool isReConnect = false;

    // 接收文字
    void receiveText();
    // 接收文件
    void receiveFile();

private:
    // 遍历出目标文件夹下所有文件名
    void listAllFiles(QString fileName);
    // 将byte转换成KB或MB
    QString sizeHuman(qint64 size);
    // 计算消息长度，并转换成9位长的16进制数的字符串
    QString comLen(QString msg);
    // 将10进制数字转换成9位长的16进制数的字符串
    QString formatLen(qint64 len);

public slots:
    //void client_second_socket_establish();
    // 被动端开始工作
    void imReady();
    // 被动附socket连接
    //void newSecondaryConn();
    // 主动端开始工作
    void imStart();
    // 单次超时处理
    void timoutOnce();
    // 连接成功，取消超时计时器，执行初始化方法
    void imStart_();
    // 发送文字
    void sendText(QString);
    // 发送文件夹预处理
    void sendDir(QString);
    // 发送文件夹
    void sendDirs();
    // 发送文件预处理
    void sendFiles(QStringList);
    // 发送单个文件
    void sendOneFile();
    // 发送单个文件头
    void sendFileHead();
    // 发送实际数据
    void sendData();
    // 发送取消文件传输消息
    void sendCancel();
    // 断开连接时的结束操作
    void finishThread();

    // socket标准错误处理
    void socketError(QAbstractSocket::SocketError);

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
    void handleMsg();
    //void handleMsgSecondary();

signals:
    // 文字发送完成信号
    void sendTextComplete();
    void sendTextComplete_add_recentlist(QString , QString);
    // 文件发送完成信号
    void sendFileComplete();
    void sendFileComplete_add_recentlist(QStringList *files, qint64 totalSize, QString dir, QString mac);
    // 文件发送取消信号
    void sendFileCancelled();

    // 文字接收完成信号
    void receiveTextComplete(QString text, QString);
    // 文件接收完成信号
    void receiveFileComplete(QStringList *files, qint64 totalSize, QString dir, QString remoteID);
    // 文件接收取消信号
    void receiveFileCancelled();
    // 是否主动连接
    void isInitiativeConn(bool);
    // 添加上层好友
    void addUpBuddy(QString ip, QString user_name , QString system , QString mac , QString Platform);

    // 开启进度条，true:发送，false:接收
    void startTransfer(bool isSend);
    // 更新进度条状态，true:发送，false:接收
    void updateTransferStatus(int percent, QString text, bool isSend);

    // 自报姓名消息
    //void updateRemoteID(QString pRemoteID, KSocket *ks);
    void updateRemoteID(QString ip, QString user_name , QString system , QString mac , QString Platform , KSocket*);

    // 主动连接成功更新聊天框信息
    void updateCw(QString ip, QString user_name , QString system , QString mac , QString Platform);

    // 异常消息
    void transferMsgSignal(int code);

    // 断开连接信号
    void finished();
    void finished(QString);
    void finished(bool);
};

#endif // KSOCKETTHREAD_H
