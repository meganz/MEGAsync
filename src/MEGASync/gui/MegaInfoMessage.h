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
                             const QString &secondParagraph = QString(),
                             const QIcon &icon = QIcon(),
                             QWidget *parent = nullptr);
    ~MegaInfoMessage();

protected:
    void changeEvent(QEvent * event);
    void setTexts();

private slots:
    void on_bClose_clicked();

private:
    Ui::MegaInfoMessage *ui;
    QString m_windowTitle;
    QString m_title;
    QString m_firstParagraph;
    QString m_secondParagraph;
};

#endif // MEGAINFOMESSAGE_H
