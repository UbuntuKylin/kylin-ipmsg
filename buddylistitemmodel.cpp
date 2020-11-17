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

#include "buddylistitemmodel.h"
#include "platform.h"
#include "peer.h"
#include "settings.h"

BuddyListItemModel::BuddyListItemModel() :
    QStandardItemModel(NULL), mSettings(NULL)
{
    QHash<int, QByteArray> roleNames;
    roleNames[Ip] = "ip";
    roleNames[Port] = "port";
    roleNames[Iptext] = "iptext";
    roleNames[Username] = "username";
    roleNames[System] = "system";
    roleNames[Platform] = "platform";
    roleNames[GenericAvatar] = "generic";
    roleNames[Avatar] = "avatar";
    roleNames[OsLogo] = "oslogo";
    roleNames[ShowBack] = "showback";
    roleNames[ShowMsgAnim] = "showMsgAnim";
    setItemRoleNames(roleNames);

    // 设置中心
    mSettings = new Settings(this);
}

// 增加本机信息
/*
* Parameters:
*   text1: msg text
*
* Return :
*/
void BuddyListItemModel::addMeElement(QString text1)
{
//    QString nickname = mSettings->nickname("MAC");
//    QString username = Platform::getSystemUsername() + text1;
//    if (nickname != "error" && nickname != "")
//        username = nickname;
    addBuddy("",
             0,
             Platform::getSystemUsername() + text1,
             Platform::getHostname(),
             "MAC",
             Platform::getPlatformName(),
             QUrl::fromLocalFile(Platform::getAvatarPath()));
}

// 增加IP发送信息
/*
* Parameters:
*   text1: msg1
*   text2: msg2
* Return :
*/
void BuddyListItemModel::addIpElement(QString text1, QString text2)
{
    addBuddy("IP",
             0,
             text1,
             text2,
             "MAC",
             "IP",
             QUrl(""));
}

// 增加好友
/*
* Parameters:
*   ip: ip address
*   port: port
*   username: user name
*   system: os system
*   mac: mac address
*   platform: os platform
*   avatarpath: user avatar picture path
* Return :
*/
void BuddyListItemModel::addBuddy(QString ip, qint16 port, QString username, QString system, QString mac, QString platform, QUrl avatarPath)
{
//    qDebug()<<"!! addbuddy: "<<ip<<" "<<port<<" "<<username<<" "<<system<<" "<<mac<<" "<<platform<<" "<<avatarPath;
    QStandardItem* it = NULL;
    bool add = true;

    qDebug()<<"!! addbuddy: "<<ip<<" "<<port<<" "<<username<<" "<<system<<" "<<mac<<" "<<platform<<" "<<avatarPath;

    // Check if the same IP is alreay in the buddy list
    if (mItemsMap.contains(ip + " " + mac)) {
        it = mItemsMap[ip + " " + mac];
        add = false;
    }else{
        it = new QStandardItem();

        if(ip == ""){
            it->setData(ip, BuddyListItemModel::Ip);
        }else{
            it->setData(ip + " " + mac, BuddyListItemModel::Ip);
        }
        it->setData(port, BuddyListItemModel::Port);
        it->setData(false, BuddyListItemModel::ShowBack);
        it->setData(false, BuddyListItemModel::ShowMsgAnim);

        // Set (or update) data
        // 如果有备注名，则显示备注名, show nickname
        QString nickname = mSettings->nickname(mac);
        if (nickname != "error" && nickname != "")
            username = nickname;
        it->setData(username, BuddyListItemModel::Username);
        it->setData(mac, BuddyListItemModel::Mac);

        if (ip != "IP" && ip != ""){
            it->setData(ip + "  ", BuddyListItemModel::Iptext);
            it->setData("" + mac, BuddyListItemModel::System);
        }else{
            it->setData("", BuddyListItemModel::Iptext);
            it->setData("" + system, BuddyListItemModel::System);
        }

        it->setData(platform, BuddyListItemModel::Platform);

        // 将好友头像保存在文件夹中
        // if (ip != "" && ip != "IP" && ip != "127.0.0.1") {
        //     QDir *fileDir = new QDir;

        //     QString filePath = QString(getenv("HOME")) + "/.ipmsgFiles";
        //     QString fileName = filePath + "/" + ip + ".png";

        //     if (!(fileDir->exists(filePath))) {
        //         fileDir->mkdir(filePath);
        //     }

        //     qDebug() << downloadUrl(avatarPath.toString(), fileName);

        //     // qml访问本地图片需使用绝对路径
        //     it->setData(QUrl("file://" + fileName), BuddyListItemModel::Avatar);
        // }
        // else {
        //     it->setData(avatarPath, BuddyListItemModel::Avatar);
        // }

        // 禁用获取好友头像功能，防止龙芯架构下的闪退问题
        it->setData(QUrl(""), BuddyListItemModel::Avatar);

        // Update generic avatar
        if ((platform.toLower() == "symbian") || (platform.toLower() == "android") || (platform.toLower() == "ios") || (platform.toLower() == "blackberry") || (platform.toLower() == "windowsphone"))
            it->setData("SmartphoneLogo.png", BuddyListItemModel::GenericAvatar);
        else if (platform.toLower() == "ip")
            it->setData("IpLogo.png", BuddyListItemModel::GenericAvatar);
        else
            it->setData("PcLogo.png", BuddyListItemModel::GenericAvatar);

        // Update logo
        if (platform.toLower() == "windows"){
            it->setData("WindowsLogo.png", BuddyListItemModel::OsLogo);
            it->setData("WindowsLogo.png", BuddyListItemModel::GenericAvatar);
        }
        else if (platform.toLower() == "macintosh"){
            it->setData("AppleLogo.png", BuddyListItemModel::OsLogo);
            it->setData("AppleLogo.png", BuddyListItemModel::GenericAvatar);
        }
        else if (platform.toLower() == "linux"){
            it->setData("LinuxLogo.png", BuddyListItemModel::OsLogo);
            it->setData("LinuxLogo.png", BuddyListItemModel::GenericAvatar);
        }
        else if (platform.toLower() == "symbian"){
            it->setData("SymbianLogo.png", BuddyListItemModel::OsLogo);
            it->setData("SymbianLogo.png", BuddyListItemModel::GenericAvatar);
        }
        else if (platform.toLower() == "ios"){
            it->setData("IosLogo.png", BuddyListItemModel::OsLogo);
            it->setData("IosLogo.png", BuddyListItemModel::GenericAvatar);
        }
        else if (platform.toLower() == "windowsphone"){
            it->setData("WindowsPhoneLogo.png", BuddyListItemModel::OsLogo);
            it->setData("WindowsPhoneLogo.png", BuddyListItemModel::GenericAvatar);
        }
        else if (platform.toLower() == "blackberry"){
            it->setData("BlackberryLogo.png", BuddyListItemModel::OsLogo);
            it->setData("BlackberryLogo.png", BuddyListItemModel::GenericAvatar);
        }
        else if (platform.toLower() == "android"){
            it->setData("AndroidLogo.png", BuddyListItemModel::OsLogo);
            it->setData("AndroidLogo.png", BuddyListItemModel::GenericAvatar);
        }
        else{
            it->setData("UnknownLogo.png", BuddyListItemModel::OsLogo);
            it->setData("UnknownLogo.png", BuddyListItemModel::GenericAvatar);
        }
    }
    if (add) {
        appendRow(it);
        if (ip != ""){
            mIpsMap.insert(mac, ip + " " + mac);
            mItemsMap.insert(ip + " " + mac, it);
        }
        else
            mMeItem = it;
    }
}

