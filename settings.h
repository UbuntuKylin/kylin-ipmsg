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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QRect>

// 错误类型
enum errorCode {
    CONN_SUCCESS,
    CONN_TIMEOUT,
    DISCONN,
    TRANSFERERR,
    TRY_RECONN,
    SKIPFILE,
    OTHER
};

class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = 0);
    QString currentPath();
    void savePath(QString path);
//    void saveWindowGeometry(QByteArray geo);
//    QByteArray windowGeometry();
    void saveWindowGeometry(QRect geo);
    QRect windowGeometry();
    void saveThemeColor(QString color);
    QString themeColor();
    void saveShowTermsOnStart(bool show);
    bool showTermsOnStart();
    QString buddyName();
    void saveBuddyName(QString name);
    void saveNickname(QString mac, QString nickname);
    QString nickname(QString mac);
    void saveIconPath(QString path);
    QString iconPath();

signals:

public slots:

private:
    QSettings mSettings;

};

#endif // SETTINGS_H
