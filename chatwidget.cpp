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

#include "chatwidget.h"
#include "ui_chatwidget.h"
#include "settings.h"
#include "ipaddressitemmodel.h"
#include "buddylistitemmodel.h"

/*
* Summary: chat widget logic
*/
ChatWidget::ChatWidget(QDialog *parent) : QDialog(parent), ui(new Ui::ChatWidget), mSettings(NULL){
    ui->setupUi(this);

    // 设置中心
    mSettings = new Settings(this);
    // 响应用户手册
    mDaemonIpcDbus = new DaemonIpcDbus();

    // 设置“添加备注”按钮大小
    QString locale = QLocale::system().name();
    if(locale == "zh_CN"){
        ui->pb_addname->setFixedWidth(90);
        ui->pb_checkname->setFixedWidth(90);
        ui->pb_sendmsg->setFixedWidth(120);
    }

//    this->setWindowIcon(QIcon("/usr/share/pixmaps/kylin-ipmsg.png"));
    QString iconPath = mSettings->iconPath();
    if (iconPath == "") {
        iconPath = "/usr/share/pixmaps/kylin-ipmsg.png";
    }
//    setWindowIcon(QIcon(iconPath));
    // setWindowIcon(QIcon::fromTheme("kylin-ipmsg"));

    this->setMaximumSize(QSize(450,480));
    this->setMinimumSize(QSize(450,480));
    this->setWindowFlags(Qt::FramelessWindowHint);

    this->ui->te_chatlog->setReadOnly(true);
    this->ui->pb_titleicon->setFocusPolicy(Qt::NoFocus);
    this->ui->pb_sendmsg->setFocusPolicy(Qt::NoFocus);
    this->ui->pb_sendfile->setFocusPolicy(Qt::NoFocus);
    this->ui->pb_senddir->setFocusPolicy(Qt::NoFocus);
    this->ui->pb_w_cancel->setFocusPolicy(Qt::NoFocus);
    this->ui->pbar_transfer->setFocusPolicy(Qt::NoFocus);
    this->ui->pbar_transfer_recv->setFocusPolicy(Qt::NoFocus);

    this->ui->pb_sendmsg->setEnabled(false);

    this->ui->te_chatlog->setOpenExternalLinks(false);
    this->ui->te_chatlog->setOpenLinks(false);

    this->connect(this->ui->le_ip, SIGNAL(textChanged(QString)), this, SLOT(leTextChanged(QString)));
    this->connect(this->ui->te_chat, SIGNAL(textChanged()), this, SLOT(teTextChanged()));
    this->connect(this->ui->te_chatlog, SIGNAL(anchorClicked(QUrl)), this, SLOT(openUrl(QUrl)));

    // 给添加备注和确认修改按钮注册事件
    this->connect(this->ui->pb_addname,SIGNAL(clicked()),this,SLOT(on_pb_addname_clicked()),Qt::UniqueConnection);
    this->connect(this->ui->pb_checkname,SIGNAL(clicked()),this,SLOT(on_pb_checkname_clicked()),Qt::UniqueConnection);

    this->ui->te_chat->installEventFilter(this);
    this->ui->te_chatlog->installEventFilter(this);
    this->ui->le_ip->installEventFilter(this);

    this->timeFlag = 0;
    this->alertTimer = new QTimer(this);
    this->alertTimer->setInterval(300);
    this->connect(this->alertTimer, SIGNAL(timeout()), this, SLOT(msgAlert()));

//    this->iconDukto = QIcon("/usr/share/pixmaps/kylin-ipmsg.png");
    this->iconDukto = QIcon::fromTheme("kylin-ipmsg");
    this->iconBlank = QIcon(":/qml/dukto/Blank.png");

    this->dbuddy = new DestinationBuddy(this);

    this->ui->lb_bottomshadow->hide();
    this->ui->pbar_transfer->setTextVisible(false);
    this->ui->pbar_transfer->setRange(0, 100);
    this->ui->pbar_transfer->reset();
    this->ui->w_progress->hide();

    this->ui->pbar_transfer_recv->setTextVisible(false);
    this->ui->pbar_transfer_recv->setRange(0, 100);
    this->ui->pbar_transfer_recv->reset();
    this->ui->w_progress_recv->hide();

    this->ui->w_alert->hide();

    this->isOnLine = false;
    this->sendType = ChatWidget::Idle;
}

ChatWidget::~ChatWidget(){
    delete ui;
}