// 增加好友
/*
* Parameters:
*   peer: peer struct
* Return :
*/
void BuddyListItemModel::addBuddy(Peer &peer)
{
    QUrl avatarPath = QUrl("http://" + peer.address.toString() + ":" + QString::number(peer.port + 1) + "/dukto/avatar");

    addBuddy(peer.address.toString(),
             peer.port,
             peer.username,
             peer.system,
             peer.mac,
             peer.platform,
             avatarPath);
}

// 删除好友
/*
* Parameters:
*   mac: mac address
* Return :
*/
void BuddyListItemModel::removeBuddy(QString mac)
{
    if (!mIpsMap.contains(mac)) return;
    QString ip = mIpsMap.value(mac);

    QStandardItem* it = mItemsMap[ip];

    mItemsMap.remove(ip);
    this->removeRow(this->indexFromItem(it).row());
}

/*
* Summary: show single back
* Parameters:
*   idx: index
* Return :
*/
void BuddyListItemModel::showSingleBack(int idx)
{
    for (int i = 0; i < rowCount(); i++)
        itemFromIndex(index(i, 0))->setData(false, BuddyListItemModel::ShowBack);
    itemFromIndex(index(idx, 0))->setData(true, BuddyListItemModel::ShowBack);
}

// 通过IP地址查找好友名字
/*
* Parameters:
*   ip: ip address
* Return :
*/
QString BuddyListItemModel::buddyNameByIp(QString ip)
{
    if (!mItemsMap.contains(ip)) return "";
    return mItemsMap.value(ip)->data(BuddyListItemModel::Username).toString();
}

// 通过IP地址查找好友
/*
* Parameters:
*   ip: ip address
* Return :
*/
QStandardItem* BuddyListItemModel::buddyByIp(QString ip)
{
    if (!mItemsMap.contains(ip)) return NULL;
    return mItemsMap.value(ip);
}

// 通过mac查找好友
/*
* Parameters:
*   mac: mac address
* Return :
*/
QStandardItem* BuddyListItemModel::buddyByMac(QString mac){
    if (!mIpsMap.contains(mac)) return NULL;
    return mItemsMap.value(mIpsMap.value(mac));
}

/*
* Summary: first friend ip
* Return :
*/
QString BuddyListItemModel::fistBuddyIp()
{
    if (this->rowCount() < 3) return "";
    return this->index(2, 0).data(BuddyListItemModel::Ip).toString();
}

// 下载头像到本地
bool BuddyListItemModel::downloadUrl(const QString &url, const QString &fileName)
{
    QNetworkAccessManager manager;
    QNetworkRequest request;
    request.setUrl(url);
    QNetworkReply *reply = manager.get(request);

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        return false;
    }

    QFile file(fileName);
    qDebug() << "fileName  " << fileName;
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.write(reply->readAll());

    file.close();
    delete reply;
    return true;
}

/*
* Summary: update my element
* Return :
*/
void BuddyListItemModel::updateMeElement()
{
    mMeItem->setData(Platform::getSystemUsername(), BuddyListItemModel::Username);
}
