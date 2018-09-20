﻿#include <assert.h>

#include "monitorscene.h"
#include "monitormapitemagv.h"
#include "global.h"
#include <QtWidgets>

MonitorScene::MonitorScene(OneMap *_onemap, MapFloor *_floor, QObject *parent) : QGraphicsScene(parent),
    cur_tool(T_NONE),
    onemap(_onemap),
    floor(_floor),
    oldSelectStation(nullptr),
    bkg(nullptr)
{
    build();
}

MonitorScene::~MonitorScene()
{
}

void MonitorScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    //    QPen oldpen = painter->pen();
    //    //    QBrush oldbrush = painter->brush();
    //    //添加原点
    //    QPen pen(Qt::blue);
    //    pen.setWidth(1);
    //    painter->setPen(pen);
    //    painter->drawEllipse(QPoint(0,0),5,5);

    //    //添加x轴
    //    QPen pen2(Qt::magenta);
    //    painter->setPen(pen2);

    //    painter->drawLine(QPoint(-100000,0),QPoint(100000,0));

    //    //添加y轴
    //    painter->drawLine(QPoint(0,-100000),QPoint(0,100000));
}


void MonitorScene::build()
{
    //TODO:
    QList<int> points = QList<int>::fromStdList(floor->getPoints());
    foreach (auto p, points) {
        MapPoint *ppp = static_cast<MapPoint *>(onemap->getSpiritById(p));
        if(ppp==nullptr)continue;
        MonitorMapItemStation *station = new MonitorMapItemStation(ppp);
        MonitorMapItemStationName *stationname = new MonitorMapItemStationName(station,ppp);
        connect(station,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
        connect(stationname,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
        addItem(station);
        addItem(stationname);
        iStations.append(station);
        iStationNames.append(stationname);
    }

    QList<int> path = QList<int>::fromStdList(floor->getPaths());
    foreach (auto p, path) {
        MapPath *ppp = static_cast<MapPath *>(onemap->getSpiritById(p));
        if(ppp==nullptr)continue;
        if(ppp->getPathType() == MapPath::Map_Path_Type_Line){
            MonitorMapItemStation *start = nullptr;
            MonitorMapItemStation *end = nullptr;

            foreach (auto s, iStations) {
                if(s->getPoint()->getId() == ppp->getStart()){start = s;}
                if(s->getPoint()->getId() == ppp->getEnd()){end = s;}
            }
            if(start == nullptr || end == nullptr)continue ;

            MonitorMapItemLine *l = new MonitorMapItemLine(start,end,ppp);
            connect(l,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
            addItem(l);
            iLines.push_back(l);
        }
        else if(ppp->getPathType() == MapPath::Map_Path_Type_Quadratic_Bezier){
            MonitorMapItemStation *start = nullptr;
            MonitorMapItemStation *end = nullptr;
            foreach (auto s, iStations) {
                if(s->getPoint()->getId() == ppp->getStart()){start = s;}
                if(s->getPoint()->getId() == ppp->getEnd()){end = s;}
            }
            if(start == nullptr || end == nullptr)continue ;

            MonitorMapItemQuadraticBezier *l = new MonitorMapItemQuadraticBezier(start,end,ppp);
            connect(l,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
            addItem(l);
            iQbs.push_back(l);
        }
        else if(ppp->getPathType() == MapPath::Map_Path_Type_Cubic_Bezier){
            MonitorMapItemStation *start = nullptr;
            MonitorMapItemStation *end = nullptr;
            foreach (auto s, iStations) {
                if(s->getPoint()->getId() == ppp->getStart()){start = s;}
                if(s->getPoint()->getId() == ppp->getEnd()){end = s;}
            }
            if(start == nullptr || end == nullptr)continue ;

            MonitorMapItemCubicBezier *l = new MonitorMapItemCubicBezier(start,end,ppp);
            connect(l,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
            addItem(l);
            iCbs.push_back(l);
        }
    }

    MapBackground *_bkg =static_cast<MapBackground *>( onemap->getSpiritById( floor->getBkg()));
    if(_bkg != nullptr){
        bkg = new MonitorMapItemBkg(_bkg);
        //connect(bkg,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
        addItem(bkg);
    }

    update();

    connect(this,SIGNAL(selectionChanged()),this,SLOT(onSelectItemChanged()));
    //connect(&msgCenter,SIGNAL(sig_pub_agv_position(int,QString,double,double,double)),this,SIGNAL(sig_pub_agv_postion(int,QString,double,double,double)));
    connect(&msgCenter,SIGNAL(sig_pub_agv_position(int,QString,double,double,double, QStringList, int)),this,SLOT(slot_pub_agv_postion(int,QString,double,double,double,QStringList,int)));
    connect(&msgCenter,SIGNAL(sig_pub_agv_occus()),this,SLOT(slot_agv_occus_set_color()));
}

void MonitorScene::slot_setCurTool(int t)
{
    //    if(t>=T_NONE && t<=T_CB){
    //        if(cur_tool == t)return ;
    //        cur_tool = static_cast<Tool>(t);
    //        oldSelectStation = nullptr;
    //    }
}

void MonitorScene::slot_agv_occus_set_color()
{
    foreach (auto s, iStations) {
        int id = s->getPoint()->getId();
        bool setColor = false;

        foreach (auto agv, agvs) {
            auto dd = agvOccus[agv->getId()];
            if(dd.contains(QString("%1").arg(id))){
                setColor = true;
                s->setColor(agv->getColor());
                break;
            }
        }

        if(!setColor){
            s->setColor(Qt::black);
        }
    }

    foreach (auto s, iStationNames) {
        int id = s->getPoint()->getId();
        bool setColor = false;

        foreach (auto agv, agvs) {
            auto dd = agvOccus[agv->getId()];
            if(dd.contains(QString("%1").arg(id))){
                setColor = true;
                s->setColor(agv->getColor());
                break;
            }
        }

        if(!setColor){
            s->setColor(Qt::black);
        }
    }

    foreach (auto s, iLines) {
        int id = s->getPath()->getId();
        bool setColor = false;

        foreach (auto agv, agvs) {
            auto dd = agvOccus[agv->getId()];
            if(dd.contains(QString("%1").arg(id))){
                setColor = true;
                s->setColor(agv->getColor());
                break;
            }
        }

        if(!setColor){
            s->setColor(Qt::black);
        }
    }

    foreach (auto s, iQbs) {
        int id = s->getPath()->getId();
        bool setColor = false;

        foreach (auto agv, agvs) {
            auto dd = agvOccus[agv->getId()];
            if(dd.contains(QString("%1").arg(id))){
                setColor = true;
                s->setColor(agv->getColor());
                break;
            }
        }

        if(!setColor){
            s->setColor(Qt::black);
        }
    }

    foreach (auto s, iCbs) {
        int id = s->getPath()->getId();
        bool setColor = false;

        foreach (auto agv, agvs) {
            auto dd = agvOccus[agv->getId()];
            if(dd.contains(QString("%1").arg(id))){
                setColor = true;
                s->setColor(agv->getColor());
                break;
            }
        }

        if(!setColor){
            s->setColor(Qt::black);
        }
    }

}

void MonitorScene::slot_pub_agv_postion(int id, QString name, double x, double y, double theta, QStringList qsl, int _floor)
{
    if(!agvIds.contains(id)){
        agvIds<<id;
        MonitorMapItemAgv *itemAgv = new MonitorMapItemAgv(id,name, floor->getId());
        itemAgv->setPos(x,y);
        itemAgv->setRotation(-1*theta);
        connect(&msgCenter,SIGNAL(sig_pub_agv_position(int,QString,double,double,double,QStringList, int)),itemAgv,SLOT(slot_update_pos(int,QString,double,double,double,QStringList, int)));
        addItem(itemAgv);
        agvs.push_back(itemAgv);
    }
    agvOccus[id] = qsl;
}

void MonitorScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if(cur_tool == T_STATION_DRAW
                || cur_tool ==T_STATION_REPORT
                || cur_tool ==T_STATION_HALT
                || cur_tool ==T_STATION_CHARGE
                || cur_tool ==T_STATION_LOAD
                || cur_tool ==T_STATION_UNLOAD
                || cur_tool ==T_STATION_LOAD_UNLOAD)
        {
            MapPoint::Map_Point_Type type;
            switch (cur_tool) {
            case T_STATION_DRAW:
                type = MapPoint::Map_Point_Type_Draw;
                break;
            case T_STATION_REPORT:
                type = MapPoint::Map_Point_Type_REPORT;
                break;
            case T_STATION_HALT:
                type = MapPoint::Map_Point_Type_HALT;
                break;
            case T_STATION_CHARGE:
                type = MapPoint::Map_Point_Type_CHARGE;
                break;
            case T_STATION_LOAD:
                type = MapPoint::Map_Point_Type_LOAD;
                break;
            case T_STATION_UNLOAD:
                type = MapPoint::Map_Point_Type_UNLOAD;
                break;
            case T_STATION_LOAD_UNLOAD:
                type = MapPoint::Map_Point_Type_LOAD_UNLOAD;
                break;
            case T_STATION_ORIGIN:
                type = MapPoint::Map_Point_Type_ORIGIN;
                break;
            default:
                type = MapPoint::Map_Point_Type_HALT;
                break;
            }
            //添加站点~
            int id = onemap->getNextId();
            QString name = QString("station %1").arg(id);
            MapPoint *p = new MapPoint(id,name.toStdString(),type,event->scenePos().x(),event->scenePos().y());
            onemap->addSpirit(p);
            floor->addPoint(p->getId());

            //添加item
            MonitorMapItemStation *station= new MonitorMapItemStation(p);
            MonitorMapItemStationName *stationname = new MonitorMapItemStationName(station,p);
            connect(station,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
            connect(stationname,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
            addItem(station);
            addItem(stationname);
            iStations.push_back(station);
            iStationNames.push_back(stationname);
            update();

            //发射信号
            emit sig_add_remove_spirit();
        }
    }

    else if(event->button() == Qt::RightButton){
        slot_setCurTool(T_NONE);
        emit sig_cancelTool();
    }

    QGraphicsScene::mousePressEvent(event);
}

void MonitorScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    emit sig_currentMousePos(event->scenePos());
    QGraphicsScene::mouseMoveEvent(event);
}

//void MonitorScene::setBackgroundImagePath(QString _path)
//{
//    QImage img(_path);
//    if(img.isNull())
//    {
//        QMessageBox::warning(nullptr,QStringLiteral("错误"),QStringLiteral("非图像文件或不存在"));
//        return ;
//    }

//    QString fileNameWithType = "image.jpg";
//    if(_path.contains("\\"))
//        fileNameWithType = _path.right(_path.length() - _path.lastIndexOf("\\"));
//    else if(_path.contains("/"))
//        fileNameWithType = _path.right(_path.length() - _path.lastIndexOf("/")-1);
//    QString fileName = fileNameWithType.left(fileNameWithType.indexOf("."));

//    MapBackground *bk = new MapBackground(onemap->getNextId(),fileName,img,fileName);

//    if(bkg!=nullptr && bkg->getBkg()!=nullptr){
//        //删除原来的元素
//        removeItem(bkg);
//        bkg = nullptr;
//    }

//    if(bkg==nullptr){
//        bkg = new MonitorMapItemBkg(bk);
//        connect(bkg,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
//        addItem(bkg);
//    }else{
//        bkg->setBkg(bk);
//        bkg->update(bkg->boundingRect());
//    }
//    floor->setBkg(bk);
//    emit sig_add_remove_spirit();
//    update();
//}

void MonitorScene::addSpirit(MapFloor *_floor,MapSpirit *_spirit)
{
    if(_floor != floor){
        return ;
    }
    if(MapSpirit::Map_Sprite_Type_Point == _spirit->getSpiritType())
    {
        MapPoint *p = static_cast<MapPoint *>(_spirit);
        MonitorMapItemStation *sstation = new MonitorMapItemStation(p);
        MonitorMapItemStationName *sstationName = new MonitorMapItemStationName(sstation,p);
        connect(sstation,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
        connect(sstationName,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
        addItem(sstation);
        addItem(sstationName);
        iStations.push_back(sstation);
        iStationNames.append(sstationName);
        update();
    }else if(MapSpirit::Map_Sprite_Type_Path == _spirit->getSpiritType())
    {
        MapPath *p = static_cast<MapPath *>(_spirit);
        if(p->getPathType() == MapPath::Map_Path_Type_Line){
            MonitorMapItemStation *start = nullptr;
            MonitorMapItemStation *end = nullptr;

            foreach (auto s, iStations) {
                if(s->getPoint()->getId() == p->getStart()){start = s;}
                if(s->getPoint()->getId() == p->getEnd()){end = s;}
            }
            if(start == nullptr || end == nullptr)return ;

            MonitorMapItemLine *l = new MonitorMapItemLine(start,end,p);
            connect(l,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
            addItem(l);
            iLines.push_back(l);
            update();
        }
        else if(p->getPathType() == MapPath::Map_Path_Type_Quadratic_Bezier){
            MonitorMapItemStation *start = nullptr;
            MonitorMapItemStation *end = nullptr;
            foreach (auto s, iStations) {
                if(s->getPoint()->getId() == p->getStart()){start = s;}
                if(s->getPoint()->getId() == p->getEnd()){end = s;}
            }
            if(start == nullptr || end == nullptr)return ;

            MonitorMapItemQuadraticBezier *l = new MonitorMapItemQuadraticBezier(start,end,p);
            connect(l,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
            addItem(l);
            iQbs.push_back(l);
            update();
        }
        else if(p->getPathType() == MapPath::Map_Path_Type_Cubic_Bezier){
            MonitorMapItemStation *start = nullptr;
            MonitorMapItemStation *end = nullptr;
            foreach (auto s, iStations) {
                if(s->getPoint()->getId() == p->getStart()){start = s;}
                if(s->getPoint()->getId() == p->getEnd()){end = s;}
            }
            if(start == nullptr || end == nullptr)return ;

            MonitorMapItemCubicBezier *l = new MonitorMapItemCubicBezier(start,end,p);
            connect(l,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
            addItem(l);
            iCbs.push_back(l);
            update();
        }
    }
}

void MonitorScene::propertyChanged(MapSpirit *_spirit)
{
    if(_spirit->getSpiritType() == MapSpirit::Map_Sprite_Type_Point){
        MapPoint *p = static_cast<MapPoint *>(_spirit);
        foreach (auto station, iStations) {
            if(station->getPoint() == p){
                station->my_update();
                break;
            }
        }
        foreach (auto name , iStationNames) {
            if(name->getPoint() == p){
                name->my_update();
                break;
            }
        }
    }
    else if(_spirit->getSpiritType() == MapSpirit::Map_Sprite_Type_Path){
        MapPath *path = static_cast<MapPath *>(_spirit);
        bool lineTypeChange = true;
        if(path->getPathType() == MapPath::Map_Path_Type_Line){
            foreach (auto l, iLines) {
                if(l->getPath() == path){
                    lineTypeChange = false;
                    l->my_update();
                    break;
                }
            }
        }
        else  if(path->getPathType() == MapPath::Map_Path_Type_Quadratic_Bezier){
            foreach (auto l, iQbs) {
                if(l->getPath() == path){
                    lineTypeChange = false;
                    l->my_update();
                    break;
                }
            }
        }
        else  if(path->getPathType() == MapPath::Map_Path_Type_Cubic_Bezier){
            foreach (auto l, iCbs) {
                if(l->getPath() == path){
                    lineTypeChange = false;
                    l->my_update();
                    break;
                }
            }
        }
        qDebug()<<"lineTypeChange = "<<lineTypeChange;
        if(lineTypeChange){
            //TODO:
            //刪除item
            foreach (auto l, iLines) {
                if(l->getPath() == path){
                    removeItem(l);
                    break;
                }
            }
            foreach (auto l, iQbs) {
                if(l->getPath() == path){
                    removeItem(l);
                    break;
                }
            }
            foreach (auto l, iCbs) {
                if(l->getPath() == path){
                    removeItem(l);
                    break;
                }
            }

            //添加新的item
            if(path->getPathType() == MapPath::Map_Path_Type_Line){
                MonitorMapItemStation *start = nullptr;
                MonitorMapItemStation *end = nullptr;

                foreach (auto s, iStations) {
                    if(s->getPoint()->getId() == path->getStart()){start = s;}
                    if(s->getPoint()->getId() == path->getEnd()){end = s;}
                }
                if(start == nullptr || end == nullptr)return ;

                MonitorMapItemLine *l = new MonitorMapItemLine(start,end,path);
                connect(l,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
                addItem(l);
                iLines.push_back(l);
                update();
            }else if(path->getPathType() == MapPath::Map_Path_Type_Quadratic_Bezier){
                MonitorMapItemStation *start = nullptr;
                MonitorMapItemStation *end = nullptr;
                foreach (auto s, iStations) {
                    if(s->getPoint()->getId() == path->getStart()){start = s;}
                    if(s->getPoint()->getId() == path->getEnd()){end = s;}
                }
                if(start == nullptr || end == nullptr)return ;

                MonitorMapItemQuadraticBezier *l = new MonitorMapItemQuadraticBezier(start,end,path);
                connect(l,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
                addItem(l);
                iQbs.push_back(l);
                update();
            }else if(path->getPathType() == MapPath::Map_Path_Type_Cubic_Bezier){
                MonitorMapItemStation *start = nullptr;
                MonitorMapItemStation *end = nullptr;
                foreach (auto s, iStations) {
                    if(s->getPoint()->getId() == path->getStart()){start = s;}
                    if(s->getPoint()->getId() == path->getEnd()){end = s;}
                }
                if(start == nullptr || end == nullptr)return ;

                MonitorMapItemCubicBezier *l = new MonitorMapItemCubicBezier(start,end,path);
                connect(l,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
                addItem(l);
                iCbs.push_back(l);
                update();
            }
        }
    }
    else if(_spirit->getSpiritType() == MapSpirit::Map_Sprite_Type_Background){
        MapBackground * b = static_cast<MapBackground *>(_spirit);
        if(bkg!=nullptr && bkg->getBkg() == b){
            bkg->my_update();
        }
    }
    else if(_spirit->getSpiritType() == MapSpirit::Map_Sprite_Type_Floor){
        //add bkg!
        MapFloor *_floor = static_cast<MapFloor *>(_spirit);
        if(_floor == this->floor){
            if(floor->getBkg() == 0 && bkg == nullptr){
                //donothing
            }
            else if(floor->getBkg() == 0 && bkg != nullptr){
                //remove current bkg
                removeItem(bkg);
                bkg = nullptr;
                update();
            }else if(floor->getBkg() != 0 && bkg == nullptr){
                //add bkg
                MapBackground *mbkg = static_cast<MapBackground *>( onemap->getSpiritById(floor->getBkg()));
                if(mbkg!=nullptr){
                    bkg = new MonitorMapItemBkg(mbkg);
                    addItem(bkg);
                }
            }
            else if(floor->getBkg()!=0 && bkg != nullptr){
                if(floor->getBkg() == bkg->getBkg()->getId()){
                    //same with old //do nothing
                }else{
                    //remove old bkg
                    removeItem(bkg);
                    bkg = nullptr;
                    update();

                    //add new bkg
                    MapBackground *mbkg = static_cast<MapBackground *>( onemap->getSpiritById(floor->getBkg()));
                    if(mbkg!=nullptr){
                        bkg = new MonitorMapItemBkg(mbkg);
                        addItem(bkg);
                    }
                }
            }
        }
    }
}

void MonitorScene::onSelectItemChanged()
{
    QList<QGraphicsItem *> sis= this->selectedItems();

    if(sis.length() > 2){
        emit sig_chooseChanged(nullptr);
    }

    if(sis.length() == 1)
    {
        QGraphicsItem *lastSelectItem = sis.last();

        //选择了一个站点
        if(lastSelectItem->type() == MonitorMapItemStation::Type || lastSelectItem->type() == MonitorMapItemStationName::Type)
        {
            MonitorMapItemStation *newStation = nullptr;
            if(lastSelectItem->type() == MonitorMapItemStation::Type){
                newStation = qgraphicsitem_cast<MonitorMapItemStation*>(lastSelectItem);

                foreach (auto name, iStationNames) {
                    if(name->getStation() == newStation){
                        name->setSelected(true);
                        break;
                    }
                }

            }else{
                MonitorMapItemStationName *stationName = qgraphicsitem_cast<MonitorMapItemStationName *>(lastSelectItem);
                newStation = stationName->getStation();
            }

            if(cur_tool == T_LINE){
                if(oldSelectStation==nullptr){
                    oldSelectStation = newStation;
                }else if(oldSelectStation!=newStation){

                    //判断是否已经有线路了
                    foreach (auto l, iLines) {
                        if(l->getPath()->getStart() == oldSelectStation->getPoint()->getId()
                                && l->getPath()->getEnd() == newStation->getPoint()->getId())
                        {
                            //两个站点之间已经有线路了
                            oldSelectStation = nullptr;
                            return ;
                        }
                    }
                    foreach (auto l, iQbs) {
                        if(l->getPath()->getStart() == oldSelectStation->getPoint()->getId()
                                && l->getPath()->getEnd() == newStation->getPoint()->getId())
                        {
                            //两个站点之间已经有线路了
                            oldSelectStation = nullptr;
                            return ;
                        }
                    }
                    foreach (auto l, iCbs) {
                        if(l->getPath()->getStart() == oldSelectStation->getPoint()->getId()
                                && l->getPath()->getEnd() == newStation->getPoint()->getId())
                        {
                            //两个站点之间已经有线路了
                            oldSelectStation = nullptr;
                            return ;
                        }
                    }


                    QString lineName = QString("%1 -- %2").arg(QString::fromStdString(oldSelectStation->getPoint()->getName())).arg(QString::fromStdString(newStation->getPoint()->getName()));

                    //对onemap添加线路数据
                    MapPath *p = new MapPath(onemap->getNextId(),lineName.toStdString(),oldSelectStation->getPoint()->getId(),newStation->getPoint()->getId(),MapPath::Map_Path_Type_Line,1);
                    onemap->addSpirit(p);
                    floor->addPath(p->getId());

                    //添加item
                    MonitorMapItemLine *line= new MonitorMapItemLine(oldSelectStation,newStation,p);
                    oldSelectStation->addLine(line);
                    newStation->addLine(line);
                    connect(line,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
                    addItem(line);
                    iLines.push_back(line);
                    update();

                    //发射信号
                    emit sig_add_remove_spirit();
                    newStation->setSelected(false);
                    oldSelectStation = nullptr;
                }
            }else if(cur_tool == T_QB){
                if(oldSelectStation==nullptr){
                    oldSelectStation = newStation;
                }else if(oldSelectStation!=newStation){
                    //判断是否已经有线路了
                    foreach (auto l, iLines) {
                        if((l->getPath()->getStart() == oldSelectStation->getPoint()->getId()
                            && l->getPath()->getEnd() == newStation->getPoint()->getId())||
                                (l->getPath()->getEnd() == oldSelectStation->getPoint()->getId()
                                 && l->getPath()->getStart() == newStation->getPoint()->getId()))
                        {
                            //两个站点之间已经有线路了
                            oldSelectStation = nullptr;
                            return ;
                        }
                    }
                    foreach (auto l, iQbs) {
                        if((l->getPath()->getStart() == oldSelectStation->getPoint()->getId()
                            && l->getPath()->getEnd() == newStation->getPoint()->getId())||
                                (l->getPath()->getEnd() == oldSelectStation->getPoint()->getId()
                                 && l->getPath()->getStart() == newStation->getPoint()->getId()))
                        {
                            //两个站点之间已经有线路了
                            oldSelectStation = nullptr;
                            return ;
                        }
                    }
                    foreach (auto l, iCbs) {
                        if((l->getPath()->getStart() == oldSelectStation->getPoint()->getId()
                            && l->getPath()->getEnd() == newStation->getPoint()->getId())||
                                (l->getPath()->getEnd() == oldSelectStation->getPoint()->getId()
                                 && l->getPath()->getStart() == newStation->getPoint()->getId()))
                        {
                            //两个站点之间已经有线路了
                            oldSelectStation = nullptr;
                            return ;
                        }
                    }
                    QString qbName = QString("%1 -- %2").arg(QString::fromStdString(oldSelectStation->getPoint()->getName())).arg(QString::fromStdString(newStation->getPoint()->getName()));
                    int cx = (oldSelectStation->getPoint()->getX()+newStation->getPoint()->getX())/2;
                    int cy = (oldSelectStation->getPoint()->getY()+newStation->getPoint()->getY())/2;
                    //对onemap添加线路数据
                    MapPath *p = new MapPath(onemap->getNextId(),qbName.toStdString(),oldSelectStation->getPoint()->getId(),newStation->getPoint()->getId(),MapPath::Map_Path_Type_Quadratic_Bezier,1,cx,cy);
                    onemap->addSpirit(p);
                    floor->addPath(p->getId());

                    //添加item
                    MonitorMapItemQuadraticBezier *qb= new MonitorMapItemQuadraticBezier(oldSelectStation,newStation,p);
                    oldSelectStation->addQb(qb);
                    newStation->addQb(qb);
                    connect(qb,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
                    addItem(qb);
                    iQbs.push_back(qb);
                    update();

                    //发射信号
                    emit sig_add_remove_spirit();
                    newStation->setSelected(false);
                    oldSelectStation = nullptr;
                }
            }else if(cur_tool == T_CB){
                if(oldSelectStation==nullptr){
                    oldSelectStation = newStation;
                }else if(oldSelectStation!=newStation){
                    //判断是否已经有线路了
                    foreach (auto l, iLines) {
                        if((l->getPath()->getStart() == oldSelectStation->getPoint()->getId()
                            && l->getPath()->getEnd() == newStation->getPoint()->getId())||
                                (l->getPath()->getEnd() == oldSelectStation->getPoint()->getId()
                                 && l->getPath()->getStart() == newStation->getPoint()->getId()))
                        {
                            //两个站点之间已经有线路了
                            oldSelectStation = nullptr;
                            return ;
                        }
                    }
                    foreach (auto l, iQbs) {
                        if((l->getPath()->getStart() == oldSelectStation->getPoint()->getId()
                            && l->getPath()->getEnd() == newStation->getPoint()->getId())||
                                (l->getPath()->getEnd() == oldSelectStation->getPoint()->getId()
                                 && l->getPath()->getStart() == newStation->getPoint()->getId()))
                        {
                            //两个站点之间已经有线路了
                            oldSelectStation = nullptr;
                            return ;
                        }
                    }
                    foreach (auto l, iCbs) {
                        if((l->getPath()->getStart() == oldSelectStation->getPoint()->getId()
                            && l->getPath()->getEnd() == newStation->getPoint()->getId())||
                                (l->getPath()->getEnd() == oldSelectStation->getPoint()->getId()
                                 && l->getPath()->getStart() == newStation->getPoint()->getId()))
                        {
                            //两个站点之间已经有线路了
                            oldSelectStation = nullptr;
                            return ;
                        }
                    }

                    QString qbName = QString("%1 -- %2").arg(QString::fromStdString(oldSelectStation->getPoint()->getName())).arg(QString::fromStdString(newStation->getPoint()->getName()));
                    int cx1 = oldSelectStation->getPoint()->getX()+(newStation->getPoint()->getX() - oldSelectStation->getPoint()->getX())/3;
                    int cy1 = oldSelectStation->getPoint()->getY()+(newStation->getPoint()->getY() - oldSelectStation->getPoint()->getY())/3;

                    int cx2 = oldSelectStation->getPoint()->getX()+(newStation->getPoint()->getX() - oldSelectStation->getPoint()->getX())*2/3;
                    int cy2 = oldSelectStation->getPoint()->getY()+(newStation->getPoint()->getY() - oldSelectStation->getPoint()->getY())*2/3;

                    //对onemap添加线路数据
                    MapPath *p = new MapPath(onemap->getNextId(),qbName.toStdString(),oldSelectStation->getPoint()->getId(),newStation->getPoint()->getId(),MapPath::Map_Path_Type_Cubic_Bezier,1,cx1,cy1,cx2,cy2);
                    onemap->addSpirit(p);
                    floor->addPath(p->getId());

                    //添加item
                    MonitorMapItemCubicBezier *cb= new MonitorMapItemCubicBezier(oldSelectStation,newStation,p);
                    oldSelectStation->addCb(cb);
                    newStation->addCb(cb);
                    connect(cb,SIGNAL(sig_propertyChanged(MapSpirit*)),this,SIGNAL(sig_propertyChanged(MapSpirit*)));
                    addItem(cb);
                    iCbs.push_back(cb);
                    update();

                    //发射信号
                    emit sig_add_remove_spirit();
                    newStation->setSelected(false);
                    oldSelectStation = nullptr;
                }
            }else if(cur_tool == T_ERASER ){
                //清除一个站点//TODO
                //要先清除，所有的相关线路
                QList<MonitorMapItemLine *> lines = newStation->getLines();
                foreach (auto line, lines) {
                    removeItem(line);
                    iLines.removeAll(line);
                    floor->removePath(line->getPath()->getId());
                    onemap->removeSpiritById(line->getPath()->getId());
                    update();

                }

                QList<MonitorMapItemCubicBezier *> cbs = newStation->getCbs();
                foreach (auto cb, cbs) {
                    removeItem(cb);
                    iCbs.removeAll(cb);
                    floor->removePath(cb->getPath()->getId());
                    onemap->removeSpiritById(cb->getPath()->getId());
                    update();
                }

                QList<MonitorMapItemQuadraticBezier *> qbs = newStation->getQbs();
                foreach (auto qb, qbs) {
                    removeItem(qb);
                    iQbs.removeAll(qb);
                    floor->removePath(qb->getPath()->getId());
                    onemap->removeSpiritById(qb->getPath()->getId());
                    update();
                }

                //然后清除站点
                MonitorMapItemStationName *stationname = nullptr;
                foreach (auto sn, iStationNames) {
                    if(sn->getStation() == newStation){
                        stationname = sn;
                        break;
                    }
                }
                assert(stationname != nullptr);
                removeItem(stationname);
                removeItem(newStation);
                iStations.removeAll(newStation);
                iStationNames.removeAll(stationname);
                floor->removePoint(newStation->getPoint()->getId());
                onemap->removeSpiritById(newStation->getPoint()->getId());

                update();
                emit sig_add_remove_spirit();
            }else{
                emit sig_chooseChanged(newStation->getPoint());
            }
        }

        //选择了一个线路
        else if(lastSelectItem->type() == MonitorMapItemLine::Type)
        {
            MonitorMapItemLine *selectLine = qgraphicsitem_cast<MonitorMapItemLine*>(lastSelectItem);
            if(cur_tool == T_ERASER ){
                //从它两头的站点中，将它移出
                selectLine->getStartStation()->removeLine(selectLine);
                selectLine->getEndStation()->removeLine(selectLine);
                removeItem(selectLine);
                iLines.removeAll(selectLine);
                floor->removePath(selectLine->getPath()->getId());
                onemap->removeSpiritById(selectLine->getPath()->getId());
                update();
                //TODO
                emit sig_add_remove_spirit();
            }else{
                emit sig_chooseChanged(selectLine->getPath());
            }
        }

        //选择了一条曲线
        else if(lastSelectItem->type() == MonitorMapItemQuadraticBezier::Type)
        {
            MonitorMapItemQuadraticBezier *selectLine = qgraphicsitem_cast<MonitorMapItemQuadraticBezier*>(lastSelectItem);
            if(cur_tool == T_ERASER ){
                //从它两头的站点中，将它移出
                selectLine->getStartStation()->removeQb(selectLine);
                selectLine->getEndStation()->removeQb(selectLine);
                removeItem(selectLine);
                iQbs.removeAll(selectLine);
                floor->removePath(selectLine->getPath()->getId());
                onemap->removeSpiritById(selectLine->getPath()->getId());
                update();
                //TODO:
                emit sig_add_remove_spirit();
            }else{
                emit sig_chooseChanged(selectLine->getPath());
            }
        }

        //选择了一条曲线
        else if(lastSelectItem->type() == MonitorMapItemCubicBezier::Type)
        {
            MonitorMapItemCubicBezier *selectLine = qgraphicsitem_cast<MonitorMapItemCubicBezier*>(lastSelectItem);
            if(cur_tool == T_ERASER ){
                //从它两头的站点中，将它移出
                selectLine->getStartStation()->removeCb(selectLine);
                selectLine->getEndStation()->removeCb(selectLine);
                removeItem(selectLine);
                iCbs.removeAll(selectLine);
                floor->removePath(selectLine->getPath()->getId());
                onemap->removeSpiritById(selectLine->getPath()->getId());
                update();
                //TODO:
                emit sig_add_remove_spirit();
            }else{
                emit sig_chooseChanged(selectLine->getPath());
            }
        }

        //选择了背景图片
        else if(lastSelectItem->type() == MonitorMapItemBkg::Type)
        {
            MonitorMapItemBkg *sekectBkg = qgraphicsitem_cast<MonitorMapItemBkg*>(lastSelectItem);
            if(cur_tool == T_ERASER ){
                removeItem(sekectBkg);
                floor->removeBkg();
                update();
                //TODO:
                emit sig_add_remove_spirit();
            }else{
                emit sig_chooseChanged(sekectBkg->getBkg());
            }
        }
    }
}


void MonitorScene::slot_selectItem(MapSpirit *_spirit)
{
    clearSelection();
    if(_spirit->getSpiritType() == MapSpirit::Map_Sprite_Type_Point)
    {
        foreach (auto staion, iStations) {
            if(staion->getPoint() == _spirit){
                staion->setSelected(true);;
                break;
            }
        }
    }
    else if(_spirit->getSpiritType() == MapSpirit::Map_Sprite_Type_Path)
    {
        MapPath *p = static_cast<MapPath *>(_spirit);

        if(p->getPathType() == MapPath::Map_Path_Type_Line){
            foreach (auto p, iLines) {
                if(p->getPath() == _spirit){
                    p->setSelected(true);
                    break;
                }
            }
        }else if(p->getPathType() == MapPath::Map_Path_Type_Quadratic_Bezier){
            foreach (auto p, iQbs) {
                if(p->getPath() == _spirit){
                    p->setSelected(true);
                    break;
                }
            }
        }else if(p->getPathType() == MapPath::Map_Path_Type_Cubic_Bezier){
            foreach (auto p, iCbs) {
                if(p->getPath() == _spirit){
                    p->setSelected(true);
                    break;
                }
            }
        }

    }
    else if(_spirit->getSpiritType() == MapSpirit::Map_Sprite_Type_Background){
        if(bkg!=nullptr && bkg->getBkg() == _spirit){
            bkg->setSelected(true);
        }
    }
}

