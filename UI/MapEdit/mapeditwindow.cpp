﻿#include "mapeditwindow.h"
#include "global.h"
#include "scene.h"
#include <QFileIconProvider>
#include "./dockview.h"
#include "./dockmaptree.h"
#include "./dockproperty.h"
#include "./dockblock.h"
#include "./dockgroup.h"

MapEditWindow::MapEditWindow(OneMap *_oneMap, QWidget *parent) : QMainWindow(parent),
    oneMap(_oneMap)
{
    setWindowTitle("Map edit");
    createActions();
    createStatusBar();

    init();
}

MapEditWindow::~MapEditWindow()
{
    delete oneMap;
}

void MapEditWindow::init()
{
    connect(&msgCenter,SIGNAL(sig_connection_connected()),this,SLOT(onServerConnect()));
    connect(&msgCenter,SIGNAL(sig_connection_disconnected()),this,SLOT(onServerDisconnect()));
    connect(&msgCenter,SIGNAL(sig_connection_conntectting()),this,SLOT(onServerConnectting()));

    connect(&msgCenter,SIGNAL(sendRequestFail()),this,SLOT(onSendFail()));
    connect(&msgCenter,SIGNAL(waitResponseTimeOut()),this,SLOT(onWaitResponseTimeOut()));
    connect(&msgCenter,SIGNAL(tip(QString)),this,SLOT(onTip(QString)));
    connect(&msgCenter,SIGNAL(err(int,QString)),this,SLOT(onErr(int,QString)));
    connect(&msgCenter,SIGNAL(sendNewRequest()),this,SLOT(onNewRequest()));

    connect(&msgCenter,SIGNAL(mapSetSuccess()),this,SLOT(onNewRequest()));
    connect(&msgCenter,SIGNAL(mapGetSuccess()),this,SLOT(onNewRequest()));

    //对树 的操作( 只有两个操作 添加楼层、选择不同的节点)
    connect(dockMapTree,SIGNAL(sig_addFloor(MapFloor*)),dockView,SLOT(slot_addFloor(MapFloor*)));
    connect(dockMapTree,SIGNAL(sig_chooseSpirit(MapSpirit*)),dockProperty,SLOT(slot_showSpirit(MapSpirit*)));
    connect(dockMapTree,SIGNAL(sig_chooseSpirit(MapSpirit*)),dockView,SLOT(slot_selectChanged(MapSpirit *)));

    connect(dockView,SIGNAL(sig_currentMousePos(QPointF)),this,SLOT(slot_currentMousePos(QPointF)));
    connect(dockView,SIGNAL(sig_cancelTool()),this,SLOT(slot_cancelTool()));
    connect(this,SIGNAL(sig_setTool(int)),dockView,SIGNAL(sig_setTool(int)));

    connect(dockView,SIGNAL(sig_add_remove_spirit()),dockMapTree,SLOT(refresh()));
    connect(dockView,SIGNAL(sig_add_remove_spirit()),dockProperty,SLOT(slot_shownull()));
    connect(dockView,SIGNAL(sig_chooseChanged(MapSpirit*)),dockMapTree,SLOT(slot_chooseChanged(MapSpirit*)));
    connect(dockView,SIGNAL(sig_chooseChanged(MapSpirit*)),dockProperty,SLOT(slot_showSpirit(MapSpirit*)));
    connect(dockView,SIGNAL(sig_propertyChanged(MapSpirit*)),dockProperty,SLOT(slot_propertyChanged(MapSpirit*)));

    //属性窗口 修改 到 地图窗口的信号
    connect(dockProperty,SIGNAL(sig_propertyChanged(MapSpirit *)),dockView,SIGNAL(sig_propertyChangedFromProperty(MapSpirit*)));
    connect(dockProperty,SIGNAL(sig_propertyChanged(MapSpirit *)),dockMapTree,SLOT(refresh()));

    connect(blockView,SIGNAL(sig_chooseSpirit(MapSpirit*)),dockProperty,SLOT(slot_showSpirit(MapSpirit *)));
    connect(groupView,SIGNAL(sig_chooseSpirit(MapSpirit*)),dockProperty,SLOT(slot_showSpirit(MapSpirit *)));

    connect(blockView,SIGNAL(sig_chooseSpirit(MapSpirit*)),dockView,SLOT(slot_selectChanged(MapSpirit *)));
    connect(groupView,SIGNAL(sig_chooseSpirit(MapSpirit*)),dockView,SLOT(slot_selectChanged(MapSpirit *)));

    connect(this,SIGNAL(sig_setSelectHand()),dockView,SIGNAL(sig_selectHand()));
    connect(this,SIGNAL(sig_setSelectSelect()),dockView,SIGNAL(sig_selectSelect()));
    connect(this,SIGNAL(sig_addBkg(int)),dockView,SLOT(slot_addBkg(int)));
    //connect(this,SIGNAL(sig_addBkg(MapBackground*)),dockMapTree,SLOT(refresh()));

    QMetaObject::connectSlotsByName(this);
}

