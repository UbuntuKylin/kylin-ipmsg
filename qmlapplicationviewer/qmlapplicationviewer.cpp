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

#include "qmlapplicationviewer.h"

#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlContext>
#include <QtCore/QTextCodec>
#include <QPushButton>

#include <qplatformdefs.h> // MEEGO_EDITION_HARMATTAN

#ifdef HARMATTAN_BOOSTER
#include <MDeclarativeCache>
#endif

/*
* qml application viewer private param and functions
*/
class QmlApplicationViewerPrivate
{
    QmlApplicationViewerPrivate(QQuickView *view_) : view(view_) {}
    QString mainQmlFile;
    QQuickView *view;
    friend class QmlApplicationViewer;
    static QString adjustPath(const QString &path);
};

/*
* Summary: adjust the path
* Parameters:
*     path: path before adjust
* Return : path after adjust.
*/
QString QmlApplicationViewerPrivate::adjustPath(const QString &path)
{
#ifdef Q_OS_MAC
    if (!QDir::isAbsolutePath(path))
        return QString::fromLatin1("%1/../Resources/%2")
                .arg(QCoreApplication::applicationDirPath(), path);
#elif defined(Q_OS_QNX)
    if (!QDir::isAbsolutePath(path))
        return QString::fromLatin1("app/native/%1").arg(path);
#elif !defined(Q_OS_ANDROID)
    QString pathInInstallDir =
            QString::fromLatin1("%1/../%2").arg(QCoreApplication::applicationDirPath(), path);
    if (QFileInfo(pathInInstallDir).exists())
        return pathInInstallDir;
    pathInInstallDir =
            QString::fromLatin1("%1/%2").arg(QCoreApplication::applicationDirPath(), path);
    if (QFileInfo(pathInInstallDir).exists())
        return pathInInstallDir;
#endif
    return path;
}

QmlApplicationViewer::QmlApplicationViewer(QWindow *parent)
    : QQuickView(parent)
    , d(new QmlApplicationViewerPrivate(this))
{
    connect(engine(), SIGNAL(quit()),QCoreApplication::instance(), SLOT(quit()));
    setResizeMode(QQuickView::SizeRootObjectToView);
}

QmlApplicationViewer::QmlApplicationViewer(QQuickView *view, QWindow *parent)
    : QQuickView(parent)
    , d(new QmlApplicationViewerPrivate(view))
{
    connect(view->engine(), SIGNAL(quit()), QCoreApplication::instance(), SLOT(quit()));
    view->setResizeMode(QQuickView::SizeRootObjectToView);
}

QmlApplicationViewer::~QmlApplicationViewer()
{
    delete d;
}

/*
* Summary: creater.
*/
QmlApplicationViewer *QmlApplicationViewer::create()
{
    return new QmlApplicationViewer();
}

/*
* Summary: set main qml file
* Parameters:
*     file: qml file path
*/
void QmlApplicationViewer::setMainQmlFile(const QString &file)
{
//    d->mainQmlFile = QmlApplicationViewerPrivate::adjustPath(file);
    d->mainQmlFile = d->adjustPath(file);
#ifdef Q_OS_ANDROID
    setSource(QUrl(QLatin1String("assets:/")+d->mainQmlFile));
#else
    setSource(QUrl::fromLocalFile(d->mainQmlFile));
#endif
}

/*
* Summary: add import path
* Parameters:
*     file: qml file path
*/
void QmlApplicationViewer::addImportPath(const QString &path)
{
    //    engine()->addImportPath(QmlApplicationViewerPrivate::adjustPath(path));
    d->view->engine()->addImportPath(d->adjustPath(path));
}

/*
* Summary: set orientation
* Parameters:
*     orientation: target orientation
*/
void QmlApplicationViewer::setOrientation(ScreenOrientation orientation)
{
#if QT_VERSION < 0x050000
    Qt::WidgetAttribute attribute;
    switch (orientation) {
#if QT_VERSION < 0x040702
    // Qt < 4.7.2 does not yet have the Qt::WA_*Orientation attributes
    case ScreenOrientationLockPortrait:
        attribute = static_cast<Qt::WidgetAttribute>(128);
        break;
    case ScreenOrientationLockLandscape:
        attribute = static_cast<Qt::WidgetAttribute>(129);
        break;
    default:
    case ScreenOrientationAuto:
        attribute = static_cast<Qt::WidgetAttribute>(130);
        break;
#else // QT_VERSION < 0x040702
    case ScreenOrientationLockPortrait:
        attribute = Qt::WA_LockPortraitOrientation;
        break;
    case ScreenOrientationLockLandscape:
        attribute = Qt::WA_LockLandscapeOrientation;
        break;
    default:
    case ScreenOrientationAuto:
        attribute = Qt::WA_AutoOrientation;
        break;
#endif // QT_VERSION < 0x040702
    };
    setAttribute(attribute, true);
#else // QT_VERSION < 0x050000
    Q_UNUSED(orientation)
#endif // QT_VERSION < 0x050000
}

/*
* Summary: show qml widget expanded
*
*/
void QmlApplicationViewer::showExpanded()
{
//#if defined(MEEGO_EDITION_HARMATTAN) || defined(Q_WS_SIMULATOR)
#if defined(MEEGO_EDITION_HARMATTAN) || defined(Q_OS_SIMULATOR)
    showFullScreen();
#elif defined(Q_WS_MAEMO_5) || defined(Q_OS_QNX)
    showMaximized();
#else
    show();
#endif
}

/*
* Summary: create application
* Parameters:
*     argc: main argc
*     argv: main argv
*/
//QApplication *createApplication(int &argc, char **argv)
QGuiApplication *createApplication(int &argc, char **argv)
{
#ifdef HARMATTAN_BOOSTER
    return MDeclarativeCache::qApplication(argc, argv);
#else
    return new QGuiApplication(argc, argv);
#endif
}
