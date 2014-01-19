#ifndef RECENTFILE_H
#define RECENTFILE_H

#include <QWidget>
#include <QFileInfo>
#include <QDateTime>

namespace Ui {
class RecentFile;
}

class RecentFile : public QWidget
{
    Q_OBJECT

public:
    explicit RecentFile(QWidget *parent = 0);
    ~RecentFile();
    void setFile(QString fileName, long long fileHandle, QString localPath, long long time);
	void updateWidget();

private:
    Ui::RecentFile *ui;

protected:
    QString fileName;
	long long fileHandle;
    QDateTime dateTime;
	QString localPath;
    void changeEvent(QEvent * event);

private slots:
	void on_pArrow_clicked();
	void on_lFileType_customContextMenuRequested(const QPoint &pos);
	void on_wText_customContextMenuRequested(const QPoint &pos);
	void showInFolder();
	void openFile();
};

#endif // RECENTFILE_H