// 事件过滤器
/*
* Summary: filter all event
* Parameters:
*   o: object
*   e: event
* Return :
*/
bool ChatWidget::eventFilter(QObject *o, QEvent *e){
    if(e->type() == QEvent::FocusIn){
        this->focusIn();
    }else if(e->type() == QEvent::FocusOut){
        this->focusOut();

    }

    else if(e->type() == QEvent::KeyPress){
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        // ctrl+enter换行
        if((ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) && (ke->modifiers() & Qt::ControlModifier)){
            this->ui->te_chat->insertPlainText("\n");
            return true;
        // enter发送消息
        }else if(ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter){
            if(this->ui->pb_sendmsg->isEnabled() == true && this->ui->pb_sendmsg->isHidden() == false){
                this->on_pb_sendmsg_clicked();
            }
            return true;
        }
    }
    return false;
}

/*
* Summary: focus in event
* Parameters:
*
* Return :
*/
void ChatWidget::focusIn(){
    this->isFocus = true;
    this->alertTimer->stop();
    // this->setWindowIcon(this->iconDukto);
    this->ui->noFocusDiv->hide();
}

/*
* Summary: focus out event
* Parameters:
*
* Return :
*/
void ChatWidget::focusOut(){
    this->isFocus = false;
    this->ui->noFocusDiv->show();
}

// 初始化界面布局和类型等
/*
*
* Return :
*/
void ChatWidget::firstStyle(){
    this->ip = this->dbuddy->ip();
    this->ui->le_nickname->hide();      // 初始化隐藏le_nickname输入框
    this->ui->pb_checkname->hide();     // 初始化隐藏按钮
    // 输入IP的对话窗口
    if(this->dbuddy->ip() == "IP MAC"){
        this->ui->lb_name->setText(tr("The Remote IP Addr"));
        //
        this->ui->lb_name->setFixedWidth(180);

        this->ui->lb_machine->hide();
        this->ui->le_ip->setEnabled(true);
        this->ui->le_ip->show();
        this->ui->le_ip->setFocus();

        this->ui->pb_addname->hide();   //在输入IP的对话窗口隐藏修改备注按钮

        this->ui->pb_sendmsg->hide();
        this->ui->pb_sendfile->hide();
        this->ui->pb_senddir->hide();
        this->ui->te_chat->setEnabled(false);

        this->dbuddy->mUsername = "? ? ?";
        this->dbuddy->mSystem = "? ? ? ?";
        this->dbuddy->mPlatform = "unknown";
        this->dbuddy->mAvatar = "";
    }else{
        this->ui->lb_name->setText(this->dbuddy->username());
        this->ui->lb_machine->setText(this->dbuddy->system());
        this->ui->lb_machine->show();
        this->ui->le_ip->setEnabled(false);
        this->ui->le_ip->hide();

        if(this->ui->lb_name->text() == "? ? ?"){
            this->ui->lb_name->setText(this->ip.split(" ")[0]);
        }
    }

    this->setWindowTitle(this->ui->lb_name->text());
//    this->setWindowTitle("麒麟传书" + this->ip);
    this->ui->pb_titleicon->setStyleSheet("QPushButton{border: 0px solid; background-image: url(':/qml/dukto/BackIconDark.png');}");
    this->ui->lb_head->setStyleSheet("QLabel{background-image: url(':/qml/dukto/" + dbuddy->osLogo() + "');}");
    this->ui->lb_name->setStyleSheet("QLabel{color: " + this->theme->color4() + ";font-size:18px;}");
    this->ui->lb_machine->setStyleSheet("QLabel{color: " + this->theme->color4() + ";font-size:18px;}");
    this->ui->noFocusDiv->setStyleSheet("QLabel{background-color:rgba(120,120,120,20%);}");
    this->ui->lb_topshadow->setStyleSheet("QLabel{background-image: url(':/qml/dukto/BottomShadow.png');}");
    this->ui->lb_bottomshadow->setStyleSheet("QLabel{background-image: url(':/qml/dukto/TopShadow.png');}");
    this->ui->lb_bottomshadow_recv->setStyleSheet("QLabel{background-image: url(':/qml/dukto/TopShadow.png');}");
    
    // 设置聊天框滚动条样式，不跟随主题颜色切换
    this->ui->te_chatlog->setStyleSheet("QTextBrowser{border: " + this->theme->color3() + ";color: " + this->theme->color4() + ";font-size:18px;background-color: white;}"
                                        "QScrollBar::vertical{width:8px;background:rgba(0, 0, 0, 0%);border:0px solid grep;margin:0px;padding-top:5px;padding-bottom:5px;}"
                                        "QScrollBar::handle:vertical{width:8px;background:rgba(0, 0, 0, 25%);border-radius:4px;min-height:30px;}"
                                        "QScrollBar::handle:vertical:hover{width:8px;background:rgba(0, 0, 0, 50%);border-radius:4px;min-height:30px;}"
                                        "QScrollBar::add-line:vertical{width:0px;height:0px;}"
                                        "QScrollBar::sub-line:vertical{width:0px;height:0px;}"
                                        );
    this->ui->te_chat->setStyleSheet("QTextEdit{border: 1px solid " + this->theme->color3() + ";color: " + this->theme->color4() + ";font-size:18px;background-color: white;}"
                                     "QScrollBar::vertical{width:8px;background:rgba(0, 0, 0, 0%);border:0px solid grep;margin:0px;padding-top:5px;padding-bottom:5px;}"
                                     "QScrollBar::handle:vertical{width:8px;background:rgba(0, 0, 0, 25%);border-radius:4px;min-height:30px;}"
                                     "QScrollBar::handle:vertical:hover{width:8px;background:rgba(0, 0, 0, 50%);border-radius:4px;min-height:30px;}"
                                     "QScrollBar::add-line:vertical{width:0px;height:0px;}"
                                     "QScrollBar::sub-line:vertical{width:0px;height:0px;}"
                                    );
    
    // 设置不绘制滚动条槽的背景色
    this->ui->te_chatlog->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    this->ui->te_chat->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
}

