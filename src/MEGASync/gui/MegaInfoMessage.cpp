#include "MegaInfoMessage.h"
#include "ui_MegaInfoMessage.h"

MegaInfoMessage::MegaInfoMessage(const QString &windowTitle, const QString &title, const QString &firstParagraph,
                                 const QString &secondParagraph, const QIcon &icon, QWidget *parent) :
    QDialog(parent), ui(new Ui::MegaInfoMessage), m_windowTitle(windowTitle), m_title(title),
    m_firstParagraph(firstParagraph), m_secondParagraph(secondParagraph)
{
    ui->setupUi(this);
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
    this->setWindowTitle(tr(m_windowTitle.toUtf8().constData()));
    ui->lInfoTitle->setText(tr(m_title.toUtf8().constData()));
    ui->lFirstDescP->setText(tr(m_firstParagraph.toUtf8().constData()));

    if (!m_secondParagraph.isEmpty())
    {
        ui->lSecondDescP->setText(tr(m_secondParagraph.toUtf8().constData()));
    }
}