void MapEditWindow::onServerConnect()
{
    statusbar_info(QStringLiteral("已连接"));
}

void MapEditWindow::onServerDisconnect()
{
    statusbar_info(QStringLiteral("连接已断开"));
}

void MapEditWindow::onServerConnectting()
{
    statusbar_info(QStringLiteral("尝试连接服务器..."));
}

void MapEditWindow::onSendFail()
{
    statusbar_err(QStringLiteral("发送请求失败"));
}

void MapEditWindow::onWaitResponseTimeOut()
{
    statusbar_err(QStringLiteral("等待响应超时"));
}
void MapEditWindow::onTip(QString s)
{
    statusbar_info(s);
}
void MapEditWindow::onErr(int errcode,QString info)
{
    statusbar_err(getErrorString(errcode)+"  "+info);
}

void MapEditWindow::onNewRequest()
{
    statusbar_err("");
    statusbar_info("");
}
void MapEditWindow::slot_load_map_success()
{
    msgCenter.mapLoad();
}
void MapEditWindow::slot_set_map_success()
{
    this->oneMap = g_onemap.clone();
    //TODO:重新载入//还是要重启
}

void MapEditWindow::createStatusBar()
{
    QStatusBar *statusbar = new QStatusBar(this);
    QLabel *usernamelabel = new QLabel(QStringLiteral("当前用户:")+current_user_info.username);
    QString role_name = "";
    switch (current_user_info.role) {
    case USER_ROLE_VISITOR:
        role_name =QStringLiteral( "未登录");
        break;
    case USER_ROLE_OPERATOR:
        role_name =QStringLiteral( "普通用户");
        break;
    case USER_ROLE_ADMIN:
        role_name =QStringLiteral( "管理员");
        break;
    case USER_ROLE_SUPER_ADMIN:
        role_name =QStringLiteral( "超级管理员");
        break;
    case USER_ROLE_DEVELOP:
        role_name =QStringLiteral( "开发人员");
        break;
    default:
        break;
    }
    QLabel *userrolelabel = new QLabel(QStringLiteral("当前角色:")+role_name);
    info_label = new QLabel(QStringLiteral("信息:"));
    error_label = new QLabel(QStringLiteral("错误:"));
    pos_label = new  QLabel(QStringLiteral("坐标:"));
    usernamelabel->setMinimumWidth(200);
    userrolelabel->setMinimumWidth(200);
    info_label->setMinimumWidth(200);
    error_label->setMinimumWidth(400);
    pos_label->setMinimumWidth(100);

    statusbar->addWidget(usernamelabel);
    statusbar->addWidget(userrolelabel);
    statusbar->addWidget(info_label);
    statusbar->addWidget(error_label);

    statusbar->addWidget(pos_label);

    setStatusBar(statusbar);
}

