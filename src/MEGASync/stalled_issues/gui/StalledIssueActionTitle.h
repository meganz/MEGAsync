#ifndef STALLEDISSUEACTIONTITLE_H
#define STALLEDISSUEACTIONTITLE_H

#include <QWidget>

namespace Ui {
class StalledIssueActionTitle;
}

class StalledIssueActionTitle : public QWidget
{
    Q_OBJECT

public:
    explicit StalledIssueActionTitle(QWidget *parent = nullptr);
    ~StalledIssueActionTitle();

    void setTitle(const QString& title);
    QString title() const;

    void addActionButton(const QIcon& icon, const QString& text, int id, bool mainButton);
    void hideActionButton(int id);

    virtual void showIcon();
    void addMessage(const QString& message, const QPixmap &pixmap = QPixmap());

    void discard(bool state);
    void setIsCloud(bool state);

signals:
    void actionClicked(int id);

protected:
    Ui::StalledIssueActionTitle *ui;
    bool mIsCloud;
    QString mTitle;

    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // STALLEDISSUEACTIONTITLE_H
