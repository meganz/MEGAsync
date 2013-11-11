#ifndef RECENTFILE_H
#define RECENTFILE_H

#include <QWidget>

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
private:
    Ui::RecentFile *ui;

protected:
    QString fileName;
};

#endif // RECENTFILE_H