// 根据主题显示现有窗口
void ChatWidget::showme(){
    this->setStyleSheet("QDialog{border:2px solid " + this->theme->color3() + "; background-color: white;}");
    this->ui->lb_title->setStyleSheet("QLabel{color: " + this->theme->color2() + ";font-size:24px;}");
    this->ui->le_ip->setStyleSheet("QLineEdit{color: " + this->theme->color4() + "; border: 1px solid " + this->theme->color3() + ";font-size:18px;background-color: white;}");

    // 设置聊天框滚动条样式，不跟随主题颜色切换
    this->ui->te_chatlog->setStyleSheet("QTextBrowser{border: " + this->theme->color3() + ";color: " + this->theme->color4() + ";font-size:18px;background-color: white;}"
                                        "QScrollBar::vertical{width:8px;background:rgba(0, 0, 0, 0%);border:0px solid grep;margin:0px;padding-top:5px;padding-bottom:5px;}"
                                        "QScrollBar::handle:vertical{width:8px;background:rgba(0, 0, 0, 25%);border-radius:4px;min-height:30px;}"
                                        "QScrollBar::handle:vertical:hover{width:8px;background:rgba(0, 0, 0, 50%);border-radius:4px;min-height:30px;}"
                                        "QScrollBar::add-line:vertical{width:0px;height:0px;}"
                                        "QScrollBar::sub-line:vertical{width:0px;height:0px;}"
                                        );
    this->ui->te_chat->setStyleSheet("QTextEdit{border: 1px solid " + this->theme->color3() + ";color: " + this->theme->color4() + ";font-size:18px;background-color: white;}"
                                     "QScrollBar::vertical{width:8px;background:rgba(0, 0, 0, 0%);border:0px solid grep;margin:0px;padding-top:5px;padding-bottom:5px;}"
                                     "QScrollBar::handle:vertical{width:8px;background:rgba(0, 0, 0, 25%);border-radius:4px;min-height:30px;}"
                                     "QScrollBar::handle:vertical:hover{width:8px;background:rgba(0, 0, 0, 50%);border-radius:4px;min-height:30px;}"
                                     "QScrollBar::add-line:vertical{width:0px;height:0px;}"
                                     "QScrollBar::sub-line:vertical{width:0px;height:0px;}"
                                    );
    this->ui->pb_sendmsg->setStyleSheet( "QPushButton{border: 2px solid " + this->theme->color2() + "; color: "+this->theme->color2()+";font-size:16px;}");
    this->ui->pb_sendfile->setStyleSheet("QPushButton{border: 2px solid " + this->theme->color2() + "; color: "+this->theme->color2()+";font-size:16px;}");
    this->ui->pb_senddir->setStyleSheet( "QPushButton{border: 2px solid " + this->theme->color2() + "; color: "+this->theme->color2()+";font-size:16px;}");
    this->ui->lb_head_bg->setStyleSheet( "QLabel{background-color: " + this->theme->color3() + "}");

    this->ui->lb_w_title->setStyleSheet("QLabel{color: " + this->theme->color6() + ";font-size:18px;}");
    this->ui->lb_w_text->setStyleSheet( "QLabel{color: " + this->theme->color6() + ";font-size:18px;}");
    this->ui->pb_w_cancel->setStyleSheet("QPushButton{border: 2px solid " + this->theme->color6() + "; color: " + this->theme->color6() + ";font-size:16px;}");
    this->ui->w_p_center->setStyleSheet("QWidget#w_p_center{background-color: " + this->theme->color2() + ";}");
    this->ui->pbar_transfer->setStyleSheet("QProgressBar{background-color: " + this->theme->color3() + "; border: 0px; border-radius: 0px;}"
                                           "QProgressBar:chunk{background-color: " + this->theme->color6() + ";}");
    this->ui->pbar_chunk->setStyleSheet("QLabel{background-color: " + this->theme->color6() + ";}");

    this->ui->lb_w_title_recv->setStyleSheet("QLabel{color: " + this->theme->color6() + ";font-size:18px;}");
    this->ui->lb_w_text_recv->setStyleSheet( "QLabel{color: " + this->theme->color6() + ";font-size:18px;}");
    this->ui->w_p_center_recv->setStyleSheet("QWidget#w_p_center_recv{background-color: " + this->theme->color2() + ";}");
    this->ui->pbar_transfer_recv->setStyleSheet("QProgressBar{background-color: " + this->theme->color3() + "; border: 0px; border-radius: 0px;}"
                                                "QProgressBar:chunk{background-color: " + this->theme->color6() + ";}");
    this->ui->pbar_chunk_recv->setStyleSheet("QLabel{background-color: " + this->theme->color6() + ";}");

    this->ui->w_alert->setStyleSheet("QWidget#w_alert{background-color: " + this->theme->color2() + ";}");
    this->ui->lb_alert->setStyleSheet("QLabel{color: " + this->theme->color6() + ";font-size:18px;}");

    // coaliang
    this->ui->pb_addname->setStyleSheet("QPushButton{border: 2px solid " + this->theme->color2() + "; color: "+this->theme->color2()+";font-size:16px;}");
    this->ui->le_nickname->setStyleSheet("QLineEdit{color: " + this->theme->color4() + "; border: 1px solid " + this->theme->color3() + ";font-size:18px;background-color: white;}");
    this->ui->pb_checkname->setStyleSheet("QPushButton{border: 2px solid " + this->theme->color2() + "; color: "+this->theme->color2()+";font-size:16px;}");

//    this->show();
}

