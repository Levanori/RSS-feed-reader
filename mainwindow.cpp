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

    connect(net, &network_access::newsSend, this, &MainWindow::newsReceived);
    connect(ui->NewsTextTable, &QTableWidget::itemDoubleClicked, this, &MainWindow::openLink);
    connect(net, &network_access::imageDownloaded, this, &MainWindow::imagePlace);
    connect(ui->pushButtonAddRSS, &QPushButton::clicked, this, &MainWindow::addSiteByUser);
    connect(ui->treeWidgetOfRSS, &QTreeWidget::itemClicked, this, &MainWindow::treeRSSClicked);

    ui->NewsTextTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->treeWidgetOfRSS->hideColumn(1);

    loadSites();
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

void MainWindow::addSiteByUser() {
    QString name = ui->lineEditNameOfSite->text();
    QString url = ui->lineEditUrl->text();

    if (!name.isEmpty() && !url.isEmpty()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidgetOfRSS);
        item->setText(0, name);
        item->setText(1, url);

        QFile fileOfSites("user_sites.txt");
        if (fileOfSites.open(QIODevice::Append | QIODevice::Text)) { // opportuniry to continue writing text
            fileOfSites.write(name.toUtf8() + ";" + url.toUtf8() + "\n");
            fileOfSites.close();
            qDebug() << "Збережено";}

        ui->lineEditNameOfSite->clear();
        ui->lineEditUrl->clear();
    }
}

void MainWindow::loadSites() {
    readAndAddToTree(":/configs/default_sites.txt");
    readAndAddToTree("user_sites.txt");
}

void MainWindow::readAndAddToTree(QString fileName) {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        QString allNamesAndUrs = QString::fromUtf8(data);
        QStringList listOfNamesAndUrs = allNamesAndUrs.split('\n');

        for (int index = 0; index < listOfNamesAndUrs.size(); index++) {
            QString nameAndUrs = listOfNamesAndUrs[index].replace("\r", "");
            if (nameAndUrs.size()) {
                QStringList partsOfNameAndURL = nameAndUrs.split(';');
                if (partsOfNameAndURL.size() == 2) {
                    QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidgetOfRSS);
                    item->setText(0, partsOfNameAndURL[0]);
                    item->setText(1, partsOfNameAndURL[1]);
                }
            }
        }
    }
}

void MainWindow::treeRSSClicked(QTreeWidgetItem *item) {
    QString url = item->text(1);
    ui->NewsTextTable->setRowCount(0);
    imageMap.clear();
    net->getRSS(url);
}

