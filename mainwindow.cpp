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
}

MainWindow::~MainWindow()
{
    delete ui;
}
