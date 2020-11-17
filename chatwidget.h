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

#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QDesktopWidget>
#include <QDialog>
#include <QMouseEvent>
#include <QDateTime>
#include <QDesktopServices>
#include <QClipboard>
#include <QFileDialog>
#include <QUrl>
#include <QRegExp>
#include <QTimer>
#include <QIcon>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QDebug>
#include <QObject>
#include <QNetworkInterface>
#include <QHostAddress>
#include "buddylistitemmodel.h"
#include "destinationbuddy.h"
#include "settings.h"
#include "theme.h"
#include "ipaddressitemmodel.h"
#include "daemonipcdbus.h"

#define MAXINPUTLEN 800

class Settings;

namespace Ui {
class ChatWidget;
}

class ChatWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ChatWidget(QDialog *parent = 0);
    ~ChatWidget();

    void firstStyle();
    void showme();
    void disableInput();
    void focusOut();
    void focusIn();

    inline Settings* settings() { return mSettings; }

    static const QString SPACE_STR;
    static const QString SPACE_STR_HTML;
    Theme * theme;
    DestinationBuddy * dbuddy;
    QString ip;
    QString myIp;
    QTimer *alertTimer;
    QHash<QString, QStandardItem*> * buddies;

public slots:
    void addTextLog(QString text);
    void addLogDirect(QString text);
    void onSendCompleted();
    void enableInput();
    void startTransfer(bool isSend);
    void updateTransferStatus(int percent, QString text, bool isSend);
    void stopTransfer(bool isSend);
    void slotTransferMsg(int code);
    void setOnLine(bool isOnLine);
    void recvCancel();
    void getIsInitiativeConn(bool);

    // 键盘F1响应用户手册
    void keyPressEvent(QKeyEvent *event);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    bool eventFilter(QObject *, QEvent *);

signals:
    void sendMsg(QString msg);
    void sendFiles(QStringList files);
    void sendDir(QString dirname);
    void sendFileCanceled();
    void reSaveCw(QString ip);
    void reBindSocket(ChatWidget *);

private slots:
    void on_pb_sendmsg_clicked();
    void on_pb_sendfile_clicked();
    void on_pb_senddir_clicked();
    void on_pb_titleicon_clicked();

    void leTextChanged(QString);
    void teTextChanged();
    void openUrl(QUrl url);
    void msgAlert();

    void on_pb_w_cancel_clicked();

    // caoliang
    void on_pb_addname_clicked();
    void on_pb_checkname_clicked();

private:
    Ui::ChatWidget *ui;

    enum sendType_ {
        Text,
        Files,
        Dir,
        Idle
    };
    int sendType;

    bool isOnLine;

    QString textToSend;
    QString dirToSend;
    QStringList fileToSend;

    QPoint mMovePosition;
    bool mMoveing;
    int timeFlag;
    QIcon iconDukto;
    QIcon iconBlank;
    bool isFocus;

    bool isIpExists();
    bool findFile(QString path);

    Settings *mSettings;
    IpAddressItemModel mIpAddresses;
    DaemonIpcDbus *mDaemonIpcDbus;
};

#endif // CHATWIDGET_H
