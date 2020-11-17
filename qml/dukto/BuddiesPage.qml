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

import QtQuick 2.3

Item {
    //  根据ip_mac地址停止好友界面对应item的头像动画
    function restoreMsgAnimByIp(ip_mac){
        for(var i = 0; i < buddiesList.count; i ++){
            buddiesList.currentIndex = i;
            if(buddiesList.currentItem.buddyIp === ip_mac){
                buddiesList.currentItem.restoreIt();
            }
        }
    }

    clip: true

    ListView {
        id: buddiesList
        anchors.fill: parent
        spacing: 10
        anchors.leftMargin: 25
        anchors.rightMargin: 0
        model: buddiesListData

        delegate: BuddyListElement {
             buddyIp: ip
             buddyAvatar: avatar
             buddyGeneric: generic
             buddyIpText: iptext
             buddyUsername: username
             buddySystem: system
             buddyOsLogo: oslogo
             buddyShowBack: showback
             buddyShowMsgAnim: showMsgAnim
         }
    }
}
