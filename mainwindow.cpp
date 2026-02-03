#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopServices>
#include <QMessageBox>

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
        imageMap.insert(imageUrl, imageLabel);
        net->getDataFromInternet(imageUrl);
    }
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

    if (addRssToTree(folder, name, url)) {
        QFile fileOfSites("user_sites.txt");
        if (fileOfSites.open(QIODevice::Append | QIODevice::Text)) { // opportuniry to continue writing text
            fileOfSites.write(folder.toUtf8() + ";" + name.toUtf8() + ";" + url.toUtf8() + "\n");
            fileOfSites.close();
            // qDebug() << "Збережено";
        }

        ui->lineEditFolder->clear();
        ui->lineEditNameOfSite->clear();
        ui->lineEditUrl->clear();
    }
}

void MainWindow::loadSites() {
    loadTheTree(":/configs/default_sites.txt");
    loadTheTree("user_sites.txt");
}

void MainWindow::loadTheTree(QString fileName) {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();

        QString readableData = QString::fromUtf8(data);
        QStringList listOfItemsForTree = readableData.split('\n');

        for (int index = 0; index < listOfItemsForTree.size(); index++) {
            QString itemForTree = listOfItemsForTree[index].replace("\r", "");
            if (itemForTree.size() > 1) {
                QStringList endListOfItemsForTree = itemForTree.split(';');

                if (endListOfItemsForTree.size() == 3) {
                    addRssToTree(endListOfItemsForTree[0], endListOfItemsForTree[1], endListOfItemsForTree[2]);
                }
                else if (endListOfItemsForTree.size() == 2) {
                    addRssToTree("", endListOfItemsForTree[0], endListOfItemsForTree[1]);
                }
            }
        }
    }
}

void MainWindow::treeRSSClicked(QTreeWidgetItem *item) {
    QString url = item->text(1);

    if (url.isEmpty()) { // folder
        return;
    }

    ui->NewsTextTable->setRowCount(0);
    imageMap.clear();
    net->getDataFromInternet(url);
}

bool MainWindow::addRssToTree(QString folder, QString name, QString url) {
    if (name.isEmpty() || url.isEmpty()) {
        return 0;
    }

    for (int indexOfItem = 0; indexOfItem < ui->treeWidgetOfRSS->topLevelItemCount(); indexOfItem++) { // rss name and link identity check
        QTreeWidgetItem *topItem = ui->treeWidgetOfRSS->topLevelItem(indexOfItem);
        bool errorDuplicate = 0;

        if (topItem->text(1) == url || topItem->text(0) == name) {
            errorDuplicate = 1;
        }
        for (int indexItemInFolder = 0; indexItemInFolder < topItem->childCount(); indexItemInFolder++) {
            if (topItem->child(indexItemInFolder)->text(1) == url || topItem->child(indexItemInFolder)->text(0) == name) {
                errorDuplicate = 1;
            }
        }
        if (errorDuplicate) {
            QMessageBox::warning(this, "Warning!", "Виявлено дублювання назви чи RSS стрічки");
            return 0;
        }
    }

    if (folder.size() > 0) {
        QTreeWidgetItem *foundFolder = 0;

        for (int index = 0; index < ui->treeWidgetOfRSS->topLevelItemCount(); index++) {
            if (ui->treeWidgetOfRSS->topLevelItem(index)->text(0) == folder) {
                foundFolder = ui->treeWidgetOfRSS->topLevelItem(index);
                break;
            }
        }

        if (foundFolder == 0) {
            foundFolder = new QTreeWidgetItem(ui->treeWidgetOfRSS);
            foundFolder->setText(0, folder);

        }
        QTreeWidgetItem *siteItem = new QTreeWidgetItem(foundFolder);
        siteItem->setText(0, name);
        siteItem->setText(1, url);
    }
    else {
        QTreeWidgetItem *siteItem = new QTreeWidgetItem(ui->treeWidgetOfRSS);
        siteItem->setText(0, name);
        siteItem->setText(1, url);
    }
    return 1;
}