// 不处理格式，直接添加传入的内容到聊天记录栏
/*
* Parameters:
*   text: set text msg into chat widget
* Return :
*/
void ChatWidget::addLogDirect(QString text){
    this->ui->te_chatlog->append(text);

    if(this->isFocus == false){
        this->alertTimer->start();
    }
}

// 处理格式，添加一条文字记录到聊天记录栏
/*
* Parameters:
*   text: text msg
* Return :
*/
void ChatWidget::addTextLog(QString text){
    QDateTime time = QDateTime::currentDateTime();
    QString alog = "<font color=" + Theme::DEFAULT_THEME_COLOR + ">";
    alog += this->dbuddy->username();
    alog += " (";
    alog += time.toString("hh:mm:ss");
    alog += ")</font>";
    this->ui->te_chatlog->append(alog);
    this->ui->te_chatlog->append("<font>" + text + "</font>");

    if(this->isFocus == false){
        this->alertTimer->start();
    }
}

// 一次发送操作完成
void ChatWidget::onSendCompleted(){
    if(this->sendType != ChatWidget::Idle){
        //  发送成功代表输入的 IP 地址有效，锁定 IP 输入框，固化本聊天窗口
        if(this->ui->le_ip->isEnabled() == true){
            this->ui->le_ip->setEnabled(false);
            this->ui->le_ip->hide();
//            this->ui->lb_name->setText("对方的 IP Mac");
            // this->ui->lb_name->setText(tr("Show Remote IP Addr"));
            this->ui->lb_machine->setText(this->ip.split(" ")[0]);
            this->ui->lb_machine->show();
            // this->setWindowTitle(tr("Kylin Ipmsg") + " " + this->ip.split(" ")[0]);
//            this->setWindowTitle("麒麟传书" + this->ip);

            qDebug() << "this->ip" << this->ip;
            qDebug() << "this->dbuddy->ip" << this->dbuddy->ip();

            // emit reSaveCw(this->ip);
        }

        QDateTime time = QDateTime::currentDateTime();
        QString alog = "<font color=green>";
        alog += tr("Me");
//        alog += "我";
        alog += " (";
        alog += time.toString("hh:mm:ss");
        alog += ")</font>";
        this->ui->te_chatlog->append(alog);

        if(this->sendType == ChatWidget::Text){
            this->ui->te_chatlog->append("<font>" + this->ui->te_chat->toPlainText() + "</font>");
            this->ui->te_chat->clear();
        }
        if(this->sendType == ChatWidget::Files){
            this->ui->te_chatlog->append("<font color=green>" + tr("file sent out") + "</font>");
//            this->ui->te_chatlog->append("<font color=green>" + "发送了文件" + "</font>");
            this->stopTransfer(true);
        }
        if(this->sendType == ChatWidget::Dir){
            this->ui->te_chatlog->append("<font color=green>" + tr("dir sent out") + "</font>");
//            this->ui->te_chatlog->append("<font color=green>" + "发送了文件夹" + "</font>");
            this->stopTransfer(true);
        }

        this->enableInput();
        this->ui->lb_alert->setText("");
        this->ui->w_alert->hide();

        this->sendType = ChatWidget::Idle;


    }
}