void MapEditWindow::createActions()
{

    dockMapTree = new DockMapTree(oneMap);
    dockProperty = new DockProperty(oneMap);
    dockView = new DockView(oneMap);
    blockView = new DockBlock(oneMap);
    groupView = new DockGroup(oneMap);

    addDockWidget(Qt::LeftDockWidgetArea,dockMapTree);
    addDockWidget(Qt::LeftDockWidgetArea,dockProperty);
    addDockWidget(Qt::RightDockWidgetArea,dockView);
    tabifyDockWidget(dockMapTree,blockView);
    tabifyDockWidget(blockView,groupView);


    QMenu *mapMenu = menuBar()->addMenu(tr("Maps"));
    QMenu *viewsMenu = menuBar()->addMenu(tr("Views"));
    QMenu *toolsMenu = menuBar()->addMenu(tr("Tools"));
    QToolBar *mapsToolBar = addToolBar(tr("Maps"));
    QToolBar *viewsToolBar = addToolBar(tr("Views"));
    QToolBar *selectToolBar = addToolBar(tr("Selects"));
    QToolBar *toolsToolBar = addToolBar(tr("Tools"));


    viewsMenu->addAction(dockMapTree->toggleViewAction());
    viewsToolBar->addAction(dockMapTree->toggleViewAction());

    viewsMenu->addAction(dockProperty->toggleViewAction());
    viewsToolBar->addAction(dockProperty->toggleViewAction());

    viewsMenu->addAction(dockView->toggleViewAction());
    viewsToolBar->addAction(dockView->toggleViewAction());

    viewsMenu->addAction(blockView->toggleViewAction());
    viewsToolBar->addAction(blockView->toggleViewAction());

    viewsMenu->addAction(groupView->toggleViewAction());
    viewsToolBar->addAction(groupView->toggleViewAction());

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAct = helpMenu->addAction(tr("&About"));
    aboutAct->setStatusTip(tr("Show the application's About box"));

    QAction *aboutQtAct = helpMenu->addAction(tr("About &HRG"));
    aboutQtAct->setStatusTip(tr("Show the HRG's About box"));

    toolSave = new QAction(this);
    toolSave->setText("save map");
    toolSave->setCheckable(false);
    toolSave->setObjectName("toolSave");
    toolLoad = new QAction(this);
    toolLoad->setText("load map");
    toolLoad->setObjectName("toolLoad");
    toolLoad->setCheckable(false);
    mapMenu->addAction(toolSave);
    mapMenu->addAction(toolLoad);
    mapsToolBar->addAction(toolSave);
    mapsToolBar->addAction(toolLoad);


    selectSelect = new QAction(this);
    selectSelect->setText("select");
    selectSelect->setObjectName("selectSelect");
//    QIcon iconSelectSelect;
//    iconSelectSelect.addFile(":/images/toolbar/edit-delete-2.png",QSize(),QIcon::Normal,QIcon::Off);
//    selectSelect->setIcon(iconSelectSelect);
    selectSelect->setCheckable(true);
    selectToolBar->addAction(selectSelect);
    toolsMenu->addAction(selectSelect);

    selectHand = new QAction(this);
    selectHand->setText("drag");
    selectHand->setObjectName("selectHand");
//    QIcon iconSelectHand;
//    iconSelectHand.addFile(":/images/toolbar/point-report.22.png",QSize(),QIcon::Normal,QIcon::Off);
//    selectHand->setIcon(iconSelectHand);
    selectHand->setCheckable(true);
    selectToolBar->addAction(selectHand);
    toolsMenu->addAction(selectHand);

    toolErase = new QAction(this);
    toolErase->setText("Eraser");
    toolErase->setObjectName("toolErase");
    QIcon iconErase;
    iconErase.addFile(":/images/toolbar/edit-delete-2.png",QSize(),QIcon::Normal,QIcon::Off);
    toolErase->setIcon(iconErase);
    toolErase->setCheckable(true);
    toolsToolBar->addAction(toolErase);
    toolsMenu->addAction(toolErase);

    toolStationDraw = new QAction(this);
    toolStationDraw->setText("Station Draw");
    toolStationDraw->setObjectName("toolStationDraw");
    QIcon iconStationDraw;
    iconStationDraw.addFile(":/images/toolbar/point-report.22.png",QSize(),QIcon::Normal,QIcon::Off);
    toolStationDraw->setIcon(iconStationDraw);
    toolStationDraw->setCheckable(true);
    toolsToolBar->addAction(toolStationDraw);
    toolsMenu->addAction(toolStationDraw);

    toolStationReport = new QAction(this);
    toolStationReport->setText("Station Report");
    toolStationReport->setObjectName("toolStationReport");
    QIcon iconStationReport;
    iconStationReport.addFile(":/images/toolbar/point-halt.22.png",QSize(),QIcon::Normal,QIcon::Off);
    toolStationReport->setIcon(iconStationReport);
    toolStationReport->setCheckable(true);
    toolsToolBar->addAction(toolStationReport);
    toolsMenu->addAction(toolStationReport);

    toolStationHalt = new QAction(this);
    toolStationHalt->setText("Station Halt");
    toolStationHalt->setObjectName("toolStationHalt");
    QIcon iconStationHalt;
    iconStationHalt.addFile(":/images/toolbar/point-park.22.png",QSize(),QIcon::Normal,QIcon::Off);
    toolStationHalt->setIcon(iconStationHalt);
    toolStationHalt->setCheckable(true);
    toolsToolBar->addAction(toolStationHalt);
    toolsMenu->addAction(toolStationHalt);

    toolStationCharge = new QAction(this);
    toolStationCharge->setText("Station Charge");
    toolStationCharge->setObjectName("toolStationCharge");
    QIcon iconStationCharge;
    iconStationCharge.addFile(":/images/point/ChargingStation.20x20.png",QSize(),QIcon::Normal,QIcon::Off);
    toolStationCharge->setIcon(iconStationCharge);
    toolStationCharge->setCheckable(true);
    toolsToolBar->addAction(toolStationCharge);
    toolsMenu->addAction(toolStationCharge);

    toolStationLoad = new QAction(this);
    toolStationLoad->setText("Station Load");
    toolStationLoad->setObjectName("toolStationLoad");
    QIcon iconStationLoad;
    iconStationLoad.addFile(":/images/point/TransferStation.20x20.png",QSize(),QIcon::Normal,QIcon::Off);
    toolStationLoad->setIcon(iconStationLoad);
    toolStationLoad->setCheckable(true);
    toolsToolBar->addAction(toolStationLoad);
    toolsMenu->addAction(toolStationLoad);

    toolStationUnload = new QAction(this);
    toolStationUnload->setText("Station Unload");
    toolStationUnload->setObjectName("toolStationUnload");
    QIcon iconStationUnload;
    iconStationUnload.addFile(":/images/point/TransferStation.20x20.png",QSize(),QIcon::Normal,QIcon::Off);
    toolStationUnload->setIcon(iconStationUnload);
    toolStationUnload->setCheckable(true);
    toolsToolBar->addAction(toolStationUnload);
    toolsMenu->addAction(toolStationUnload);

    toolStationLoadUnload = new QAction(this);
    toolStationLoadUnload->setText("Station Load Unload");
    toolStationLoadUnload->setObjectName("toolStationLoadUnload");
    QIcon iconStationLoadUnload;
    iconStationLoadUnload.addFile(":/images/point/TransferStation.20x20.png",QSize(),QIcon::Normal,QIcon::Off);
    toolStationLoadUnload->setIcon(iconStationLoadUnload);
    toolStationLoadUnload->setCheckable(true);
    toolsToolBar->addAction(toolStationLoadUnload);
    toolsMenu->addAction(toolStationLoadUnload);

    toolStationOrigin = new QAction(this);
    toolStationOrigin->setText("Station Origin");
    toolStationOrigin->setObjectName("toolStationOrigin");
    QIcon iconStationOrigin;
    iconStationOrigin.addFile(":/images/point/TransferStation.20x20.png",QSize(),QIcon::Normal,QIcon::Off);
    toolStationOrigin->setIcon(iconStationOrigin);
    toolStationOrigin->setCheckable(true);
    toolsToolBar->addAction(toolStationOrigin);
    toolsMenu->addAction(toolStationOrigin);

    toolLine = new QAction(this);
    toolLine->setText("Line");
    toolLine->setObjectName("toolLine");
    QIcon iconLine;
    iconLine.addFile(":/images/toolbar/path-direct.22.png",QSize(),QIcon::Normal,QIcon::Off);
    toolLine->setIcon(iconLine);
    toolLine->setCheckable(true);
    toolsToolBar->addAction(toolLine);
    toolsMenu->addAction(toolLine);

    toolQb = new QAction(this);
    toolQb->setText("2-Bezier");
    toolQb->setObjectName("toolQb");
    QIcon iconQb;
    iconQb.addFile(":/images/toolbar/path-bezier.22.png",QSize(),QIcon::Normal,QIcon::Off);
    toolQb->setIcon(iconQb);
    toolQb->setCheckable(true);
    toolsToolBar->addAction(toolQb);
    toolsMenu->addAction(toolQb);

    toolCb = new QAction(this);
    toolCb->setText("3-Bezier");
    toolCb->setObjectName("toolCb");
    QIcon iconCb;
    iconCb.addFile(":/images/toolbar/path-bezier.22.png",QSize(),QIcon::Normal,QIcon::Off);
    toolCb->setIcon(iconCb);
    toolCb->setCheckable(true);
    toolsToolBar->addAction(toolCb);
    toolsMenu->addAction(toolCb);

    addBkgd = new QAction(this);
    addBkgd->setText("set background img");
    addBkgd->setObjectName("addBkgd");
    //            QIcon iconAddBkg;
    //            iconAddBkg.addFile(":/images/toolbar/path-bezier.22.png",QSize(),QIcon::Normal,QIcon::Off);
    //            addBkgd->setIcon(iconCb);
    addBkgd->setCheckable(true);
    toolsToolBar->addAction(addBkgd);
    toolsMenu->addAction(addBkgd);

}

