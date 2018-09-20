﻿#include "dialogaddtask.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include "global.h"
#include "common.h"

DialogAddTask::DialogAddTask(QWidget *parent) : QDialog(parent)
{
    //////////////////////base
    agvLabel = new QLabel(QStringLiteral("指定AGV:"));
    agvCombobox = new QComboBox;
    agvCombobox->addItem(QStringLiteral("不指定"));
    priorityLabel = new QLabel(QStringLiteral("优先级:"));
    priorityCombobox = new QComboBox;
    priorityCombobox->addItem(QStringLiteral("最低"));
    priorityCombobox->addItem(QStringLiteral("较低"));
    priorityCombobox->addItem(QStringLiteral("普通"));
    priorityCombobox->addItem(QStringLiteral("较高"));
    priorityCombobox->addItem(QStringLiteral("最高"));
    priorityCombobox->setCurrentIndex(2);

    runtimeLabel = new QLabel(QStringLiteral("循环次数:"));
    runtimeInput = new QSpinBox;
    runtimeInput->setRange(1,100);
    runtimeInput->setValue(1);

    baseGroupbox = new QGroupBox("base");

    QHBoxLayout *h1 = new QHBoxLayout;
    QHBoxLayout *h2 = new QHBoxLayout;
    QHBoxLayout *h3 = new QHBoxLayout;
    QVBoxLayout *v1 = new QVBoxLayout;

    h1->addWidget(agvLabel);
    h1->addWidget(agvCombobox);
    h2->addWidget(priorityLabel);
    h2->addWidget(priorityCombobox);
    h3->addWidget(runtimeLabel);
    h3->addWidget(runtimeInput);
    v1->addItem(h1);
    v1->addItem(h2);
    v1->addItem(h3);

    baseGroupbox->setLayout(v1);

    //////////////////////extra param
    extraGroupbox = new QGroupBox("extra");
    extraTable = new QTableWidget(0,2);
    extraTable->horizontalHeader()->hide();
    extraTable->verticalHeader()->hide();
    //extraTable->setShowGrid(false);
    extraTable->setContextMenuPolicy(Qt::CustomContextMenu);
    extraTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);    //x先自适应宽度
    extraTable->setSelectionBehavior(QAbstractItemView::SelectRows);//选中就是选中这一行
    extraTable->setSelectionMode(QAbstractItemView::SingleSelection);//只能选中一行
    //extraTable->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑

    QPushButton *addParamBtn = new QPushButton("add param");
    QPushButton *removeParamBtn = new QPushButton("remove param");
    QHBoxLayout *parambtnLayout = new QHBoxLayout;
    parambtnLayout->addWidget(addParamBtn);
    parambtnLayout->addWidget(removeParamBtn);
    QVBoxLayout *paramGroupLayout = new QVBoxLayout;
    paramGroupLayout->addItem(parambtnLayout);
    paramGroupLayout->addWidget(extraTable);
    connect(addParamBtn,SIGNAL(clicked(bool)),this,SLOT(addParam()));
    connect(removeParamBtn,SIGNAL(clicked(bool)),this,SLOT(removeParam()));
    extraGroupbox->setLayout(paramGroupLayout);



    //////////////////////node
    nodeTable = new QTableWidget(0,3);
    nodeTable->horizontalHeader()->hide();
    nodeTable->verticalHeader()->hide();
    //nodeTable->setShowGrid(false);
    nodeTable->setContextMenuPolicy(Qt::CustomContextMenu);
    nodeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);    //x先自适应宽度
    nodeTable->setSelectionBehavior(QAbstractItemView::SelectRows);//选中就是选中这一行
    nodeTable->setSelectionMode(QAbstractItemView::SingleSelection);//只能选中一行
    //nodeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑

    nodeGroupbox = new QGroupBox("nodes");
    QPushButton *addNodeBtn = new QPushButton("add node");
    QPushButton *removeNodeBtn = new QPushButton("remove node");
    QPushButton *addNodeFromFileBtn = new QPushButton("add from file");
    QHBoxLayout *nodebtnLayout = new QHBoxLayout;
    nodebtnLayout->addWidget(addNodeBtn);
    nodebtnLayout->addWidget(removeNodeBtn);
    nodebtnLayout->addWidget(addNodeFromFileBtn);
    QVBoxLayout *nodeGroupLayout = new QVBoxLayout;
    nodeGroupLayout->addItem(nodebtnLayout);
    nodeGroupLayout->addWidget(nodeTable);
    connect(addNodeBtn,SIGNAL(clicked(bool)),this,SLOT(addNode()));
    connect(removeNodeBtn,SIGNAL(clicked(bool)),this,SLOT(removeNode()));
    connect(addNodeFromFileBtn,SIGNAL(clicked(bool)),this,SLOT(addNodeFromFile()));
    nodeGroupbox->setLayout(nodeGroupLayout);

    tipLabel = new QLabel("");
    okBtn = new QPushButton(QStringLiteral("添加"));
    cancelBtn = new QPushButton(QStringLiteral("取消"));

    QHBoxLayout *hboxLayout = new QHBoxLayout;
    hboxLayout->addWidget(okBtn);
    hboxLayout->addWidget(cancelBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(baseGroupbox);
    mainLayout->addWidget(extraGroupbox);
    mainLayout->addWidget(nodeGroupbox);
    mainLayout->addItem(hboxLayout);
    mainLayout->addWidget(tipLabel);

    setLayout(mainLayout);

    connect(okBtn,SIGNAL(clicked(bool)),this,SLOT(onOkBtn()));
    connect(cancelBtn,SIGNAL(clicked(bool)),this,SLOT(close()));

    //TODO:
    connect(&msgCenter,SIGNAL(tip(QString)),tipLabel,SLOT(setText(QString)));
    connect(&msgCenter,SIGNAL(addTaskSuccess()),this,SLOT(accept()));
    connect(&msgCenter,SIGNAL(mapGetSuccess()),this,SLOT(stationsUpdate()));

    agvsUpdate();
    stationsUpdate();
}

