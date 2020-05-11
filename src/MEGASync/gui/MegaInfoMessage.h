#ifndef MEGAINFOMESSAGE_H
#define MEGAINFOMESSAGE_H

#include <QDialog>
#include <QIcon>

namespace Ui {
class MegaInfoMessage;
}

class MegaInfoMessage : public QDialog
{
    Q_OBJECT

public:
    explicit MegaInfoMessage(const QString &windowTitle,
                             const QString &title,
                             const QString &firstParagraph,
                             const QString &SecondParagraph = QString(),
                             const QIcon &icon = QIcon(),
                             QWidget *parent = nullptr);
    ~MegaInfoMessage();

private slots:
    void on_bClose_clicked();
private:
    Ui::MegaInfoMessage *ui;
};

#endif // MEGAINFOMESSAGE_H