void MapEditWindow::about()
{
    QMessageBox::about(this, tr("About Application"),
                       tr("The <b>HRG Dispatch client</b> connect to dispatch server,<br>"
                          "and show the ui for user to watch the status of agvs and<br>"
                          "other information about the agv dispatch system."));
}

void MapEditWindow::aboutHrg()
{
    QMessageBox::about(this, tr("About Hit Robot Group"),
                       tr("<b>Hit Robot Group</b><br>"
                          "we have the top robotics research scientists in<br>"
                          "the country and we are in the robot technology <br>"
                          "subject frountirer struggle,unity,hard work of<br>"
                          "high level research team.<br>"
                          "To get more information about us,you can visit our<br>"
                          " website: <a href=\'http://www.hrgrobotics.com/\'>HRG</a>"));
}

void MapEditWindow::statusbar_info(QString msg)
{
    info_label->setText(QStringLiteral("信息:")+msg);
}

void MapEditWindow::statusbar_err(QString msg)
{
    error_label->setText(QStringLiteral("错误:")+msg);
}

void MapEditWindow::statusbar_pos(QString msg)
{
    pos_label->setText(QStringLiteral("坐标:")+msg);
}

void MapEditWindow::slot_currentMousePos(QPointF pos)
{
    statusbar_pos(QString("(%1,%2)").arg(pos.x()).arg(pos.y()));
}

