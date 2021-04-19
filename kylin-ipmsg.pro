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
greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++11 link_pkgconfig

# 配置gsettings
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt

QMAKE_CXXFLAGS += -g

TRANSLATIONS += kylin-ipmsg_zh_CN.ts

# 统一日志输出
LIBS += -L/usr/lib/libukui-log4qt.so.1.0.0 -lukui-log4qt

TARGET = kylin-ipmsg

target.path = /usr/bin
target.source += $$TARGET
icon.path = /usr/share/pixmaps
icon.files = kylin-ipmsg.png
desktop.path = /usr/share/applications/
desktop.files = kylin-ipmsg.desktop

# gsettings
schemes.files += \
    $$PWD/data/org.ukui.log4qt.kylin-ipmsg.gschema.xml
schemes.path = /usr/share/glib-2.0/schemas/

INSTALLS += target \
    icon    \
    desktop \
    schemes \

# v10禁用窗管和自定义用户手册
#lessThan(QT_MAJOR_VERSION, 5) | lessThan(QT_MINOR_VERSION, 9) {
    # message("QT_VERSION ("$$QT_VERSION")")
#    DEFINES   += __V10__
#    INSTALLS  -= guide
    # QT      -= x11extras
    # LIBS    -= -lpthread
    # LIBS    -= -lX11
#}

# V10Pro使用自定义用户手册
greaterThan(QT_MAJOR_VERSION, 5) | greaterThan(QT_MINOR_VERSION, 9) {
    # message("QT_VERSION ("$$QT_VERSION")")
    DEFINES   += __V10Pro__
    guide.path = /usr/share/kylin-user-guide/data/guide/
    guide.files = data/guide/kylin-ipmsg/

    INSTALLS += guide
}

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
