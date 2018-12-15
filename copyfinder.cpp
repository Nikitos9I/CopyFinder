#include "copyfinder.h"
#include "scan.h"
#include "search.h"
#include "ui_copyfinder.h"

#include <QCommonStyle>
#include <QDirIterator>
#include <QCryptographicHash>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QThread>
#include <QDesktopServices>
#include <QDebug>

CopyFinder::CopyFinder(QWidget *parent) : QMainWindow(parent), ui(new Ui::CopyFinder) {
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    QCommonStyle style;
    ui->actionScan_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout_us->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));

    setup_treeWidget(1);

    connect(ui->actionScan_Directory, &QAction::triggered, this, &CopyFinder::select_directory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout_us, &QAction::triggered, this, &CopyFinder::show_about_dialog);
    connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(onItemClicked(QTreeWidgetItem*,int)));

    qRegisterMetaType<QDir>("QDir");
    qRegisterMetaType<QFileInfoList>("QFileInfoList");

    ui->findCopiesButton->setDisabled(true);
    ui->deleteCopiesButton->setDisabled(true);
    ui->cancelButton->setDisabled(true);
    ui->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void CopyFinder::select_directory() {
    setup_treeWidget(1);
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    createScanThread();
    globalDirectory = dir;
    _list.clear();
    scan_directory_main(dir);

    ui->findCopiesButton->setDisabled(false);
}

void CopyFinder::createScanThread() {
    timer.start();
    ui->cancelButton->setDisabled(false);
    ui->deleteCopiesButton->setDisabled(true);
    ui->statusBar->showMessage("Scannig directory ...");

    new_thread = new QThread;
    Scan * scan = new Scan;
    scan->moveToThread(new_thread);

    connect(new_thread, &QThread::finished, scan, &QObject::deleteLater);
    connect(this, &CopyFinder::scan_directory_main, scan, &Scan::scan_directory);
    connect(scan, &Scan::send_file_info, this, &CopyFinder::show_scanning_files);
    connect(scan, &Scan::scan_finished, this, &CopyFinder::killScanThread);

    new_thread->start();
}

void CopyFinder::killScanThread() {
    ui->statusBar->showMessage(QString("Scanning was finished in ") + QString::number(timer.elapsed() / 1000.0) + QString(" sec"));
    ui->cancelButton->setDisabled(true);
    ui->findCopiesButton->setDisabled(false);
}

void CopyFinder::createSearchThread() {
    timer.restart();
    ui->cancelButton->setDisabled(false);
    ui->deleteCopiesButton->setDisabled(true);
    ui->statusBar->showMessage("Finding copies ...");
    setup_treeWidget(2);

    new_thread = new QThread;
    Search * search = new Search;
    search->moveToThread(new_thread);

    connect(new_thread, &QThread::finished, search, &QObject::deleteLater);
    connect(this, &CopyFinder::find_copies_main, search, &Search::find_copies);
    connect(search, &Search::progress_value, this, &CopyFinder::listen_progress);
    connect(search, &Search::send_result, this, &CopyFinder::show_searching_files);
    connect(search, &Search::search_finished, this, &CopyFinder::killSearchThread);

    new_thread->start();
}

void CopyFinder::killSearchThread() {
    ui->statusBar->showMessage(QString("Finding was finished in ") + QString::number(timer.elapsed() / 1000.0) + QString(" sec"));
    ui->cancelButton->setDisabled(true);

    if (ui->treeWidget->topLevelItem(0) == nullptr) {
        no_copies();
    } else {
        ui->deleteCopiesButton->setDisabled(false);
    }
}

void CopyFinder::show_searching_files(QFileInfoList copies) {
    for (QFileInfo file_info : copies) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, file_info.dir().path());
        item->setText(1, file_info.fileName());
        item->setText(2, QString::number(file_info.size()) + QString(" b"));
        ui->treeWidget->addTopLevelItem(item);
    }
}

void CopyFinder::show_scanning_files(QFileInfo file_info) {
    renew_dir_list(file_info);

    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, file_info.fileName());
    item->setText(1, QString::number(file_info.size()) + QString(" b"));
    ui->treeWidget->addTopLevelItem(item);
}

void CopyFinder::setup_treeWidget(int level) {
    QStringList headerLabels;

    if (level == 1) {
        headerLabels.push_back(tr("Filename"));
        headerLabels.push_back(tr("Size"));
        ui->treeWidget->setColumnCount(headerLabels.count());
        ui->treeWidget->setHeaderLabels(headerLabels);
        ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    } else if (level == 2) {
        headerLabels.push_back(tr("Directory"));
        headerLabels.push_back(tr("FileName"));
        headerLabels.push_back(tr("Size"));
        ui->treeWidget->setColumnCount(headerLabels.count());
        ui->treeWidget->setHeaderLabels(headerLabels);
        ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::Stretch);
        ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    }

    ui->treeWidget->clear();
    listen_progress(0);
}

void CopyFinder::renew_dir_list(QFileInfo file_info) {
    this->_list.push_back(file_info);
}

void CopyFinder::on_findCopiesButton_clicked() {
    createSearchThread();
    find_copies_main(_list);
}

void CopyFinder::on_chooseDirectoryButton_clicked() {
    select_directory();
}

void CopyFinder::no_copies() {
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, "Found no copies");
    item->setFlags(Qt::ItemIsEnabled);
    ui->treeWidget->addTopLevelItem(item);
}

void CopyFinder::onItemClicked(QTreeWidgetItem *item, int column) {
    QString path = QDir(globalDirectory).absoluteFilePath(item->text(column));
//    qDebug() << path;
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void CopyFinder::listen_progress(int progress) {
    ui->progressBar->setValue(progress);
}

void CopyFinder::cancel_search() {
    new_thread->requestInterruption();
}

void CopyFinder::on_cancelButton_clicked() {
    cancel_search();
}

void CopyFinder::on_deleteCopiesButton_clicked() {
    deletE();
}

void CopyFinder::show_about_dialog() {
    QMessageBox::aboutQt(this);
}

void CopyFinder::deletE() {
    auto list = ui->treeWidget->selectedItems();

    if (list.size() == 0) {
        return;
    }

    QMessageBox dialog;
    dialog.setWindowTitle("Confirm");
    dialog.setText("Are you sure?");
    dialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    dialog.setDefaultButton(QMessageBox::No);

    if (dialog.exec() == QMessageBox::Yes) {
        int number = 0;
        for (auto it = list.begin(); it != list.end(); ++it) {
            QString path = (*it)->text(0) + "/" + (*it)->text(1);
            qDebug() << path;

            if (QFile::remove(path)) {
                ++number;
                (*it)->setSelected(false);
                (*it)->setDisabled(true);
//                delete *it;
            }
        }

        QString message = QString("In last time succesfully deleted / Errors   –   ") + QString::number(number) + QString(" / ") +
                QString::number(list.size() - number);

        ui->statusBar->showMessage(message);
    }
}

CopyFinder::~CopyFinder() {
    delete ui;
}
