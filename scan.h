#ifndef SCAN_H
#define SCAN_H

#include <QDir>

class Scan : public QObject
{
     Q_OBJECT

signals:
    void send_file_info(QFileInfo file_info);
    void scan_finished();

public slots:
    void scan_directory(const QString &dir);
};

#endif // SCAN_H