// IP输入框格式判断
/*
* Parameters:
*   text: change text
* Return :
*/
void ChatWidget::leTextChanged(QString text){
    this->ui->lb_alert->setText("");
    this->ui->w_alert->hide();
    //  IPv4地址格式正则匹配符
    QRegExp rx("((?:(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)\\.){3}(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d))");
    text = this->ui->le_ip->text().trimmed();
    bool match = rx.exactMatch(text);

    if(match == true){
        bool isExist = false;
        for (int i = 0; i < this->buddies->keys().size(); i ++){
            QStandardItem *item = this->buddies->value(this->buddies->keys().at(i));
            if(text == item->data(BuddyListItemModel::Iptext).toString().trimmed()){
                isExist = true;
            }
        }

        if(isExist == false){
            if(text == this->myIp){
                this->ui->pb_sendmsg->hide();
                this->ui->pb_sendfile->hide();
                this->ui->pb_senddir->hide();
                this->ui->te_chat->setEnabled(false);
                this->ui->lb_alert->setText(tr("This is your Ip Addr"));
//                this->ui->lb_alert->setText("这是你自己的IP地址");
                this->ui->w_alert->show();
            }else{
                this->ui->pb_sendmsg->show();
                this->ui->pb_sendfile->show();
                this->ui->pb_senddir->show();
                this->ui->te_chat->setEnabled(true);

                // 主动连接情况下，目标IP只可能对应一台机器，所以mac设置为和IP相同
                // 被动连接则不同，一个来源IP可能对应一个下级网络的多台机器
                this->dbuddy->mIp = text + " " + text;
                this->ip = text + " " + text;
            }

        // 已有该 IP 好友
        }else{
            this->ui->pb_sendmsg->hide();
            this->ui->pb_sendfile->hide();
            this->ui->pb_senddir->hide();
            this->ui->te_chat->setEnabled(false);
            this->ui->lb_alert->setText(tr("IP already exist in buddies"));
//            this->ui->lb_alert->setText("已有该IP好友，请点击好友打开聊天界面");
            this->ui->w_alert->show();
        }

    }else{
        this->ui->pb_sendmsg->hide();
        this->ui->pb_sendfile->hide();
        this->ui->pb_senddir->hide();
        this->ui->te_chat->setEnabled(false);
        this->ui->lb_alert->setText(tr("Illegal IP address"));
//        this->ui->lb_alert->setText("输入的IP地址不合法");
        this->ui->w_alert->show();
    }
}

// 输入 IP 模式时判断好友是否存在
bool ChatWidget::isIpExists(){
    if(this->ui->le_ip->isEnabled() == true){
        QString text = this->ui->le_ip->text();
        bool isExist = false;

        for (int i = 0; i < this->buddies->keys().size(); i ++){
            QStandardItem *item = this->buddies->value(this->buddies->keys().at(i));
            if(text == item->data(BuddyListItemModel::Iptext).toString().trimmed()){
                isExist = true;
            }
        }

        // 已有该 IP 好友
        if(isExist == true){
            this->ui->pb_sendmsg->hide();
            this->ui->pb_sendfile->hide();
            this->ui->pb_senddir->hide();
            this->ui->te_chat->setEnabled(false);
            this->ui->lb_alert->setText(tr("IP already exist in buddies"));
//            this->ui->lb_alert->setText("已有该IP好友，请点击好友打开聊天界面");
            this->ui->w_alert->show();

            return true;

        // 没有该 IP 好友
        }else{
            return false;
        }

    // 不是 IP 模式
    }else{
        return false;
    }
}

/*
* Summary: text changed event
* Return :
*/
void ChatWidget::teTextChanged(){
    // 空消息判断
    QString text = this->ui->te_chat->toPlainText().trimmed();
    if(text == ""){
        this->ui->pb_sendmsg->setEnabled(false);
    }else{
        if(this->ui->pb_sendmsg->isHidden() == true){
            this->ui->pb_sendmsg->show();
        }
        this->ui->pb_sendmsg->setEnabled(true);
    }

    // 限制单次最大发送字数
    QString strText = this->ui->te_chat->toPlainText();
    int length = strText.count();
    if(length > MAXINPUTLEN){
        int position = this->ui->te_chat->textCursor().position();
        strText.remove(position - (length - MAXINPUTLEN), length - MAXINPUTLEN);
        this->ui->te_chat->setText(strText);
        QTextCursor cursor = this->ui->te_chat->textCursor();
        cursor.setPosition(position - (length - MAXINPUTLEN));
        this->ui->te_chat->setTextCursor(cursor);
    }
}

