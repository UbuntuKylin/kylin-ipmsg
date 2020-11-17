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

#include "recentlistitemmodel.h"


RecentListItemModel::RecentListItemModel() :
    QStandardItemModel(NULL)
{
    QHash<int, QByteArray> roleNames;
    roleNames[Name] = "name";
    roleNames[Value] = "value";
    roleNames[Type] = "type";
    roleNames[TypeIcon] = "typeIcon";
    roleNames[DateTime] = "dateTime";
    roleNames[Sender] = "sender";
    roleNames[Size] = "size";
    roleNames[Mac] = "mac";
    setItemRoleNames(roleNames);
}

// 添加最近聊天记录
/*
* Parameters:
*   name: user name
*   value: msg value
*   type: msg type
*   sender: msg sender
*   size: msg size
*   ip_mac: sender ip and mac address string
* Return :
*/
void RecentListItemModel::addRecent(QString name, QString value, QString type, QString sender, qint64 size, QString ip_mac)
{
    QStandardItem* it = new QStandardItem();

    // Format timestamp
    QDateTime now = QDateTime::currentDateTime();
    QString datetime = now.toString(Qt::SystemLocaleShortDate);

    // Convert size data
    QString sizeFormatted;
    if (size < 1024)
        sizeFormatted = QString::number(size) + " B";
    else if (size < 1048576)
        sizeFormatted = QString::number(size * 1.0 / 1024, 'f', 1) + " KB";
    else
        sizeFormatted = QString::number(size * 1.0 / 1048576, 'f', 1) + " MB";

    // Icon
    if (type == "text")
        it->setData("RecentText.png", RecentListItemModel::TypeIcon);
    else if (type == "file")
        it->setData("RecentFile.png", RecentListItemModel::TypeIcon);
    else
        it->setData("RecentFiles.png", RecentListItemModel::TypeIcon);

    if (type == "text")
        it->setData(name, RecentListItemModel::Name);
    else
        it->setData(name + " (" + sizeFormatted + ")", RecentListItemModel::Name);
    it->setData(value, RecentListItemModel::Value);
    it->setData(type, RecentListItemModel::Type);
    it->setData(datetime, RecentListItemModel::DateTime);
    it->setData(sender, RecentListItemModel::Sender);
    it->setData(sizeFormatted, RecentListItemModel::Size);
    it->setData(ip_mac, RecentListItemModel::Mac);
    insertRow(0, it);
}