void MapEditWindow::slot_cancelTool()
{
    toolErase->setChecked(false);
    toolStationDraw->setChecked(false);
    toolStationReport->setChecked(false);
    toolStationHalt->setChecked(false);
    toolStationCharge->setChecked(false);
    toolStationLoad->setChecked(false);
    toolStationUnload->setChecked(false);
    toolStationLoadUnload->setChecked(false);
    toolStationOrigin->setChecked(false);
    toolLine->setChecked(false);
    toolQb->setChecked(false);
    toolCb->setChecked(false);

    selectSelect->setChecked(true);
    selectHand->setChecked(false);
    emit sig_setTool(Scene::T_NONE);
}

void MapEditWindow::on_toolErase_triggered(bool b)
{
    //toolErase->setChecked(false);
    toolStationDraw->setChecked(false);
    toolStationReport->setChecked(false);
    toolStationHalt->setChecked(false);
    toolStationCharge->setChecked(false);
    toolStationLoad->setChecked(false);
    toolStationUnload->setChecked(false);
    toolStationLoadUnload->setChecked(false);
    toolStationOrigin->setChecked(false);
    toolLine->setChecked(false);
    toolQb->setChecked(false);
    toolCb->setChecked(false);

    if(b){
        emit sig_setTool(Scene::T_ERASER);
    }else{
        emit sig_setTool(Scene::T_NONE);
    }
}

