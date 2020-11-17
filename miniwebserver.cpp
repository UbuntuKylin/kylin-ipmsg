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

#include "miniwebserver.h"
#include "platform.h"


MiniWebServer::MiniWebServer()
{
    // 获取用户头像文件
    QString path = Platform::getAvatarPath();
    if (path != "")
    {
        QImage img(path);
        QImage scaled = img.scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        QBuffer tmp(&mAvatarData);
        tmp.open(QIODevice::WriteOnly);
        scaled.save(&tmp, "PNG");
    }
}

// 新进连接
/*
* Parameters:
*   handle: new connection handle
* Return :
*/
void MiniWebServer::incomingConnection(qintptr handle)
{
    qDebug() << "MiniWebServer::incomingConnection";
    QTcpSocket* s = new QTcpSocket(this);
    connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
    s->setSocketDescriptor(handle);
}

/*
* Summary: read clinet
* Return :
*/
void MiniWebServer::readClient()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    if (socket->canReadLine()) {
        QStringList tokens = QString(socket->readLine()).split(QRegExp("[ \r\n][ \r\n]*"));
        if (tokens[0] == "GET") {

            QTextStream os(socket);
            os.setAutoDetectUnicode(true);
            os << "HTTP/1.0 200 OK\r\n"
                "Content-Type: image/png\r\n"
                "Content-Length: " << mAvatarData.size() << "\r\n"
                "\r\n";
            os.flush();

            QDataStream ds(socket);
            ds.writeRawData(mAvatarData.data(), mAvatarData.size());

            socket->close();
            if (socket->state() == QTcpSocket::UnconnectedState)
            {
//                delete socket;
                socket->deleteLater();
                socket = NULL;
            }
        }
    }
}

/*
* Summary: discard a clinet
* Return :
*/
void MiniWebServer::discardClient()
{
    qDebug() << "discardClient";
    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->deleteLater();
}
