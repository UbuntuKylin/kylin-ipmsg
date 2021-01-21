# DUKTO - A simple, fast and multi-platform file transfer tool for LAN users
# Copyright (C) 2011 Emanuele Colombo
#               2020 KylinSoft Co., Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


include(qmlapplicationviewer/qmlapplicationviewer.pri)

QT += core gui network dbus

lessThan(QT_MAJOR_VERSION, 5) {
 QT += declarative
} else {
 QT += widgets quick qml
}

#for nullptr
greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++11

VERSION = 1.1.17

QMAKE_CXXFLAGS += -g

TRANSLATIONS = kylin-ipmsg_zh_CN.ts

TARGET = kylin-ipmsg

target.path = /usr/bin
target.source += $$TARGET
icon.path = /usr/share/pixmaps
icon.files = kylin-ipmsg.png
desktop.path = /usr/share/applications/
desktop.files = kylin-ipmsg.desktop
guide.path = /usr/share/kylin-user-guide/data/guide/
guide.files = data/guide/kylin-ipmsg/

INSTALLS += target \
    icon \
    desktop \
    guide

SOURCES += main.cpp \
    guibehind.cpp \
    platform.cpp \
    buddylistitemmodel.cpp \
    duktoprotocol.cpp \
    miniwebserver.cpp \
    ipaddressitemmodel.cpp \
    recentlistitemmodel.cpp \
    settings.cpp \
    destinationbuddy.cpp \
    duktowindow.cpp \
    theme.cpp \
    chatwidget.cpp \
    ktcpserver.cpp \
    ksocket.cpp \
    daemonipcdbus.cpp

HEADERS += \
    guibehind.h \
    platform.h \
    buddylistitemmodel.h \
    peer.h \
    miniwebserver.h \
    ipaddressitemmodel.h \
    recentlistitemmodel.h \
    settings.h \
    destinationbuddy.h \
    duktoprotocol.h \
    duktowindow.h \
    theme.h \
    chatwidget.h \
    ksocket.h \
    daemonipcdbus.h

FORMS += \
    chatwidget.ui

RESOURCES += \
    qml.qrc

include(qtsingleapplication/qtsingleapplication.pri)

lupdate_only{
    SOURCES = qml/dukto/*.qml
}
