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
    explicit StalledIssueActionTitle(QWidget *parent = nullptr);
    ~StalledIssueActionTitle();

    void setTitle(const QString& title);
    QString title() const;

    void addActionButton(const QString& text, int id);

    void setIndent(int indent);

signals:
    void actionClicked(int id);

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::StalledIssueChooseTitle *ui;
    QString mTitle;
};

#endif // STALLEDISSUECHOOSETITLE_H
