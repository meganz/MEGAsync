#include "QmlDialog.h"

#include "DialogOpener.h"
#include "Platform.h"
#include "QmlDialogWrapperUtilities.h"

#include <QEvent>
#include <QResizeEvent>
#include <QScreen>

namespace
{
const QLatin1String DEFAULT_RES_MEGA_ICON(":/images/app_ico.ico");
const QLatin1String DEFAULT_TITLE("MEGA");
const qreal HIDDEN_OPACITY(0.0);
const qreal DEFAULT_VISIBLE_OPACITY(1.0);
const int SHOW_WHEN_CREATED_FALLBACK_DELAY_MS(60);
const int RESTORE_OPACITY_DELAY_MS(60);
}

QmlDialog::QmlDialog(QWindow* parent):
    QQuickWindow(parent),
    mIconSrc(DEFAULT_RES_MEGA_ICON)
{
    setFlags(flags() | Qt::Dialog);
    setIcon(QIcon(mIconSrc));
    setTitle(DEFAULT_TITLE);
    setVisible(false);

    connect(this,
            &QmlDialog::requestPageFocus,
            this,
            &QmlDialog::onRequestPageFocus,
            Qt::QueuedConnection);

    // SNC-6567 (Phase 4): QmlInstancesManager removed. Data is exposed to QML
    // directly via child QQmlContext properties registered in QmlDialogWrapper
    // before qmlComponent.create() runs.

    mShowWhenCreatedFallbackTimer.setInterval(SHOW_WHEN_CREATED_FALLBACK_DELAY_MS);
    mShowWhenCreatedFallbackTimer.setSingleShot(true);
    connect(&mShowWhenCreatedFallbackTimer,
            &QTimer::timeout,
            this,
            [this]()
            {
                if (mCenterAndRaiseAfterFirstHeightChangeEvent)
                {
                    placeAndRaise();
                }
            });

    // We restore the opacity with a timer to ensure the dialog is render
    mRestoreOpacityTimer.setInterval(RESTORE_OPACITY_DELAY_MS);
    mRestoreOpacityTimer.setSingleShot(true);
    connect(&mRestoreOpacityTimer,
            &QTimer::timeout,
            this,
            [this]()
            {
                setOpacity(mPreviousOpacity > HIDDEN_OPACITY ? mPreviousOpacity :
                                                               DEFAULT_VISIBLE_OPACITY);

                // requestActivate() is required by Windows; raise() by macOS.
                // Qt5-ONLY Wayland branch: on Qt5 a client cannot activate itself — the call
                // logs a warning and can crash — so we request attention via alert() instead.
                // TODO Qt6: remove the Wayland branch and call requestActivate()
                // unconditionally; Qt6 activates via xdg-activation without crashing.
                if (Platform::getInstance()->isWayland())
                {
                    alert(0);
                }
                else
                {
                    requestActivate();
                }
                raise();

                if (!mInitialLayoutComplete)
                {
                    mInitialLayoutComplete = true;
                    emit initialLayoutCompleteChanged();
                }
            });
}

void QmlDialog::setIconSrc(const QString& iconSrc)
{
    QString source = iconSrc;
    if (iconSrc.startsWith(QString::fromUtf8("qrc:")))
    {
        source = source.mid(3);
    }

    if (source != mIconSrc)
    {
        mIconSrc = source;
        setIcon(QIcon(mIconSrc));
    }
}

void QmlDialog::readyToBeShow()
{
    mCenterAndRaiseAfterFirstHeightChangeEvent = true;
    mTrackedSize = geometry().size();

    if (mInitialLayoutComplete)
    {
        mInitialLayoutComplete = false;
        emit initialLayoutCompleteChanged();
    }

    hide();
    // Set the opacity to 0.0 to hide the window even if it is shown
    // The opacity will be set again to the real opacity
    mPreviousOpacity = opacity() > HIDDEN_OPACITY ? opacity() : DEFAULT_VISIBLE_OPACITY;

    // Qt5-ONLY Wayland branch: the opacity-hide trick is unreliable on Qt5
    // Wayland. The qtwayland QPA plugin does not implement window opacity
    // (every setOpacity() call logs "This plugin does not support setting
    // window opacity" and is ignored), so on most compositors the hide is a
    // no-op that only spams warnings; on plugins/compositors that DO honor it,
    // the later restore in mRestoreOpacityTimer is not re-presented (no forced
    // surface commit), leaving the dialog mapped-but-invisible. A Wayland
    // client also cannot position its own window, so there is nothing to hide
    // *for*: placeAndRaise()'s setFramePosition() is a no-op there. Keep the
    // window opaque; the fallback timer still drives the subsequent
    // raise/activate. TODO Qt6: remove this branch — Qt6 Wayland honors live
    // opacity changes, so the cross-platform opacity path below works there.
    if (Platform::getInstance()->isWayland())
    {
        setOpacity(mPreviousOpacity);
    }
    else
    {
        setOpacity(HIDDEN_OPACITY);
    }

    show();
    mShowWhenCreatedFallbackTimer.start();
}

