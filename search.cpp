#include "search.h"

#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QThread>
#include <QTreeWidget>
#include <QDebug>

void Search::find_copies(QFileInfoList list) {
    emit progress_value(0);

    QMap<qint64, QFileInfoList> sortedFilesGroups;

    for (QFileInfo file_info : list) {
        auto iter = sortedFilesGroups.find(file_info.size());
        if (iter == sortedFilesGroups.end()) {
            QFileInfoList currentVector;
            currentVector.push_back(file_info);
            sortedFilesGroups.insert(file_info.size(), currentVector);
        } else {
            iter->push_back(file_info);
        }

        if (QThread::currentThread()->isInterruptionRequested()) {
            emit search_finished();
            return;
        }
    }

    int percent = 1;
    QCryptographicHash sha(QCryptographicHash::Sha3_256);
    for (auto it = sortedFilesGroups.begin(); it != sortedFilesGroups.end(); ++it) {
        if (it->size() <= 1) {
            continue;
        }

        QMap<QByteArray, QFileInfoList> hashes;
        for (QFileInfo file_info : *it) {
            sha.reset();
            QFile file(file_info.filePath());

//            qDebug() << file_info.path() + "/" + file_info.fileName();

            if (file.open(QIODevice::ReadOnly)) {
                sha.addData(&file);
            }

            QByteArray res = sha.result();
            auto st = hashes.find(res);
            if (st != hashes.end()) {
                st->push_back(file_info);
            } else {
                QFileInfoList temp;
                temp.push_back(file_info);
                hashes.insert(res, temp);
            }

            if (QThread::currentThread()->isInterruptionRequested()) {
                emit search_finished();
                return;
            }
        }

        for (auto st = hashes.begin(); st != hashes.end(); ++st) {
            if (st->size() > 1) {
                emit send_result(*st);
            }
        }

        emit progress_value(100 * ++percent / sortedFilesGroups.size());

        if (QThread::currentThread()->isInterruptionRequested()) {
            emit search_finished();
            return;
        }
    }

    emit progress_value(100);
    emit search_finished();
}
