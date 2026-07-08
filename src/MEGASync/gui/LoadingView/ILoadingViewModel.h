#ifndef ILOADINGVIEWMODEL_H
#define ILOADINGVIEWMODEL_H

#include <QtGlobal>

// Interface for models attached to views managed by ViewLoadingScene.
// isWorking() must return true while the model is doing work in another thread;
// the loading scene refuses to hide while it returns true, and the model is
// responsible for requesting the hide again when that work finishes.
class ILoadingViewModel
{
public:
    virtual ~ILoadingViewModel() = default;

    virtual bool isWorking() const = 0;

protected:
    // Canonical interface pattern: constructible only as a base subobject, never
    // copied/moved through the interface (prevents accidental slicing).
    ILoadingViewModel() = default;
    Q_DISABLE_COPY_MOVE(ILoadingViewModel)
};

#endif // ILOADINGVIEWMODEL_H
