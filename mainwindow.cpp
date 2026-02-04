#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setMinimumSize(960, 313);
    this->showMaximized();

    feedTimer = new QTimer(this);
    net = new network_access(this);
    rssStorage = new rss_storage(ui->treeWidgetOfRSS);

    connect(net, &network_access::newsSend, this, &MainWindow::newsReceived);
    connect(ui->NewsTextTable, &QTableWidget::itemDoubleClicked, this, &MainWindow::openLink);
    connect(net, &network_access::imageDownloaded, this, &MainWindow::imagePlace);
    connect(ui->pushButtonAddRSS, &QPushButton::clicked, this, &MainWindow::addSiteByUser);
    connect(ui->treeWidgetOfRSS, &QTreeWidget::itemClicked, this, &MainWindow::treeRSSClicked);
    connect(ui->pushButtonDeleteRSS, &QPushButton::clicked, this, &MainWindow::deleteSiteByUser);
    connect(feedTimer, &QTimer::timeout, this, &MainWindow::refreshCurrentSelection);
    connect(ui->boxForTimer, &QComboBox::currentIndexChanged, this, &MainWindow::timerConfigChanged);

    ui->NewsTextTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->treeWidgetOfRSS->hideColumn(1);
    ui->boxForTimer->setItemData(0, 0);
    ui->boxForTimer->setItemData(1, 60000);
    ui->boxForTimer->setItemData(2, 300000);
    ui->boxForTimer->setItemData(3, 900000);
    ui->boxForTimer->setItemData(4, 1800000);

    rssStorage->loadSites();
}

MainWindow::~MainWindow()
{
    delete rssStorage;
    delete ui;
}

void MainWindow::newsReceived(QString category, QString title, QString date, QString description, QString link, QString imageUrl)
{
    for (int row = 0; row < ui->NewsTextTable->rowCount(); row++) {
        if (ui->NewsTextTable->item(row, 5)->text() == link) {
            return;
        }
    }

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

    if (folder == "Усі новини" || name == "Усі новини") {
        QMessageBox::warning(this, "Warning", "Не можна використовувати 'Усі новини' як назву");
        return;
    }

    if (url.isEmpty() || name.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Введіть хоча б назву сайту та URL для додавання");
        return;
    }
    if (rssStorage->addRssToTree(folder, name, url)) {
        rssStorage->saveAllSites("user_sites.txt");
        // qDebug() << "Збережено";

        ui->lineEditFolder->clear();
        ui->lineEditNameOfSite->clear();
        ui->lineEditUrl->clear();
    }
}

void MainWindow::treeRSSClicked(QTreeWidgetItem *item) {
    if (item->text(1) == "all_news") {
        ui->NewsTextTable->setRowCount(0);
        refreshCurrentSelection();
        return;
    }
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

void MainWindow::deleteSiteByUser() {
    QString folder = ui->lineEditFolder->text();
    QString name = ui->lineEditNameOfSite->text();
    QString url = ui->lineEditUrl->text();

    if (name == "Усі новини") {
        QMessageBox::warning(this, "Warning", "Не можна видалити 'Усі новини'");
        return;
    }

    int count = 0;
    if (!folder.isEmpty()) {
        count++;
    }
    if (!name.isEmpty()) {
        count++;
    }
    if (!url.isEmpty()) {
        count++;
    }
    if (count == 0) {
        QMessageBox::warning(this, "Warning", "Введіть тільки назву папки, або сайту, або URL для видалення");
        return;
    }
    if (count > 1) {
        QMessageBox::warning(this, "Warning", "Заповніть лише одне поле");
        return;
    }
    if (!rssStorage->deleteRss(folder, name, url)) {
        QMessageBox::warning(this, "Warning", "Нічого не знайдено за запитом");
        return;
    }

    ui->lineEditFolder->clear();
    ui->lineEditNameOfSite->clear();
    ui->lineEditUrl->clear();
    ui->NewsTextTable->setRowCount(0);
}

void MainWindow::refreshAllFeeds()
{
    for (int indexOfItem = 0; indexOfItem < ui->treeWidgetOfRSS->topLevelItemCount(); indexOfItem++) {
        QTreeWidgetItem *topItem = ui->treeWidgetOfRSS->topLevelItem(indexOfItem);

        if (topItem->text(1) == "all_news") {
            continue;
        }

        if (topItem->childCount() > 0) {
            for (int indexItemInFolder = 0; indexItemInFolder < topItem->childCount(); indexItemInFolder++) {
                QString url = topItem->child(indexItemInFolder)->text(1);
                if (!url.isEmpty()) {
                    qDebug() << "Оновлюється сайт із папки:" << topItem->child(indexItemInFolder)->text(0);
                    net->getDataFromInternet(url);
                }
            }
        }
        else if (!topItem->text(1).isEmpty()) {
            qDebug() << "Оновлюється одиночний сайт:" << topItem->text(0);
            net->getDataFromInternet(topItem->text(1));
        }
    }
}

void MainWindow::refreshCurrentSelection() {
    QTreeWidgetItem *currentItem = ui->treeWidgetOfRSS->currentItem();
    QString url = currentItem->text(1);

    if (url == "all_news") {
        refreshAllFeeds();
    }
    else if (url.isEmpty()) {
        for (int indexItemInFolder = 0; indexItemInFolder < currentItem->childCount(); indexItemInFolder++) {
            QString childUrl = currentItem->child(indexItemInFolder)->text(1);
            if (!childUrl.isEmpty()) {
                qDebug() << "Оновлюється сайт із папки:" << currentItem->child(indexItemInFolder)->text(0);
                net->getDataFromInternet(childUrl);
            }
        }
    }
    else {
        qDebug() << "Оновлюється одиночний сайт:" << currentItem->text(0);
        net->getDataFromInternet(url);
    }
}

void MainWindow::timerConfigChanged() {
    feedTimer->stop();

    int interval = ui->boxForTimer->currentData().toInt();
    if (interval > 0) {
        feedTimer->start(interval);
        qDebug() << "Таймер успішно запущено на" << interval/60000 << "хв";
    }
    else {
        qDebug() << "Таймер зупинено";
    }
}


