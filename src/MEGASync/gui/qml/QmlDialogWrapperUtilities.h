#ifndef QML_DIALOG_WRAPPER_UTILITIES_H
#define QML_DIALOG_WRAPPER_UTILITIES_H

#include <QGuiApplication>
#include <QObject>
#include <QQuickWindow>
#include <QRect>
#include <QScreen>
#include <QVariant>

namespace QmlDialogWrapperUtilities
{

constexpr const char* SHOW_WHEN_CREATED_PROPERTY = "ShowWhenCreated";

// A dialog is QML when its visible window is a QQuickWindow. The caller must
// obtain the window calling windowHandle() on the dialog's static type:
// QmlDialogWrapperBase::windowHandle() hides (does not override) the
// non-virtual QWidget::windowHandle(), and it is the one returning the inner
// QQuickWindow. For plain widget dialogs, windowHandle() is null or a plain
// QWindow, so the cast fails.
inline bool isQML(const QWindow* window)
{
    return qobject_cast<const QQuickWindow*>(window) != nullptr;
}

// Binds `window` to the screen used for positioning before its global frame
// position is applied. A freshly created QQuickWindow is associated with the
// primary screen, so setFramePosition()/move() converts global coordinates
// using the primary screen's DPI and the window lands off-position on high-DPI
// secondary monitors. Binding it to the right screen first makes the
// logical->native conversion correct.
//
// `screenRef` is the preferred reference (typically the centering target);
// `posRef` is where the window will actually be placed and is used as a
// fallback when `screenRef` lands on no screen (e.g. a gap between monitors or
// a just-disconnected display). If neither resolves to a screen, the window is
// left on its current screen.
inline void bindToScreenForPositioning(QWindow* window,
                                       const QPoint& screenRef,
                                       const QPoint& posRef)
{
    if (!window)
    {
        return;
    }

    QScreen* targetScreen = QGuiApplication::screenAt(screenRef);
    if (!targetScreen)
    {
        targetScreen = QGuiApplication::screenAt(posRef);
    }

    if (targetScreen && window->screen() != targetScreen)
    {
        window->setScreen(targetScreen);
    }
}

inline void setShowWhenCreated(QObject* dialog, bool showWhenCreated)
{
    if (dialog)
    {
        dialog->setProperty(SHOW_WHEN_CREATED_PROPERTY, showWhenCreated);
    }
}

inline bool isShowWhenCreated(const QObject* dialog)
{
    if (!dialog)
    {
        return false;
    }

    auto value(dialog->property(SHOW_WHEN_CREATED_PROPERTY));
    return value.isValid() && value.toBool();
}
}

#endif // QML_DIALOG_WRAPPER_UTILITIES_H