/*
* Summary: enable input event
* Return :
*/
void ChatWidget::enableInput(){
    if(this->sendType != ChatWidget::Idle){
        if(this->ui->te_chat->toPlainText().trimmed() != ""){
            this->ui->pb_sendmsg->setEnabled(true);
        }
        this->ui->pb_sendfile->setEnabled(true);
        this->ui->pb_senddir->setEnabled(true);
        this->ui->te_chat->setEnabled(true);
        this->ui->te_chat->setFocus();
        this->ui->pb_addname->setEnabled(true);

        this->sendType = ChatWidget::Idle;
    }
}

/*
* Summary: disable input event
* Return :
*/
void ChatWidget::disableInput(){
    this->ui->te_chat->setEnabled(false);
    this->ui->pb_sendmsg->setEnabled(false);
    this->ui->pb_sendfile->setEnabled(false);
    this->ui->pb_senddir->setEnabled(false);
    this->ui->pb_addname->setEnabled(false);
}

// 开始数据传输，true为发送，false为接收
void ChatWidget::startTransfer(bool isSend){
    if(isSend == true){
        this->ui->w_progress->show();
        this->disableInput();
    }else{
        this->ui->w_progress_recv->show();
        this->ui->pb_addname->hide();
    }
}

// 传输状态更新，true为发送，false为接收
void ChatWidget::updateTransferStatus(int percent, QString text, bool isSend){
    if(percent != 0){
        int len = 412 * percent / 100;

        if(isSend == true){
            this->ui->pbar_chunk->resize(len, 45);
            this->ui->lb_w_text->setText(text);
        }
        else{
            this->ui->pbar_chunk_recv->resize(len, 45);
            this->ui->lb_w_text_recv->setText(text);
        }
    }
}

// 结束数据传输，true为发送，false为接收
void ChatWidget::stopTransfer(bool isSend){
    if(isSend == true){
        this->ui->pbar_chunk->resize(0, 0);
        this->ui->lb_w_text->setText("");
        this->ui->w_progress->hide();
        this->enableInput();
    }
    else{
        this->ui->pbar_chunk_recv->resize(0, 0);
        this->ui->lb_w_text_recv->setText("");
        this->ui->w_progress_recv->hide();
        this->ui->pb_addname->show();
    }
}

// 取消接收数据
void ChatWidget::recvCancel() {
    if (this->ui->w_progress_recv->isHidden() == false) { 
        this->stopTransfer(false);   
        this->addTextLog(tr("Remoter has stoped the transfer"));
        // this->addTextLog("对方中止了文件传输");
    }
}

// 点击发送文字
void ChatWidget::on_pb_sendmsg_clicked(){
    if(this->isIpExists() == false){
        this->sendType = ChatWidget::Text;

        this->disableInput();

        textToSend = this->ui->te_chat->toPlainText();

        if(this->isOnLine == true){
            emit sendMsg(textToSend);

        // 断线重连
        }else{
            this->slotTransferMsg(TRY_RECONN);
            emit reBindSocket(this);
        }
    }
    else{
        this->focusIn();
    }
}

// 点击发送文件
void ChatWidget::on_pb_sendfile_clicked(){
    if(this->isIpExists() == false){
        this->sendType = ChatWidget::Files;

        fileToSend = QFileDialog::getOpenFileNames(nullptr, tr("pls choose files to send"));
//        fileToSend = QFileDialog::getOpenFileNames(this, "请选择要发送的文件");

        if (fileToSend.count() != 0) {
            this->disableInput();

            if(this->isOnLine == true){
                emit sendFiles(fileToSend);

            // 断线重连
            }else{
                this->slotTransferMsg(TRY_RECONN);
                emit reBindSocket(this);
            }
        }
    }
    else{
        this->focusIn();
    }
}

// 点击发送文件夹
void ChatWidget::on_pb_senddir_clicked(){
    if(this->isIpExists() == false){
        this->sendType = ChatWidget::Dir;

        dirToSend = QFileDialog::getExistingDirectory(nullptr, tr("pls choose dir to send"), ".", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
//        dirToSend = QFileDialog::getExistingDirectory(this, "请选择要发送的文件夹", ".", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if (dirToSend != "") {
            bool rtn = this->findFile(dirToSend);
            if(rtn){
                this->disableInput();
                if(this->isOnLine == true){
                    emit sendDir(dirToSend);

                // 断线重连
                }else{
                    this->slotTransferMsg(TRY_RECONN);
                    emit reBindSocket(this);
                }
            }else{
                this->ui->te_chatlog->append("<font color=green>" + tr("pls do not send empty dir") + "</font>");
    //            this->ui->te_chatlog->append("<font color=green>" + "请勿发送空文件夹" + "</font>");
            }
        }
    }
    else{
        this->focusIn();
    }
}