void MapEditWindow::on_toolStationDraw_triggered(bool b)
{
    toolErase->setChecked(false);
    //toolStationDraw->setChecked(false);
    toolStationReport->setChecked(false);
    toolStationHalt->setChecked(false);
    toolStationCharge->setChecked(false);
    toolStationLoad->setChecked(false);
    toolStationUnload->setChecked(false);
    toolStationLoadUnload->setChecked(false);
    toolStationOrigin->setChecked(false);
    toolLine->setChecked(false);
    toolQb->setChecked(false);
    toolCb->setChecked(false);

    if(b){
        emit sig_setTool(Scene::T_STATION_DRAW);
    }else{
        emit sig_setTool(Scene::T_NONE);
    }
}

void MapEditWindow::on_toolStationReport_triggered(bool b)
{
    toolErase->setChecked(false);
    toolStationDraw->setChecked(false);
    //toolStationReport->setChecked(false);
    toolStationHalt->setChecked(false);
    toolStationCharge->setChecked(false);
    toolStationLoad->setChecked(false);
    toolStationUnload->setChecked(false);
    toolStationLoadUnload->setChecked(false);
    toolStationOrigin->setChecked(false);
    toolLine->setChecked(false);
    toolQb->setChecked(false);
    toolCb->setChecked(false);

    if(b){
        emit sig_setTool(Scene::T_STATION_REPORT);
    }else{
        emit sig_setTool(Scene::T_NONE);
    }
}

void MapEditWindow::on_toolStationHalt_triggered(bool b)
{
    toolErase->setChecked(false);
    toolStationDraw->setChecked(false);
    toolStationReport->setChecked(false);
    //toolStationHalt->setChecked(false);
    toolStationCharge->setChecked(false);
    toolStationLoad->setChecked(false);
    toolStationUnload->setChecked(false);
    toolStationLoadUnload->setChecked(false);
    toolStationOrigin->setChecked(false);
    toolLine->setChecked(false);
    toolQb->setChecked(false);
    toolCb->setChecked(false);

    if(b){
        emit sig_setTool(Scene::T_STATION_HALT);
    }else{
        emit sig_setTool(Scene::T_NONE);
    }
}
void MapEditWindow::on_toolStationCharge_triggered(bool b)
{
    toolErase->setChecked(false);
    toolStationDraw->setChecked(false);
    toolStationReport->setChecked(false);
    toolStationHalt->setChecked(false);
    //toolStationCharge->setChecked(false);
    toolStationLoad->setChecked(false);
    toolStationUnload->setChecked(false);
    toolStationLoadUnload->setChecked(false);
    toolStationOrigin->setChecked(false);
    toolLine->setChecked(false);
    toolQb->setChecked(false);
    toolCb->setChecked(false);

    if(b){
        emit sig_setTool(Scene::T_STATION_CHARGE);
    }else{
        emit sig_setTool(Scene::T_NONE);
    }
}
void MapEditWindow::on_toolStationLoad_triggered(bool b)
{
    toolErase->setChecked(false);
    toolStationDraw->setChecked(false);
    toolStationReport->setChecked(false);
    toolStationHalt->setChecked(false);
    toolStationCharge->setChecked(false);
    //toolStationLoad->setChecked(false);
    toolStationUnload->setChecked(false);
    toolStationLoadUnload->setChecked(false);
    toolStationOrigin->setChecked(false);
    toolLine->setChecked(false);
    toolQb->setChecked(false);
    toolCb->setChecked(false);

    if(b){
        emit sig_setTool(Scene::T_STATION_LOAD);
    }else{
        emit sig_setTool(Scene::T_NONE);
    }
}
void MapEditWindow::on_toolStationUnload_triggered(bool b)
{
    toolErase->setChecked(false);
    toolStationDraw->setChecked(false);
    toolStationReport->setChecked(false);
    toolStationHalt->setChecked(false);
    toolStationCharge->setChecked(false);
    toolStationLoad->setChecked(false);
    //toolStationUnload->setChecked(false);
    toolStationLoadUnload->setChecked(false);
    toolStationOrigin->setChecked(false);
    toolLine->setChecked(false);
    toolQb->setChecked(false);
    toolCb->setChecked(false);

    if(b){
        emit sig_setTool(Scene::T_STATION_UNLOAD);
    }else{
        emit sig_setTool(Scene::T_NONE);
    }
}

