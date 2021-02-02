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

#ifndef DUKTOWINDOW_H
#define DUKTOWINDOW_H

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QDebug>
#include <QQuickWindow>
#include <QtWidgets/QMainWindow>
#include <QCloseEvent>
#include <QRect>
#include <QScreen>

#include "qmlapplicationviewer/qmlapplicationviewer.h"
#include "daemonipcdbus.h"

class GuiBehind;

class DuktoWindow : public QmlApplicationViewer
{
    Q_OBJECT

public:
    explicit DuktoWindow(QWindow *parent = 0);
    void setGuiBehindReference(GuiBehind* ref);

protected:
    void closeEvent(QCloseEvent *event);

signals:

public slots:

    void keyPressEvent(QKeyEvent *event);

private:
    GuiBehind *mGuiBehind;
    DaemonIpcDbus *mDaemonIpcDbus;
};

#endif // DUKTOWINDOW_H
