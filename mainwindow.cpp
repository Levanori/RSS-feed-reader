#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setMinimumSize(960, 313);
    this->showMaximized();

    net = new network_access(this);
    net->getRSS();

    connect(net, &network_access::newsSend, this, &MainWindow::newsReceived);
    connect(ui->NewsTextTable, &QTableWidget::itemDoubleClicked, this, &MainWindow::openLink);
    connect(net, &network_access::imageDownloaded, this, &MainWindow::imagePlace);

    ui->NewsTextTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newsReceived(QString category, QString title, QString date, QString description, QString link, QString imageUrl)
{
    int row = ui->NewsTextTable->rowCount();
    ui->NewsTextTable->insertRow(row);

    QLabel *imageLabel = new QLabel();
    imageLabel->setScaledContents(true);
    imageLabel->setFixedSize(150, 120);

    ui->NewsTextTable->setItem(row, 0, new QTableWidgetItem(date));
    ui->NewsTextTable->setItem(row, 1, new QTableWidgetItem(category));
    ui->NewsTextTable->setItem(row, 2, new QTableWidgetItem(title));
    ui->NewsTextTable->setItem(row, 3, new QTableWidgetItem(description));
    ui->NewsTextTable->setCellWidget(row, 4, imageLabel);
    ui->NewsTextTable->setItem(row, 5, new QTableWidgetItem(link));

    if (!imageUrl.isEmpty()) {
        imageMap[imageUrl] = imageLabel;
        net->downloadImage(imageUrl);
    }
}

void MainWindow::openLink(QTableWidgetItem *item)
{
    QString link = ui->NewsTextTable->item(item->row(), 5)->text();
    QDesktopServices::openUrl(QUrl(link));
}

void MainWindow::imagePlace(QByteArray data, QString url) {
    if(imageMap.contains(url)) {
        QPixmap photo;
        photo.loadFromData(data);
        imageMap[url]->setPixmap(photo);
    }
    imageMap.remove(url);
}

