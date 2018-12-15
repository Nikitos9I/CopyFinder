#ifndef COPYFINDER_H
#define COPYFINDER_H

#include <QFileInfoList>
#include <QMainWindow>
#include <QThread>
#include <QTime>
#include <QTreeWidgetItem>

namespace Ui {
class CopyFinder;
}

class CopyFinder : public QMainWindow
{
    Q_OBJECT

public:
    explicit CopyFinder(QWidget *parent = nullptr);
    QThread * new_thread;
    ~CopyFinder();

public slots:
    void renew_dir_list(QFileInfo file_info);
    void show_scanning_files(QFileInfo file_info);
    void show_searching_files(QFileInfoList copies);
    void select_directory();
    void show_about_dialog();
    void on_chooseDirectoryButton_clicked();
    void on_findCopiesButton_clicked();
    void on_cancelButton_clicked();
    void listen_progress(int progress);
    void no_copies();
    void cancel_search();
    void deletE();

signals:
    void scan_directory_main(const QString &dir);
    void find_copies_main(QFileInfoList list);

private slots:
    void onItemClicked(QTreeWidgetItem *item, int column);

    void on_deleteCopiesButton_clicked();

private:
    Ui::CopyFinder * ui;
    QFileInfoList _list;
    QString globalDirectory;
    QTime timer;

    void setup_treeWidget(int level);
    void createScanThread();
    void killScanThread();
    void createSearchThread();
    void killSearchThread();
};

#endif // COPYFINDER_H