void DialogAddTask::onOkBtn()
{
    if(nodeTable->rowCount() == 0){
        QMessageBox::warning(this,QStringLiteral("node is null"),QStringLiteral("node is null"));
        return ;
    }

    //TODO组织所有的参数
    //优先级
    int priority = priorityCombobox->currentIndex() - 1;
    //指定agv
    int agv = -1;
    if(agvCombobox->currentIndex() > 0){
        agv = this->agvinfos[agvCombobox->currentIndex() - 1].id;
    }

    int runTimes = runtimeInput->value();

    //params
    QMap<QString,QString> params;
    for(int row=0;row<extraTable->rowCount();++row){
        if(extraTable->item(row,0)->text().trimmed().length()<=0)continue;
        params.insert(extraTable->item(row,0)->text().trimmed(),extraTable->item(row,1)->text().trimmed());
    }

    QList<TaskNode> nodes;
    for(int row=0;row<nodeTable->rowCount();++row){
        QWidget * stationwidget=nodeTable->cellWidget(row,0);
        QWidget * dowhatwidget=nodeTable->cellWidget(row,1);
        QWidget * paramwidget=nodeTable->cellWidget(row,2);
        QComboBox *stationCombobox=(QComboBox*)stationwidget;
        QComboBox *dowhatCombobox=(QComboBox*)dowhatwidget;
        QLineEdit *paraminput=(QLineEdit*)paramwidget;

        TaskNode node;
        if(stationCombobox->currentIndex()==0){
            node.stationid = -1;
        }else{
            node.stationid = this->stations[stationCombobox->currentIndex()-1]->getId();
        }

        node.dowhat = dowhatCombobox->currentIndex();

        QString paramstr = paraminput->text();
        if(paramstr.trimmed().length()>0){
            node.params = paramstr.trimmed().split(";");
        }
        nodes<<node;
    }

    //TODO
    msgCenter.addTask(priority,agv,runTimes,params,nodes);

    accept();
}