void MapEditWindow::on_toolStationLoadUnload_triggered(bool b)
{
    toolErase->setChecked(false);
    toolStationDraw->setChecked(false);
    toolStationReport->setChecked(false);
    toolStationHalt->setChecked(false);
    toolStationCharge->setChecked(false);
    toolStationLoad->setChecked(false);
    toolStationUnload->setChecked(false);
    //toolStationLoadUnload->setChecked(false);
    toolStationOrigin->setChecked(false);
    toolLine->setChecked(false);
    toolQb->setChecked(false);
    toolCb->setChecked(false);

    if(b){
        emit sig_setTool(Scene::T_STATION_LOAD_UNLOAD);
    }else{
        emit sig_setTool(Scene::T_NONE);
    }
}

void MapEditWindow::on_toolStationOrigin_triggered(bool b)
{
    toolErase->setChecked(false);
    toolStationDraw->setChecked(false);
    toolStationReport->setChecked(false);
    toolStationHalt->setChecked(false);
    toolStationCharge->setChecked(false);
    toolStationLoad->setChecked(false);
    toolStationUnload->setChecked(false);
    toolStationLoadUnload->setChecked(false);
    //toolStationOrigin->setChecked(false);
    toolLine->setChecked(false);
    toolQb->setChecked(false);
    toolCb->setChecked(false);

    if(b){
        emit sig_setTool(Scene::T_STATION_ORIGIN);
    }else{
        emit sig_setTool(Scene::T_NONE);
    }
}

void MapEditWindow::on_toolLine_triggered(bool b)
{
    toolErase->setChecked(false);
    toolStationDraw->setChecked(false);
    toolStationReport->setChecked(false);
    toolStationHalt->setChecked(false);
    toolStationCharge->setChecked(false);
    toolStationLoad->setChecked(false);
    toolStationUnload->setChecked(false);
    toolStationLoadUnload->setChecked(false);
    //toolLine->setChecked(false);
    toolQb->setChecked(false);
    toolCb->setChecked(false);

    if(b){
        emit sig_setTool(Scene::T_LINE);
    }else{
        emit sig_setTool(Scene::T_NONE);
    }
}
void MapEditWindow::on_toolQb_triggered(bool b)
{
    toolErase->setChecked(false);
    toolStationDraw->setChecked(false);
    toolStationReport->setChecked(false);
    toolStationHalt->setChecked(false);
    toolStationCharge->setChecked(false);
    toolStationLoad->setChecked(false);
    toolStationUnload->setChecked(false);
    toolStationLoadUnload->setChecked(false);
    toolLine->setChecked(false);
    //toolQb->setChecked(false);
    toolCb->setChecked(false);

    if(b){
        emit sig_setTool(Scene::T_QB);
    }else{
        emit sig_setTool(Scene::T_NONE);
    }
}
void MapEditWindow::on_toolCb_triggered(bool b)
{
    toolErase->setChecked(false);
    toolStationDraw->setChecked(false);
    toolStationReport->setChecked(false);
    toolStationHalt->setChecked(false);
    toolStationCharge->setChecked(false);
    toolStationLoad->setChecked(false);
    toolStationUnload->setChecked(false);
    toolStationLoadUnload->setChecked(false);
    toolLine->setChecked(false);
    toolQb->setChecked(false);
    //toolCb->setChecked(false);

    if(b){
        emit sig_setTool(Scene::T_CB);
    }else{
        emit sig_setTool(Scene::T_NONE);
    }
}