// 递归判断文件夹里是否有文件
bool ChatWidget::findFile(QString path){
    QDir dir(path);
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::DirsFirst);
    QFileInfoList list = dir.entryInfoList();

    if(list.size() == 0){
        return false;
    }

    int i=0;
    do{
        QFileInfo fileInfo = list.at(i);
        if(fileInfo.fileName()=="." | fileInfo.fileName()==".."){
            i++;
            continue;
        }
        bool bisDir=fileInfo.isDir();
        if(bisDir){
            return findFile(fileInfo.filePath());
        }else{
            return true;
        }
        i++;

    }while(i<list.size());

    return true;
}

// 点击取消文件传输
void ChatWidget::on_pb_w_cancel_clicked(){
    emit sendFileCanceled();
    this->stopTransfer(true);

    QDateTime time = QDateTime::currentDateTime();
    QString alog = "<font color=green>";
    alog += tr("Me");
//    alog += "我";
    alog += " (";
    alog += time.toString("hh:mm:ss");
    alog += ")</font>";
    this->ui->te_chatlog->append(alog);
    this->ui->te_chatlog->append(tr("transmission canceled"));
//    this->ui->te_chatlog->append("文件传输已取消");

    this->enableInput();
    this->ui->lb_alert->setText("");
    this->ui->w_alert->hide();

    this->sendType = ChatWidget::Idle;
}

// 槽 当前连接是否为主动
/*
* Parameters:
*   isinitiative: is initiative
* Return :
*/
void ChatWidget::getIsInitiativeConn(bool isInitiative){
    // 是主动则再次发起连接
    if(isInitiative == true){
        emit reBindSocket(this);
    }
}

// 主动连接成功后更新聊天框信息
void ChatWidget::reSetCw(QString ip, QString user_name , QString system , QString mac , QString Platform) 
{
    qDebug() << "reset chat widget";

    // QHash<QString, QStandardItem*>::iterator iter;
    // for (iter = buddies->begin(); iter != buddies->end(); iter++) {
    //     qDebug() << "iter.key" << iter.key() << "\n";
    // }

    QStandardItem *buddyItem = NULL;
    if (this->buddies->contains(ip + " " + mac)) {
        buddyItem = this->buddies->value(ip + " " + mac);
        if (buddyItem != NULL) {
            this->dbuddy->fillFromItem(buddyItem);
        }
        
        this->ui->lb_head->setStyleSheet("QLabel{background-image: url(':/qml/dukto/" + dbuddy->osLogo() + "');}");
    }

    this->dbuddy->mIp = ip + " " + mac;
    this->dbuddy->mUsername = user_name;
    this->dbuddy->mPlatform = Platform;
    this->ip = ip + " " + mac;

    this->ui->le_ip->clear();
    this->ui->le_ip->hide();
    this->ui->le_ip->setEnabled(false);

    this->ui->lb_name->setText(user_name);
    this->ui->lb_machine->setText(mac);

    QString nickname = mSettings->nickname(mac);
    if (nickname != "error" && nickname != "") {
        this->ui->lb_name->setText(nickname);
        this->dbuddy->mUsername = nickname;
    }

    this->setWindowTitle(this->ui->lb_name->text());

    this->ui->pb_addname->show();
    this->ui->pb_sendmsg->show();
    this->ui->pb_sendfile->show();
    this->ui->pb_senddir->show();
    this->ui->lb_machine->show();
}

// 键盘F1响应用户手册
void ChatWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {

    case Qt::Key_F1:
        if (!mDaemonIpcDbus->daemonIsNotRunning()){
            //增加标题栏帮助菜单、F1快捷键打开用户手册
            mDaemonIpcDbus->showGuide("messages");
        }
        break;

    default:
        break;
    }
}

/*
* Summary: btn titleicon clicked
* Return :
*/
void ChatWidget::on_pb_titleicon_clicked(){
    // 聊天窗关闭时，恢复原样
    // 如果是好友界面，则需要在下一次打开时显示修改备注按钮
    if (!(this->ui->pb_checkname->isHidden())) {
        this->ui->pb_addname->show();
    }

    this->ui->le_nickname->clear();
    this->ui->le_nickname->hide();
    this->ui->pb_checkname->hide();
    this->ui->lb_name->show();
    this->ui->le_ip->clear();
    this->ui->w_alert->hide();

    if (this->ui->le_ip->isEnabled() == true) {
        this->ui->te_chat->clear();
    }

    this->close();
    //this->hide();
}