void DialogAddTask::addNode()
{
    nodeTable->insertRow(nodeTable->rowCount());

    //添加三个
    QComboBox *stationSelect = new QComboBox;
    stationSelect->addItem(QStringLiteral("无"));

    for(auto station:stations){
        stationSelect->addItem(QString::fromStdString(station->getName()));
    }

    QComboBox *doWhatSelect = new QComboBox;
    doWhatSelect->addItem("pick");//0
    doWhatSelect->addItem("put");//1
    doWhatSelect->addItem("go charge");//2
    doWhatSelect->addItem("move");//3

    QLineEdit *paramInput = new QLineEdit;
    paramInput->setPlaceholderText(QStringLiteral("参数列表以;区分"));
    nodeTable->setCellWidget(nodeTable->rowCount()-1,0,stationSelect);
    nodeTable->setCellWidget(nodeTable->rowCount()-1,1,doWhatSelect);
    nodeTable->setCellWidget(nodeTable->rowCount()-1,2,paramInput);
}

void DialogAddTask::addNodeFromFile()
{
    QFile taskfile("task.txt");
    if (!taskfile.open(QIODevice::ReadOnly|QIODevice::Text)) {
        return;
    }
    QTextStream txtInput(&taskfile);
    while(!txtInput.atEnd())
    {
        QStringList task_line = txtInput.readLine().split(" ");
        if(task_line.size() < 2)
            continue;
        nodeTable->insertRow(nodeTable->rowCount());

        //添加三个
        QComboBox *stationSelect = new QComboBox;
        stationSelect->addItem(QStringLiteral("无"));

        for(auto station:stations){
            stationSelect->addItem(QString::fromStdString(station->getName()));
        }

        stationSelect->setCurrentText(task_line.at(1));

        QComboBox *doWhatSelect = new QComboBox;
        doWhatSelect->addItem("pick");//0
        doWhatSelect->addItem("put");//1
        doWhatSelect->addItem("go charge");//2
        doWhatSelect->addItem("move");//3
        doWhatSelect->setCurrentText(task_line.at(0));

        QLineEdit *paramInput = new QLineEdit;
        paramInput->setPlaceholderText(QStringLiteral("参数列表以;区分"));
        if(task_line.size() > 2)
        {
            paramInput->setText(task_line.at(2));
        }
        nodeTable->setCellWidget(nodeTable->rowCount()-1,0,stationSelect);
        nodeTable->setCellWidget(nodeTable->rowCount()-1,1,doWhatSelect);
        nodeTable->setCellWidget(nodeTable->rowCount()-1,2,paramInput);
    }
    taskfile.close();

}

void DialogAddTask::removeNode()
{
    //    int kk = nodeTable->currentRow();
    //    QTableWidgetItem *tt = nodeTable->currentItem();
    //    if(kk>=0){
    //        nodeTable->removeRow(kk );
    //    }else{
    //        QMessageBox::warning(this,QStringLiteral("未选择行"),QStringLiteral("未选择行"));
    //    }

    if(nodeTable->rowCount()>0){
        nodeTable->removeRow(0);
    }
}

void DialogAddTask::addParam()
{
    extraTable->insertRow(extraTable->rowCount());

    QLineEdit *paramKey = new QLineEdit;
    paramKey->setPlaceholderText("key");

    QLineEdit *paramValue = new QLineEdit;
    paramValue->setPlaceholderText("value");

    extraTable->setCellWidget(nodeTable->rowCount()-1,0,paramKey);
    extraTable->setCellWidget(nodeTable->rowCount()-1,1,paramValue);
}

void DialogAddTask::removeParam()
{
    int kk = extraTable->currentRow();
    if(kk>=0){
        extraTable->removeRow(kk );
    }else{
        QMessageBox::warning(this,QStringLiteral("未选择行"),QStringLiteral("未选择行"));
    }
}

void DialogAddTask::agvsUpdate()
{
    int currentIndex = agvCombobox->currentIndex();
    agvinfos = msgCenter.getAgvListModel();
    agvCombobox->clear();
    agvCombobox->addItem(QStringLiteral("不指定"));
    for(auto agvinfo:agvinfos){
        agvCombobox->addItem(agvinfo.name);
    }
    if(agvCombobox->count()>currentIndex){
        agvCombobox->setCurrentIndex(currentIndex);
    }
}

void DialogAddTask::stationsUpdate()
{
    stations.clear();
    auto ae = g_onemap.getAllElement();
    for(auto e:ae){
        if(e->getSpiritType() == MapSpirit::Map_Sprite_Type_Point){
            MapPoint *pp = static_cast<MapPoint *>(e);
            stations.push_back(pp);
        }
    }
}
