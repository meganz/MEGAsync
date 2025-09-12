#include "SearchLineEdit.h"

#include "EventHelper.h"
#include "TokenParserWidgetManager.h"
#include "ui_SearchLineEdit.h"

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>

static int COLLAPSE_SIZE = 32; /* Square */

SearchLineEdit::SearchLineEdit(QWidget* parent):
    QFrame(parent),
    ui(new Ui::SearchLineEdit)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground, true);

    EventManager::addEvent(ui->tSearchCancel, QEvent::MouseButtonDblClick, EventHelper::BLOCK);
    EventManager::addEvent(ui->tSearchIcon, QEvent::MouseButtonDblClick, EventHelper::BLOCK);

    connect(ui->tSearchCancel, &QPushButton::clicked, this, &SearchLineEdit::onClearClicked);
    connect(ui->tSearchIcon, &QPushButton::clicked, this, &SearchLineEdit::onSearchButtonClicked);
    connect(ui->leSearchField, &QLineEdit::textChanged, this, &SearchLineEdit::onTextChanged);

    ui->tSearchCancel->setGraphicsEffect(new QGraphicsOpacityEffect());
    ui->leSearchField->setGraphicsEffect(new QGraphicsOpacityEffect());
    ui->customWidget->setGraphicsEffect(new QGraphicsOpacityEffect());

    ui->leSearchField->installEventFilter(this);
    parentWidget()->installEventFilter(this);

#ifdef Q_OS_MACOS
    ui->leSearchField->setAttribute(Qt::WA_MacShowFocusRect, 0);
#endif

    ui->tSearchCancel->setVisible(false);
    ui->leSearchField->setVisible(false);
    ui->customWidget->setVisible(true);

    ui->searchContainer->resize(COLLAPSE_SIZE, COLLAPSE_SIZE);

    setFocusProxy(ui->leSearchField);
}

SearchLineEdit::~SearchLineEdit()
{
    delete ui;
}

void SearchLineEdit::setText(const QString& text)
{
    ui->leSearchField->setText(text);
}

void SearchLineEdit::showTextEntry(bool state)
{
    if ((state && ui->leSearchField->isVisible()) || (!state && !ui->leSearchField->isVisible()))
    {
        return;
    }

    if (!state)
    {
        // Collapse lineEdit
        {
            runWidthAnimation(ui->leSearchField, false);
        }

        // Collapse lineEdit
        {
            runWidthAnimation(ui->tSearchCancel, false);
        }

        // Move search icon
        {
            QRect startRect(ui->tSearchIcon->geometry());
            QRect endRect(ui->searchContainer->width() - ui->tSearchIcon->width(),
                          ui->tSearchIcon->y(),
                          ui->tSearchIcon->size().width(),
                          ui->tSearchIcon->size().height());

            QPropertyAnimation* animation =
                runGeometryAnimation(ui->tSearchIcon, startRect, endRect, QEasingCurve::Linear);

            connect(animation,
                    &QPropertyAnimation::finished,
                    this,
                    [this]()
                    {
                        ui->leSearchField->hide();
                        ui->tSearchCancel->hide();
                        ui->customWidget->show();
                    });
        }

        {
            // Reduce background to COLLAPSE size
            QRect startRect(ui->searchContainer->geometry());
            QRect endRect(size().width() - COLLAPSE_SIZE, 0, COLLAPSE_SIZE, COLLAPSE_SIZE);

            runGeometryAnimation(ui->searchContainer, startRect, endRect, QEasingCurve::Linear);
        }
    }
    else
    {
        // resize background
        {
            QRect startRect(ui->searchContainer->geometry().x(), 0, COLLAPSE_SIZE, COLLAPSE_SIZE);
            QRect endRect(QRect(0, 0, size().width(), size().height()));

            QPropertyAnimation* animation =
                runGeometryAnimation(ui->searchContainer, startRect, endRect, QEasingCurve::Linear);

            // As we finish the resize, show the rest of elements
            connect(animation,
                    &QPropertyAnimation::finished,
                    this,
                    [this]()
                    {
                        ui->leSearchField->show();
                        ui->leSearchField->resize(ui->leSearchField->sizeHint());
                        if (!ui->leSearchField->text().isEmpty())
                        {
                            ui->tSearchCancel->resize(ui->tSearchCancel->sizeHint());
                            ui->tSearchCancel->show();
                        }
                        ui->customWidget->hide();
                    });
        }

        // Move search icon
        {
            QRect startRect(ui->tSearchIcon->geometry().x(),
                            ui->tSearchIcon->geometry().y(),
                            ui->tSearchIcon->size().width(),
                            ui->tSearchIcon->size().height());
            // Not sure why it is 10, but with this magical number works very well
            QRect endRect(QRect(10,
                                ui->tSearchIcon->y(),
                                ui->tSearchIcon->size().width(),
                                ui->tSearchIcon->size().height()));

            runGeometryAnimation(ui->tSearchIcon, startRect, endRect, QEasingCurve::Linear);
        }
    }
}

