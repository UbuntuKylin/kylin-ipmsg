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

    Rectangle {
        x: 27
        y: 15
        width: 64
        height: 64
        Image {
            source: "TileGradient.png"
            anchors.fill: parent
        }
        Image {
//            source: "file:///usr/share/icons/ukui-icon-theme-default/64x64/apps/kylin-ipmsg.png"
            source: guiBehind.iconPath
            anchors.fill: parent
        }
    }

    SmoothText {
        x: 22
        y: 85
        font.pixelSize: 72
        text: qsTr("Kylin Ipmsg") + " 1.1.15"
        color: theme.color4
    }

    SmoothText {
        x: 25
        y: 135
        font.pixelSize: 32
        text: "<a href='http://www.kylinos.cn'>http://www.kylinos.cn</a>"
        onLinkActivated: Qt.openUrlExternally(link)
        color: theme.color5
    }

    SText {
        anchors.right: parent.right
        anchors.rightMargin: 40
        anchors.left: parent.left
        anchors.leftMargin: 25
        y: 180
        font.pixelSize: 14
        color: theme.color5
        wrapMode: "WordWrap"
        text: qsTr("Provide text chat and file transfer function with no server. ") + "\n" +
              qsTr("Mult person concurrency. ") + "\n\n" +
              qsTr("The UI resources comes from Dukto")
        onLinkActivated: Qt.openUrlExternally(link)
    }
}
