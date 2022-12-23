#include "EventHelper.h"
#include "ui_SearchLineEdit.h"

#include "SearchLineEdit.h"
#include <QDebug>

#include <functional>


SearchLineEdit::SearchLineEdit(QWidget *parent)
    : QFrame(parent),
      ui(new Ui::SearchLineEdit)
{
    ui->setupUi(this);
    ui->tSearchCancel->setVisible(false);
    EventManager::addEvent(ui->tSearchCancel, QEvent::MouseButtonDblClick, EventHelper::BLOCK);
    EventManager::addEvent(ui->tSearchIcon, QEvent::MouseButtonDblClick, EventHelper::BLOCK);
    mButtonManager.addButton(ui->tSearchCancel);
    connect(ui->tSearchCancel, &QToolButton::clicked, this, &SearchLineEdit::onClearClicked);
    connect(ui->leSearchField, &QLineEdit::textChanged, this, &SearchLineEdit::onTextChanged);
    connect(ui->leSearchField, &QLineEdit::editingFinished, this, &SearchLineEdit::onEditingFinieshed);
    ui->tSearchCancel->setGraphicsEffect(new QGraphicsOpacityEffect());
}

SearchLineEdit::~SearchLineEdit()
{
    delete ui;
}

void SearchLineEdit::setText(const QString &text)
{
    ui->leSearchField->setText(text);
}

void SearchLineEdit::onClearClicked()
{
    ui->leSearchField->clear();
    if(ui->tSearchCancel->isVisible())
    {
        makeEffect(true);
    }
    else
    {
        ui->tSearchCancel->hide();
    }
    mOldString = QString();
}

void SearchLineEdit::onTextChanged(const QString &text)
{
    if(!text.isEmpty() && !ui->tSearchCancel->isVisible())
    {
        makeEffect(true);
    }
    else if(text.isEmpty() && ui->tSearchCancel->isVisible())
    {
        makeEffect(false);
    }
}

void SearchLineEdit::animationFinished()
{
   ui->tSearchCancel->setVisible(!ui->leSearchField->text().isEmpty());
   connect(ui->leSearchField, &QLineEdit::textChanged, this, &SearchLineEdit::onTextChanged);
}

void SearchLineEdit::onEditingFinieshed()
{
    if(ui->leSearchField->text().isEmpty() || mOldString == ui->leSearchField->text())
    {
        return;
    }
    mOldString = ui->leSearchField->text();
    emit search(ui->leSearchField->text());
}

void SearchLineEdit::makeEffect(bool fadeIn)
{
    auto an = new QPropertyAnimation(ui->tSearchCancel->graphicsEffect(), "opacity");
    an->setDuration(350);
    if(fadeIn)
    {
        an->setStartValue(0);
        an->setEndValue(1);
        an->setEasingCurve(QEasingCurve::InBack);
        ui->tSearchCancel->setVisible(true);
    }
    else
    {
        an->setStartValue(1);
        an->setEndValue(0);
        an->setEasingCurve(QEasingCurve::OutBack);
    }
    connect(an, &QPropertyAnimation::finished, this, &SearchLineEdit::animationFinished);
    an->start(QAbstractAnimation::DeleteWhenStopped);
    disconnect(ui->leSearchField, &QLineEdit::textChanged, this, &SearchLineEdit::onTextChanged);
}
