#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "network_access.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    network_access *net = new network_access(this);
    net->getRSS();

    connect(net, &network_access::newsSend, this, &MainWindow::newsReceived);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newsReceived(QString category, QString title, QString date, QString description)
{
    int row = ui->NewsTextTable->rowCount();
    ui->NewsTextTable->insertRow(row);

    ui->NewsTextTable->setItem(row, 0, new QTableWidgetItem(date));
    ui->NewsTextTable->setItem(row, 1, new QTableWidgetItem(category));
    ui->NewsTextTable->setItem(row, 2, new QTableWidgetItem(title));
    ui->NewsTextTable->setItem(row, 3, new QTableWidgetItem(description));
}
