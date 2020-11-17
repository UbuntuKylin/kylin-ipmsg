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

Rectangle {
    color: "#00000000"

    function refreshSettingsColor() {
        settingsPage.refreshColor();
    }

    Rectangle {
        id: disabler
        anchors.fill: parent
        color: theme.color9
        opacity: 0
        visible: false

        MouseArea {
            anchors.fill: parent
        }
    }

    IpPage {
        id: ipPage
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.bottom: parent.bottom
        width: parent.width
        x: -50
        opacity: 0
        visible: false
        onBack: {
            console.log("IpPage fanhui")
            parent.state = ""
        }
    }

    SettingsPage {
        id: settingsPage
        width: parent.width
        height: parent.height
        x: -50
        opacity: 0
        anchors.fill: parent
        visible: false
        onBack: parent.state = ""
//        onBack: {
//            console.log("SettingsPage fanhui")
//            SettingsPage.controlSettingsPage()
//        }
    }

    states: [
        State {
            name: "ip"
            PropertyChanges {
                target: ipPage
                opacity: 1
                x: 0
            }
            PropertyChanges {
                target: disabler
                opacity: 1
                visible: true
            }
        },
        State {
            name: "progress"
            PropertyChanges {
                target: progressPage
                opacity: 1
                x: 0
            }
            PropertyChanges {
                target: disabler
                opacity: 1
                visible: true
            }
        },
        //  收起进度条界面
        State {
            name: "progress_"
            PropertyChanges {
                target: progressPage
                opacity: 0
                x: -50
            }
            PropertyChanges {
                target: disabler
                opacity: 0
                visible: false
            }
        },

        State {
            name: "settings"
            PropertyChanges {
                target: settingsPage
                opacity: 1
                x: 0
            }
        },
        State {
            name: "send"
            PropertyChanges {
                target: sendPage
                opacity: 1
                x: 0
            }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation { properties: "x,y"; easing.type: Easing.OutCubic; duration: 500 }
            NumberAnimation { properties: "opacity"; easing.type: Easing.OutCubic; duration: 500 }
        }
    ]
}
