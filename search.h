#ifndef SCANANDSEARCH_H
#define SCANANDSEARCH_H

#include <QFileInfoList>
#include <QString>

class Search : public QObject
{
     Q_OBJECT

signals:
    void progress_value(int value);
    void send_result(QFileInfoList copies);
    void search_finished();

public slots:
    void find_copies(QFileInfoList);
};

#endif // SCANANDSEARCH_H
