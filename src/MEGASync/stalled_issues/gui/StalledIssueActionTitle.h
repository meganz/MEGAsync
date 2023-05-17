#ifndef STALLEDISSUEACTIONTITLE_H
#define STALLEDISSUEACTIONTITLE_H

#include <QWidget>
#include <QMap>
#include <QLabel>
#include <QPointer>

namespace Ui {
class StalledIssueActionTitle;
}

class StalledIssueActionTitle : public QWidget
{
    Q_OBJECT

public:
    explicit StalledIssueActionTitle(QWidget *parent = nullptr);
    ~StalledIssueActionTitle();

    void removeBackgroundColor();

    void setTitle(const QString& title);
    QString title() const;

    void addActionButton(const QIcon& icon, const QString& text, int id, bool mainButton);
    void hideActionButton(int id);

    virtual void showIcon();
    void addMessage(const QString& message, const QPixmap &pixmap = QPixmap());

    QLabel* addExtraInfo(const QString& title, const QString& info, int level);

    void setDisabled(bool state);
    void setIsCloud(bool state);

    void updateLastTimeModified(QDateTime &&time);
    void updateCreatedTime(QDateTime &&time);
    void updateUser(const QString& user);
    void updateSize(const QString& size);

signals:
    void actionClicked(int id);

protected:
    Ui::StalledIssueActionTitle *ui;
    bool mIsCloud;
    QString mTitle;

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QMap<int, int> mExtraInfoItemsByRow;
    QPointer<QLabel> mUserLabel;
    QPointer<QLabel> mLastTimeLabel;
    QPointer<QLabel> mCreatedTimeLabel;
    QPointer<QLabel> mSizeLabel;
};

#endif // STALLEDISSUEACTIONTITLE_H
