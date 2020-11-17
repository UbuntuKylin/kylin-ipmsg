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

Flipable {
   id: contactDelegateItem
   width: parent.width
   height: buddyIp == "" ? 94 : 64

   property string buddyIp
   property alias buddyGeneric: buddyGenericImage.source
   property alias buddyAvatar: buddyAvatarImage.source
   property alias buddyOsLogo: buddyOsLogoImage.source
   property alias buddyIpText: buddyIpText.text
   property alias buddyUsername: buddyUsernameText.text
   property alias buddySystem: buddySystemText.text
   property bool buddyShowBack: false
   property bool buddyShowMsgAnim: false

   // 检测鼠标悬停
   property bool buddyShowIp: buddyMouseArea.containsMouse

   function restoreIt(){
       restoreScale.start()
       restoreOpacity.start()
   }

   MouseArea {
       id: buddyMouseArea
       anchors.fill: parent
       hoverEnabled: true
       onClicked: {
           guiBehind.showSendPage(buddyIp)
           restoreScale.start()
           restoreOpacity.start()
       }

   }

   Rectangle {
       anchors.fill: parent
       color: "#00000000"
       visible: buddyMouseArea.containsMouse;
       Rectangle {
           anchors.right: parent.right
           anchors.top: parent.top
           height: 64
           width: 5
           color: theme.color3
       }
   }

   front: Item {
       anchors.fill: parent
       id: temp
       width: contactDelegateItem.width

       Flipable {
           id: flipableAvatar
           width: 64
           height: 64

           front: Rectangle {
               anchors.fill: parent
               color: theme.color2
               Image {
                   anchors.fill: parent
                   source: "TileGradient.png"
               }
               Image {
                   id: buddyGenericImage
                   anchors.fill: parent
               }
               Image {
                   id: buddyAvatarImage
                   anchors.fill: parent
                   smooth: true
               }
           }

           back: Rectangle {
               anchors.fill: parent
               color: theme.color2
               Image {
                   anchors.fill: parent
                   source: "TileGradient.png"
               }
               Image {
                   id: buddyOsLogoImage
                   anchors.fill: parent
               }
           }

           transform: Rotation {
                id: innerRotation
                origin.x: 32
                origin.y: 32
                axis.x: 0; axis.y: 0; axis.z: 1
                angle: 0
           }

           opacity:1

           states: [
               State {
                    name: "MsgAnim"
                    when: buddyShowMsgAnim
               }
           ]

           transitions: [
                Transition {
                    from: ""
                    to: "MsgAnim"
                    SequentialAnimation {
                        NumberAnimation { target: flipableAvatar;  property: "scale"; from: 0.5; to: 1.2; duration: 500; easing.type: Easing.InQuad;}
                        NumberAnimation { target: flipableAvatar;  property: "scale"; from: 1.2; to: 0.5; duration: 500; easing.type: Easing.OutQuad;}
                        loops: Animation.Infinite
                    }
                    SequentialAnimation {
                        NumberAnimation { target: flipableAvatar;  property: "opacity"; from: 0.3; to: 1; duration: 500; easing.type: Easing.InQuad;}
                        NumberAnimation { target: flipableAvatar;  property: "opacity"; from: 1; to: 0.3; duration: 500; easing.type: Easing.OutQuad;}
                        loops: Animation.Infinite
                    }
                }
           ]
       }

       SText {
           id: buddyIpText
           anchors.top:  buddyUsernameText.bottom
           anchors.topMargin: 8
           anchors.left: buddySystemText.right
           anchors.leftMargin: 10
           wrapMode: Text.Wrap
           font.pixelSize: 14
           elide: "ElideRight"
           color: theme.color2
           // caoliang 隐藏或显示ip
           visible: buddyShowIp
       }

       SText {
           id: buddyUsernameText
           anchors.top:  flipableAvatar.top
           anchors.topMargin: 8
           anchors.left: flipableAvatar.right
           anchors.leftMargin: 10
           anchors.right: parent.right
           anchors.rightMargin: 20
           font.pixelSize: 18
           height: 21
           elide: "ElideRight"
           color: theme.color2
       }

       SText {
           id: buddySystemText
           anchors.left: flipableAvatar.right
           anchors.leftMargin: 10
           anchors.top: buddyUsernameText.bottom
           anchors.topMargin: 8
//           anchors.right: parent.right
           anchors.rightMargin: 10
           font.pixelSize: 14
           elide: "ElideRight"
           color: theme.color4
       }
   }

   transform: Rotation {
        id: rotation
        origin.x: 32
        origin.y: 32
        axis.x: 1; axis.y: 0; axis.z: 0     // set axis.y to 1 to rotate around y-axis
        angle: 0    // the default angle
   }

   ListView.onAdd: NumberAnimation { target: rotation; property: "angle"; from: -90; to: 0; duration: 300; easing.type: Easing.OutCubic }
   ListView.onRemove: SequentialAnimation {
       PropertyAction { target: contactDelegateItem; property: "ListView.delayRemove"; value: true }
       NumberAnimation { target: rotation; property: "angle"; from: 0; to: -90; duration: 300; easing.type: Easing.InCubic }
       PropertyAction { target: contactDelegateItem; property: "ListView.delayRemove"; value: false }

   }

   Rectangle {
       color: theme.color2
       width: parent.width - 80
       height: 1
       x: (parent.width - width) / 2 - 10
       y: parent.height - 10
       visible: buddyIp == ""
   }

   NumberAnimation {
       id: restoreOpacity
       target: flipableAvatar
       properties: "opacity"
       to: 1.0
       duration: 300
   }

   NumberAnimation {
       id: restoreScale
       target: flipableAvatar
       properties: "scale"
       to: 1.0
       duration: 300
   }

}
