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

#include "theme.h"

const QString Theme::DEFAULT_THEME_COLOR = "#3a6cbc";


Theme::Theme(QObject *parent) :
    QObject(parent),
    mColor1("#000000"), mColor2(DEFAULT_THEME_COLOR), mColor3("#4cb328"),
    mColor4("#555555"), mColor5("#888888"), mColor6("#ffffff"),
    mColor7("#cccccc"), mColor8("#eeeeee"), mColor9("#ccffffff"){
}

// 设置主题
/*
* Parameters:
*   color: theme color
* Return :
*/
void Theme::setThemeColor(QString color)
{
    QColor c;
    c.setNamedColor(color);
    c.setRed(qMin(c.red() + 40, 255));
    c.setGreen(qMin(c.green() + 40, 255));
    c.setBlue(qMin(c.blue() + 40, 255));

    mColor2 = color;
    mColor3 = c.name();
    emit color2Changed();
    emit color3Changed();
}

// 具体颜色操作
/*
* Parameters:
*   color: theme color
* Return :
*/
float Theme::getHue(QString color) {

    QColor c;
    c.setNamedColor(color);
    QColor converted = c.toHsv();
    return converted.hsvHueF();
}

/*
* Summary: get saturation
* Parameters:
*   color: color
* Return : color saturation
*/
float Theme::getSaturation(QString color) {

    QColor c;
    c.setNamedColor(color);
    QColor converted = c.toHsv();
    return converted.hsvSaturationF();
}

/*
* Summary: get lightness
* Parameters:
*   color: color
* Return :  lightness value
*/
float Theme::getLightness(QString color) {

    QColor c;
    c.setNamedColor(color);
    QColor converted = c.toHsv();
    return converted.valueF();
}
