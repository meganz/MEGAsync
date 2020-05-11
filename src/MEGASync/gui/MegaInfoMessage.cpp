#include "MegaInfoMessage.h"
#include "ui_MegaInfoMessage.h"

MegaInfoMessage::MegaInfoMessage(const QString &windowTitle, const QString &title, const QString &firstParagraph,
                                 const QString &secondParagraph, const QIcon &icon, QWidget *parent) :
    QDialog(parent), m_windowTitle(windowTitle), m_title(title), m_firstParagraph(firstParagraph), m_secondParagraph(secondParagraph),
    ui(new Ui::MegaInfoMessage)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setTexts();

    if (!icon.isNull())
    {
        ui->bIcon->setIcon(icon);
    }
    else
    {
        ui->wIcon->hide();
    }
}

void MegaInfoMessage::on_bClose_clicked()
{
    accept();
}

MegaInfoMessage::~MegaInfoMessage()
{
    delete ui;
}

void MegaInfoMessage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        setTexts();
    }
    QWidget::changeEvent(event);
}

void MegaInfoMessage::setTexts()
{
    this->setWindowTitle(m_windowTitle);
    ui->lInfoTitle->setText(m_title);
    ui->lFirstDescP->setText(m_firstParagraph);

    if (!m_secondParagraph.isEmpty())
    {
        ui->lSecondDescP->setText(m_secondParagraph);
    }
}