void MapEditWindow::on_selectSelect_triggered(bool b)
{
    toolErase->setChecked(false);
    toolStationDraw->setChecked(false);
    toolStationReport->setChecked(false);
    toolStationHalt->setChecked(false);
    toolStationCharge->setChecked(false);
    toolStationLoad->setChecked(false);
    toolStationUnload->setChecked(false);
    toolStationLoadUnload->setChecked(false);
    toolLine->setChecked(false);
    toolQb->setChecked(false);
    toolCb->setChecked(false);
    //selectSelect->setChecked(false);
    selectHand->setChecked(false);
    emit sig_setTool(Scene::T_NONE);
    emit sig_setSelectSelect();
}

void MapEditWindow::on_selectHand_triggered(bool b)
{
    toolErase->setChecked(false);
    toolStationDraw->setChecked(false);
    toolStationReport->setChecked(false);
    toolStationHalt->setChecked(false);
    toolStationCharge->setChecked(false);
    toolStationLoad->setChecked(false);
    toolStationUnload->setChecked(false);
    toolStationLoadUnload->setChecked(false);
    toolLine->setChecked(false);
    toolQb->setChecked(false);
    toolCb->setChecked(false);
    selectSelect->setChecked(false);
    //selectHand->setChecked(false);
    emit sig_setTool(Scene::T_NONE);
    if(b)
        emit sig_setSelectHand();
    else
        emit sig_setSelectSelect();
}

void MapEditWindow::on_toolSave_triggered(bool b)
{
    msgCenter.mapSave(oneMap);
}

void MapEditWindow::on_toolLoad_triggered(bool b)
{
    msgCenter.mapLoad();
}

void MapEditWindow::on_addBkgd_triggered(bool b)
{
    if(b){
        toolErase->setChecked(false);
        toolStationDraw->setChecked(false);
        toolStationReport->setChecked(false);
        toolStationHalt->setChecked(false);
        toolStationCharge->setChecked(false);
        toolStationLoad->setChecked(false);
        toolStationUnload->setChecked(false);
        toolStationLoadUnload->setChecked(false);
        toolLine->setChecked(false);
        toolQb->setChecked(false);
        toolCb->setChecked(false);
        selectSelect->setChecked(false);
        selectHand->setChecked(false);
        emit sig_setTool(Scene::T_NONE);

        if(oneMap->getFloors().size()>0){
            QString filePath = QFileDialog::getOpenFileName(this,tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));
            if(filePath.length()>0){
                //file name
                QString fileName = filePath.right(filePath.length() - filePath.lastIndexOf("/")-1);

                //data
                QByteArray ba;
                QFile *file = new QFile(filePath);
                file->open(QIODevice::ReadOnly);
                ba = file->readAll();
                file->close();

                //img
                QImage img;
                img.load(filePath);

                MapBackground *_bkg = new MapBackground(oneMap->getNextId(),fileName.toStdString(),ba.data(),ba.length(),img.width(),img.height(),fileName.toStdString());
                oneMap->addSpirit(_bkg);
                emit sig_addBkg(_bkg->getId());
            }
        }
        addBkgd->setChecked(false);
    }
}