QPropertyAnimation* SearchLineEdit::runGeometryAnimation(QWidget* target,
                                                         const QRect& startRect,
                                                         const QRect& endRect,
                                                         QEasingCurve type)
{
    QPropertyAnimation* animation = new QPropertyAnimation(target, "geometry");
    animation->setDuration(350);
    animation->setStartValue(startRect);
    animation->setEndValue(endRect);
    animation->setEasingCurve(type);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    return animation;
}

void SearchLineEdit::addCustomWidget(QWidget* widget)
{
    ui->customLayout->addWidget(widget);
}

bool SearchLineEdit::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    return QFrame::event(event);
}

bool SearchLineEdit::eventFilter(QObject* obj, QEvent* evnt)
{
    if (obj == ui->leSearchField && evnt->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(evnt);
        if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
        {
            if (!ui->leSearchField->text().isEmpty() && mOldString != ui->leSearchField->text())
            {
                mOldString = ui->leSearchField->text();
                emit search(ui->leSearchField->text());
            }
        }
    }
    else if (obj == ui->leSearchField && ui->leSearchField->isVisible() &&
             evnt->type() == QEvent::FocusOut && ui->leSearchField->text().isEmpty())
    {
        showTextEntry(false);
    }

    return QFrame::eventFilter(obj, evnt);
}

void SearchLineEdit::onClearClicked()
{
    ui->leSearchField->clear();
    if (ui->tSearchCancel->isVisible())
    {
        toggleClearButton(false);
    }
    else
    {
        ui->tSearchCancel->hide();
    }
    mOldString = QString();
}

void SearchLineEdit::onTextChanged(const QString& text)
{
    if (!text.isEmpty() && !ui->tSearchCancel->isVisible())
    {
        toggleClearButton(true);
    }
    else if (text.isEmpty() && ui->tSearchCancel->isVisible())
    {
        toggleClearButton(false);
    }
}

void SearchLineEdit::onSearchButtonClicked()
{
    showTextEntry(true);
}

void SearchLineEdit::animationFinished()
{
    ui->tSearchCancel->setVisible(!ui->leSearchField->text().isEmpty());
    connect(ui->leSearchField, &QLineEdit::textChanged, this, &SearchLineEdit::onTextChanged);
}

void SearchLineEdit::toggleClearButton(bool fadeIn)
{
    auto an = runOpacityAnimation(ui->tSearchCancel, fadeIn);
    connect(an, &QPropertyAnimation::finished, this, &SearchLineEdit::animationFinished);

    // Meanwhile the close button appears/disappears
    disconnect(ui->leSearchField, &QLineEdit::textChanged, this, &SearchLineEdit::onTextChanged);
}

QPropertyAnimation* SearchLineEdit::runWidthAnimation(QWidget* target, bool expand)
{
    auto an = new QPropertyAnimation(target->graphicsEffect(), "width");
    an->setDuration(250);
    if (expand)
    {
        an->setStartValue(0);
        an->setEndValue(target->sizeHint().width());
        an->setEasingCurve(QEasingCurve::Linear);
        ui->tSearchCancel->setVisible(true);
    }
    else
    {
        an->setStartValue(target->width());
        an->setEndValue(0);
        an->setEasingCurve(QEasingCurve::Linear);
    }
    an->start(QAbstractAnimation::DeleteWhenStopped);
    return an;
}

QPropertyAnimation* SearchLineEdit::runOpacityAnimation(QWidget* target, bool fadeIn)
{
    auto an = new QPropertyAnimation(target->graphicsEffect(), "opacity");
    an->setDuration(350);
    if (fadeIn)
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
    an->start(QAbstractAnimation::DeleteWhenStopped);
    return an;
}
