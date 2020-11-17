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
    id: mainElement
    width: 330
    height: 600

    FontLoader {
        id: duktofont
        source: "Klill-Light.ttf"
    }

    FontLoader {
        id: duktofontsmall
        source: "LiberationSans-Regular.ttf"
    }

    FontLoader {
        id: duktofonthappy
        source: "KGLikeASkyscraper.ttf"
    }

    DuktoInner {
        id: duktoInner
        anchors.fill: parent
        onShowIpList: {
            duktoOverlay.state = "ip"
            ipPage.controlShowIpPage();
        }

        onShowSettings: {
            duktoOverlay.refreshSettingsColor();
            duktoOverlay.state = "settings";
            settingsPage.controlSettingsPage();
        }
    }

    DuktoOverlay {
        id: duktoOverlay
//        anchors.fill: parent
    }

    IpPage {
        id: ipPage
        anchors.fill: parent
    }

    SettingsPage {
        id: settingsPage
        anchors.fill: parent
    }

    Binding {
        target: guiBehind
        property: "overlayState"
        value: duktoOverlay.state
    }
}