/*
* Summary: open a url
* Parameters:
*   url: http url
* Return :
*/
void ChatWidget::openUrl(QUrl url){
    QDesktopServices::openUrl(QUrl::fromLocalFile(url.toString()));
}

// 任务栏图标闪烁
void ChatWidget::msgAlert(){
    if(this->timeFlag % 2 == 0){
        this->setWindowIcon(this->iconBlank);
    }else{
        this->setWindowIcon(this->iconDukto);
    }
    this->timeFlag ++;
}

// 注释后使用主题拖动
// 窗口拖拽移动
// void ChatWidget::mousePressEvent(QMouseEvent *event){
//     mMoveing = true;
//     mMovePosition = event->globalPos() - pos();
// }
// void ChatWidget::mouseMoveEvent(QMouseEvent *event){
//     if (mMoveing && (event->buttons() && Qt::LeftButton)
//         && (event->globalPos()-mMovePosition).manhattanLength() > QApplication::startDragDistance())
//     {
//         move(event->globalPos()-mMovePosition);
//         mMovePosition = event->globalPos() - pos();
//     }
// }
// void ChatWidget::mouseReleaseEvent(QMouseEvent *){
//     mMoveing = false;
// }

// 设置是否在线
void ChatWidget::setOnLine(bool isOnLine){
    qDebug() << "setOnLine " << isOnLine;
    this->isOnLine = isOnLine;
}

// 后台消息处理
void ChatWidget::slotTransferMsg(int code){
    qDebug() << "ChatWidget::slotTransferMsg";
    if(code == CONN_SUCCESS){
        this->isOnLine = true;
        this->ui->lb_alert->setText("");
        this->ui->w_alert->hide();

        if(this->sendType == ChatWidget::Text){
            emit sendMsg(textToSend);
        }else if(this->sendType == ChatWidget::Files){
            emit sendFiles(fileToSend);
        }else if(this->sendType == ChatWidget::Dir){
            emit sendDir(dirToSend);
        }else{
        }
    }
    if(code == CONN_TIMEOUT){
        this->ui->lb_alert->setText(tr("conn failed, try again later"));
//        this->ui->lb_alert->setText("连接失败，请确认好友在线，稍后再试");
        this->ui->w_alert->show();
        this->enableInput();
    }
    if(code == DISCONN){
    }
    if(code == TRANSFERERR){
        this->ui->lb_alert->setText(tr("transmission error"));
//        this->ui->lb_alert->setText("数据传输错误");
        this->ui->w_alert->show();
    }
    if(code == TRY_RECONN){
        this->ui->lb_alert->setText(tr("connecting..."));
//        this->ui->lb_alert->setText("正在尝试连接");
        this->ui->w_alert->show();
    }
    if(code == SKIPFILE){
//        this->ui->lb_alert->setText("跳过了若干无权限的文件或符号链接");
//        this->ui->w_alert->show();
    }
}

// 点击修改备注
// 隐藏修改备注按钮，显示备注文本框和确认修改按钮
void ChatWidget::on_pb_addname_clicked()
{
    this->ui->pb_addname->hide();
    this->ui->lb_name->hide();
    this->ui->le_nickname->show();
    this->ui->pb_checkname->show();
    this->ui->le_nickname->setFocus();
}


// 确认修改按钮
// 获取备注文本框的内容，隐藏文本框和确认修改按钮，显示备注名和修改备注按钮

void ChatWidget::on_pb_checkname_clicked()
{
    this->ip = this->dbuddy->ip();

    QString nickname = this->ui->le_nickname->text().trimmed();
    QString mac = this->ip.split(" ")[1];

    if (nickname != "") {
        mSettings->saveNickname(mac, nickname);
        for (int i = 0; i < this->buddies->keys().size(); i ++){
            QStandardItem *item = this->buddies->value(this->buddies->keys().at(i));
            if(mac.trimmed() == item->data(BuddyListItemModel::Mac).toString().trimmed()){
                item->setData(nickname, BuddyListItemModel::Username);
                this->ui->lb_name->setText(nickname);
                this->dbuddy->mUsername = nickname;
                this->setWindowTitle(this->ui->lb_name->text());
//                qDebug() << "修改备注名" << nickname;
            }
        }
    }

    this->ui->le_nickname->clear();
    this->ui->le_nickname->hide();
    this->ui->pb_checkname->hide();
    this->ui->lb_name->show();
    this->ui->pb_addname->show();
}

