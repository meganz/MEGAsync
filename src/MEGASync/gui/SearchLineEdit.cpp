#include "SearchLineEdit.h"

#include "EventHelper.h"
#include "ThemeManager.h"
#include "TokenParserWidgetManager.h"
#include "ui_SearchLineEdit.h"
#include "Utilities.h"

#include <QDebug>
#include <QEvent>
#include <QGraphicsEffect>
#include <QKeyEvent>
#include <QMoveEvent>
#include <QResizeEvent>

static int COLLAPSE_SIZE = 32; /* Square */

SearchLineEdit::SearchLineEdit(QWidget* parent):
    QFrame(parent),
    ui(new Ui::SearchLineEdit),
    mMode(Mode::EXPANDABLE)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground, true);

    EventManager::addEvent(ui->tSearchCancel, QEvent::MouseButtonDblClick, EventHelper::BLOCK);
    EventManager::addEvent(ui->tSearchIcon, QEvent::MouseButtonDblClick, EventHelper::BLOCK);

    connect(ui->tSearchCancel, &QPushButton::clicked, this, &SearchLineEdit::onClearClicked);
    connect(ui->tSearchIcon, &QPushButton::clicked, this, &SearchLineEdit::onSearchButtonClicked);
    connect(ui->leSearchField, &QLineEdit::textChanged, this, &SearchLineEdit::onTextChanged);

    ui->tSearchCancel->setGraphicsEffect(new QGraphicsOpacityEffect());
    ui->customWidget->setGraphicsEffect(new QGraphicsOpacityEffect());

    ui->leSearchField->installEventFilter(this);

#ifdef Q_OS_MACOS
    ui->leSearchField->setAttribute(Qt::WA_MacShowFocusRect, 0);
#endif

    ui->tSearchCancel->setVisible(false);
    ui->leSearchField->setVisible(false);
    ui->customWidget->setVisible(true);

    ui->searchContainer->resize(COLLAPSE_SIZE, COLLAPSE_SIZE);

    setFocusProxy(ui->leSearchField);

    applyPlaceholderColor();

    mTopParent = Utilities::getTopParent<QDialog>(this);
    if (mTopParent)
    {
        mTopParent->installEventFilter(this);
    }
}

SearchLineEdit::~SearchLineEdit()
{
    delete ui;
}

void SearchLineEdit::setText(const QString& text)
{
    ui->leSearchField->setVisible(mMode == Mode::ALWAYS_EXPANDED || !text.isEmpty());
    ui->leSearchField->setText(text);
}

void SearchLineEdit::showTextEntry(bool state, bool force)
{
    if (!force &&
        ((state && ui->leSearchField->isVisible()) || (!state && !ui->leSearchField->isVisible())))
    {
        return;
    }

    if (!state && mMode == Mode::ALWAYS_EXPANDED)
    {
        return;
    }

    if (!state)
    {
        ui->customWidget->show();
        ui->leSearchField->hide();
        ui->tSearchCancel->hide();
        ui->searchContainer->resize(COLLAPSE_SIZE, COLLAPSE_SIZE);
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
                        expand();
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

        ui->leSearchField->setFocus();
    }
}

void SearchLineEdit::setMode(Mode mode)
{
    mMode = mode;
    if (mode == Mode::ALWAYS_EXPANDED)
    {
        expand();
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

void SearchLineEdit::expand()
{
    ui->leSearchField->show();
    ui->leSearchField->resize(ui->leSearchField->sizeHint());
    if (!ui->leSearchField->text().isEmpty())
    {
        ui->tSearchCancel->resize(ui->tSearchCancel->sizeHint());
        ui->tSearchCancel->show();
    }
    ui->customWidget->hide();
}

void SearchLineEdit::setContainerStyle(const QString& backgroundToken,
                                       const QString& borderToken,
                                       const int borderRadius)
{
    // Build a tokenized stylesheet so TokenParserWidgetManager re-themes it on theme changes.
    // One color token per line: the tokenizer's regex does not span newlines.
    QString sheet = QLatin1String("#searchContainer\n{\n");
    sheet +=
        QString::fromLatin1("background-color: #000000; /*colorToken.%1*/\n").arg(backgroundToken);
    sheet += QLatin1String("border-radius: %1px;\n").arg(QString::number(borderRadius));
    if (!borderToken.isEmpty())
    {
        sheet +=
            QString::fromLatin1("border: 1px solid #000000; /*colorToken.%1*/\n").arg(borderToken);
    }
    sheet += QLatin1String("}\n");

    ui->searchContainer->setStyleSheet(sheet);
    ui->searchContainer->style()->unpolish(ui->searchContainer);
    ui->searchContainer->style()->polish(ui->searchContainer);
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
    else if (event->type() == ThemeManager::ThemeChanged)
    {
        applyPlaceholderColor();
    }

    return QFrame::event(event);
}

void SearchLineEdit::moveEvent(QMoveEvent* event)
{
    QFrame::moveEvent(event);
    refreshClearButtonEffect();
}

void SearchLineEdit::resizeEvent(QResizeEvent* event)
{
    QFrame::resizeEvent(event);
    refreshClearButtonEffect();
}

void SearchLineEdit::refreshClearButtonEffect()
{
    // A relayout of the parent header (e.g. when the action buttons collapse as the ghost
    // search tab appears) can leave the clear button's opacity effect with a stale cached
    // pixmap, so it renders clipped until a hover repaint re-grabs the source. effect->update()
    // only recomposites that stale cache; toggling the effect off/on drops it and forces a
    // full re-render at the current geometry.
    if (auto* effect = ui->tSearchCancel->graphicsEffect())
    {
        const bool wasEnabled = effect->isEnabled();
        effect->setEnabled(false);
        effect->setEnabled(wasEnabled);
    }
}

void SearchLineEdit::applyPlaceholderColor()
{
    // QSS cannot style the placeholder text color, so the token is resolved to a QColor and
    // applied through QPalette::PlaceholderText. Re-applied on theme changes (see event()).
    QPalette palette = ui->leSearchField->palette();
    palette.setColor(
        QPalette::PlaceholderText,
        TokenParserWidgetManager::instance()->getColor(QLatin1String("text-placeholder")));
    ui->leSearchField->setPalette(palette);
}

bool SearchLineEdit::eventFilter(QObject* obj, QEvent* evnt)
{
    if (obj == ui->leSearchField && evnt->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(evnt);
        if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
        {
            if (!ui->leSearchField->text().isEmpty())
            {
                emit search(ui->leSearchField->text());
            }
            evnt->accept();
            return true;
        }
        else if (keyEvent && keyEvent->key() == Qt::Key_Escape)
        {
            evnt->accept();
            focusNextChild();
            return true;
        }
    }
    else if (mMode == Mode::EXPANDABLE)
    {
        if (obj == ui->leSearchField && evnt->type() == QEvent::FocusOut &&
            ui->leSearchField->text().isEmpty())
        {
            showTextEntry(false, true);
        }
        else if (mTopParent == obj && evnt->type() == QEvent::MouseButtonRelease)
        {
            onClearClicked();
            showTextEntry(false);
        }
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

    if (text.isEmpty())
    {
        emit cleared();
    }
}

void SearchLineEdit::onSearchButtonClicked()
{
    if (ui->leSearchField->isVisible())
    {
        onClearClicked();
        showTextEntry(false);
    }
    else
    {
        showTextEntry(true);
    }
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
    an->setDuration(125);
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
