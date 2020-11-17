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

//  A toy QML colorpicker control, by Ruslan Shestopalyuk
import QtQuick 2.3
import "ColorUtils.js" as ColorUtils

Rectangle {
    id: colorPicker
    property color colorValue: ColorUtils.hsba(hueSlider.value, sbPicker.saturation,
                                               sbPicker.brightness, 1)
    width: 144; height: 126
    color: "#FFFFFF"

    signal changed()

    function setColor(color) {

        var h = theme.getHue(color);
        var s = theme.getSaturation(color);
        var b = theme.getLightness(color);

        hueSlider.setValue(h);
        sbPicker.setValue(s, b);

        this.changed();
    }

    Row {
        anchors.fill: parent

//        antialiasing: true;
        spacing: 3

        // saturation/brightness picker box
        SBPicker {
            id: sbPicker
            hueColor : ColorUtils.hsba(hueSlider.value, 1.0, 1.0, 1.0, 1.0)
            width: parent.height; height: parent.height
            onChanged: {
                colorPicker.changed();
            }
        }

        // hue picking slider
        Item {
            width: 12; height: parent.height
            Rectangle {
                anchors.fill: parent
                gradient: Gradient {
                    GradientStop { position: 1.0;  color: "#FF0000" }
                    GradientStop { position: 0.85; color: "#FFFF00" }
                    GradientStop { position: 0.76; color: "#00FF00" }
                    GradientStop { position: 0.5;  color: "#00FFFF" }
                    GradientStop { position: 0.33; color: "#0000FF" }
                    GradientStop { position: 0.16; color: "#FF00FF" }
                    GradientStop { position: 0.0;  color: "#FF0000" }
                }
                border.color: "#f0f0f0"
                border.width: 2
            }
            ColorSlider {
                id: hueSlider
                anchors.fill: parent
                onChanged: colorPicker.changed()
            }
        }
    }
}
