#ifndef RECENTFILE_H
#define RECENTFILE_H

#include <QWidget>
#include <QFileInfo>
#include <QDateTime>
#include "utils/WindowsUtils.h"

namespace Ui {
class RecentFile;
}

class RecentFile : public QWidget
{
    Q_OBJECT

public:
    explicit RecentFile(QWidget *parent = 0);
    ~RecentFile();
    void setFileName(QString fileName);
    void updateTime();

private:
    Ui::RecentFile *ui;

protected:
    QString fileName;
    QDateTime dateTime;
private slots:
	void on_pArrow_clicked();
};

#endif // RECENTFILE_H
