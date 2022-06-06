#ifndef STALLEDISSUECHOOSETITLE_H
#define STALLEDISSUECHOOSETITLE_H

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

    void addActionButton(const QIcon& icon, const QString& text, int id);
    void hideActionButton(int id);

    void showIcon();
    void addMessage(const QString& message);
    void setIndent(int indent);
    void setRoundedCorners(RoundedCorners type);

signals:
    void actionClicked(int id);

protected:
    void paintEvent(QPaintEvent *) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::StalledIssueChooseTitle *ui;
    QString mTitle;
    RoundedCorners mRoundedCorners;
};

#endif // STALLEDISSUECHOOSETITLE_H
