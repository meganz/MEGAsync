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

    void setSolved(bool state);
    bool isSolved() const;

    void setIsCloud(bool state);

    void setPath(const QString &newPath);

    void updateLastTimeModified(const QDateTime &time);
    void updateCreatedTime(const QDateTime &time);
    void updateUser(const QString& user, bool show);
    void updateVersionsCount(int versions);
    void updateSize(const QString &size);

    enum class AttributeType
    {
        LastModified,
        CreatedTime,
        Size,
        User,
        Versions
    };
    void hideAttribute(AttributeType type);

signals:
    void actionClicked(int id);

protected:
    Ui::StalledIssueActionTitle *ui;
    bool mIsCloud;
    QString mPath;

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void showAttribute(AttributeType type);
    void updateLabel(QLabel* label, const QString& text);

    QMap<AttributeType, QPointer<QLabel>> mUpdateLabels;
};

#endif // STALLEDISSUEACTIONTITLE_H