bool QmlDialog::initialLayoutComplete() const
{
    return mInitialLayoutComplete;
}

void QmlDialog::attachToParentWindow(QWindow* parentWindow, bool embedded)
{
    if (parentWindow == nullptr || parentWindow == this)
    {
        return;
    }

    // Idempotent: only re-bind if the transient parent actually changed.
    if (transientParent() != parentWindow)
    {
        setTransientParent(parentWindow);
    }

    if (embedded)
    {
        // If QML left it NonModal, promote to WindowModal. Do not downgrade
        // ApplicationModal if a dialog explicitly asked for it.
        if (modality() == Qt::NonModal)
        {
            setModality(Qt::WindowModal);
        }

#ifndef Q_OS_WIN
        // Embedded dialogs should not show maximize/minimize affordances.
        // Keep Qt::Dialog (set by the constructor) and the close button.
        //
        // NOTE: this is intentionally skipped on Windows and on the native
        // Wayland QPA. Calling setFlags() on a QQuickWindow whose native
        // window already exists triggers a window recreation that loses the
        // QML width/height bindings — the dialog flashes frameless and then
        // collapses to a tiny / very narrow window. On Windows the native
        // HWND is recreated; on Wayland the wl_surface/xdg_toplevel is
        // recreated and comes back unconfigured (the compositor proposes a
        // 0-width size), so the dialog reappears collapsed. transientParent +
        // WindowModal above are enough to give us the embedded modal behavior;
        // the fixed size is already guaranteed by the QML's
        // minimumWidth/maximumWidth declarations, and the native dialog frame
        // from Qt::Dialog (set in the constructor) is preserved untouched.
        // REVIEW(Qt6): cross-version fix, not a pure Qt5 workaround — do NOT
        // blindly revert. The setFlags() surface-recreation collapse is not
        // Qt5-specific; the Qt6 tree carries the same guard. Verify it is still
        // needed on Qt6 before changing.
        if (!Platform::getInstance()->isWayland())
        {
            Qt::WindowFlags wflags = flags();
            wflags |= Qt::Dialog;
            wflags &= ~(Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);
            if (wflags != flags())
            {
                setFlags(wflags);
            }
        }
#endif
    }
}

bool QmlDialog::getCloseOnEscapePressed() const
{
    return mCloseOnEscapePressed;
}

void QmlDialog::setCloseOnEscapePressed(bool active)
{
    if (active != mCloseOnEscapePressed)
    {
        mCloseOnEscapePressed = active;

        emit closeOnEscapePressedChanged();
    }
}

bool QmlDialog::event(QEvent* event)
{
    if (event->type() == QEvent::Close)
    {
        emit finished();
    }
    else if (event->type() == QEvent::Resize)
    {
        auto* resizeEvent = static_cast<QResizeEvent*>(event);

#ifdef Q_OS_LINUX
        // Linux qml dialogs starts with QSize(1,1), so the first resize is invalid
        if (mTrackedSize.height() <= 1)
        {
            mTrackedSize = resizeEvent->size();
            return QQuickWindow::event(event);
        }
#endif

        // Debounce: while we are in the post-show "place & raise" phase,
        // every Resize event restarts the fallback timer. placeAndRaise()
        // only runs when the timer fires (no resize for one interval),
        // i.e. after the QML content has settled on its final size.
        //
        // Without this, multi-pass layouts (e.g. BackupCandidates' folder
        // list whose height is computed in two passes) produce a visible
        // shrink: the first Resize triggered placeAndRaise immediately,
        // restored the opacity, and the second Resize that arrived after
        // was already visible on screen.
        if (mCenterAndRaiseAfterFirstHeightChangeEvent && resizeEvent &&
            mTrackedSize != resizeEvent->size())
        {
            mTrackedSize = resizeEvent->size();
            mShowWhenCreatedFallbackTimer.start();
        }
    }
    else if (event->type() == QEvent::KeyPress)
    {
        auto* keyPressEvent = static_cast<QKeyEvent*>(event);
        if (mCloseOnEscapePressed && keyPressEvent != nullptr &&
            keyPressEvent->key() == Qt::Key_Escape)
        {
            close();
        }
    }

    return QQuickWindow::event(event);
}

void QmlDialog::onRequestPageFocus()
{
    emit initializePageFocus();
}

void QmlDialog::placeAndRaise()
{
    mShowWhenCreatedFallbackTimer.stop();

    mCenterAndRaiseAfterFirstHeightChangeEvent = false;

    QSize dialogSize = geometry().size();
    auto parentGeometry = QmlDialogWrapperUtilities::getParentGeometry(this);
    QmlDialog::setFramePosition(DialogOpener::initialDialogPosition(dialogSize, parentGeometry));

    mRestoreOpacityTimer.start();
}
