#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setMinimumSize(960, 313);
    this->showMaximized();

    net = new network_access(this);
    rssStorage = new rss_storage(ui->treeWidgetOfRSS);

    connect(net, &network_access::newsSend, this, &MainWindow::newsReceived);
    connect(ui->NewsTextTable, &QTableWidget::itemDoubleClicked, this, &MainWindow::openLink);
    connect(net, &network_access::imageDownloaded, this, &MainWindow::imagePlace);
    connect(ui->pushButtonAddRSS, &QPushButton::clicked, this, &MainWindow::addSiteByUser);
    connect(ui->treeWidgetOfRSS, &QTreeWidget::itemClicked, this, &MainWindow::treeRSSClicked);

    ui->NewsTextTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->treeWidgetOfRSS->hideColumn(1);

    rssStorage->loadSites();
}

MainWindow::~MainWindow()
{
    delete rssStorage;
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
        imageMap.insert(imageUrl, imageLabel);
        net->getDataFromInternet(imageUrl);
    }

    ui->NewsTextTable->sortItems(0, Qt::DescendingOrder); // sort by time excluding GMT
}

void MainWindow::openLink(QTableWidgetItem *item)
{
    QString link = ui->NewsTextTable->item(item->row(), 5)->text();
    QDesktopServices::openUrl(QUrl(link));
}

void MainWindow::imagePlace(QByteArray data, QString url) {
    QPixmap photo;
    if (photo.loadFromData(data)) {
        while (imageMap.contains(url)) {
            QLabel *labelForPhoto = imageMap.value(url);
            if (labelForPhoto) {
                labelForPhoto->setPixmap(photo);
            }
            imageMap.remove(url, labelForPhoto);
        }
    } else {
        imageMap.remove(url);
    }
}

void MainWindow::addSiteByUser() {
    QString folder = ui->lineEditFolder->text();
    QString name = ui->lineEditNameOfSite->text();
    QString url = ui->lineEditUrl->text();

    if (rssStorage->addRssToTree(folder, name, url)) {
        rssStorage->saveAllSites("user_sites.txt");
        // qDebug() << "Збережено";

        ui->lineEditFolder->clear();
        ui->lineEditNameOfSite->clear();
        ui->lineEditUrl->clear();
    }
}

void MainWindow::treeRSSClicked(QTreeWidgetItem *item) {
    QString url = item->text(1);
    ui->NewsTextTable->setRowCount(0);

    if (url.isEmpty()) { // folder
        for (int indexItemInFolder = 0; indexItemInFolder < item->childCount(); indexItemInFolder++) {
            QString childUrl = item->child(indexItemInFolder)->text(1);
            if (!childUrl.isEmpty()) {
                net->getDataFromInternet(childUrl);
            }
        }
    }
    else {
        net->getDataFromInternet(url);
    }
}
