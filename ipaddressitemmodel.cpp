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

#include "ipaddressitemmodel.h"


IpAddressItemModel::IpAddressItemModel() :
    QStandardItemModel(NULL)
{
    QHash<int, QByteArray> roleNames;
    roleNames[Ip] = "ip";
    setItemRoleNames(roleNames);

    refreshIpList();
}

/*
* Summary: add a ip address
* Parameters:
*   ip: ip address
* Return :
*/
void IpAddressItemModel::addIp(QString ip)
{
    QStandardItem* it = new QStandardItem();
    it->setData(ip, IpAddressItemModel::Ip);
    appendRow(it);
}

/*
* Summary: refresh ip list
* Return :
*/
void IpAddressItemModel::refreshIpList()
{
    clear();

    QList<QHostAddress> addrs = QNetworkInterface::allAddresses();
    for (int i = 0; i < addrs.length(); i++)
        if ((addrs[i].protocol() == QAbstractSocket::IPv4Protocol) && (addrs[i].toString() != "127.0.0.1")){
            addIp(addrs[i].toString());

            // 选择正确的局域网IP地址
            if (addrs[i].toString().left(3) == "192") {
                this->myIp = addrs[i].toString();
                break;
            }
            else if (addrs[i].toString().left(3) == "172") {
                this->myIp = addrs[i].toString();
                break;
            }
            else if (addrs[i].toString().left(2) == "10") {
                this->myIp = addrs[i].toString();
                break;
            }
        }
}
