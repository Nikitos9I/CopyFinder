#include "scan.h"

#include <QDirIterator>
#include <QThread>
#include <QTreeWidget>

void Scan::scan_directory(QString const& dir) {
    QDirIterator d(dir, QDir::Hidden | QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);

    while (d.hasNext()) {
        emit send_file_info(QFileInfo(d.next()));

        if (QThread::currentThread()->isInterruptionRequested()) {
            emit scan_finished();
            return;
        }
    }

    emit scan_finished();
}
