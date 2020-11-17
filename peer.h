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

#ifndef PEER_H
#define PEER_H

#include <QtNetwork/QHostInfo>

class Peer
{

public:
    Peer() { }

    QString username;
    QString system;
    QString platform;
    QHostAddress address;
    QString mac;
    qint16 port;

    inline Peer(QHostAddress a, QString n, qint16 p) {
        QStringList data = n.split(" ");

        if(data.size() >= 5){
            username = data.at(1);
            system = data.at(2);
            mac = data.at(3);
            platform = data.at(4);
        }

        address = a;
        port = p;
    }
};

#endif // PEER_H
