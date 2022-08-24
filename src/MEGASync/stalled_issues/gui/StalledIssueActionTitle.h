#ifndef STALLEDISSUEACTIONTITLE_H
#define STALLEDISSUEACTIONTITLE_H

#include <QWidget>

namespace Ui {
class StalledIssueChooseTitle;
}

class StalledIssueActionTitle : public QWidget
{
    Q_OBJECT

public:
    enum class RoundedCorners
    {
        TOP_CORNERS,
        ALL_CORNERS
    };

    explicit StalledIssueActionTitle(QWidget *parent = nullptr);
    ~StalledIssueActionTitle();

    void setTitle(const QString& title);
    QString title() const;

    void addActionButton(const QIcon& icon, const QString& text, int id, bool mainButton);
    void hideActionButton(int id);

    virtual void showIcon();
    void addMessage(const QString& message, const QPixmap &pixmap = QPixmap());
    void setIndent(int indent);
    void setRoundedCorners(RoundedCorners type);

    void setDisabled(bool state);
    void setIsCloud(bool state);

signals:
    void actionClicked(int id);

protected:
    Ui::StalledIssueChooseTitle *ui;
    bool mIsCloud;
    QString mTitle;

    void paintEvent(QPaintEvent *) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    RoundedCorners mRoundedCorners;
};

#endif // STALLEDISSUEACTIONTITLE_H
