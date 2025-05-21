#include "ContextMenuCommandUpload.h"

#include "MEGAinterface.h"

static const std::wstring ICON = L"upload.ico";

ContextMenuCommandUpload::ContextMenuCommandUpload():
    ContextMenuCommandBase(L"ContextMenuCommandUpload")
{}

IFACEMETHODIMP ContextMenuCommandUpload::GetTitle(IShellItemArray*, LPWSTR* ppszName)
{
    SetTitle(MegaInterface::STRING_UPLOAD, ppszName);

    return S_OK;
}

IFACEMETHODIMP ContextMenuCommandUpload::GetToolTip(IShellItemArray* psiItemArray,
                                                    LPWSTR* ppszInfotip)
{
    return GetTitle(psiItemArray, ppszInfotip);
}

IFACEMETHODIMP ContextMenuCommandUpload::Invoke(IShellItemArray* psiItemArray,
                                                IBindCtx* pbc) noexcept
{
    UNREFERENCED_PARAMETER(pbc);

    mContextMenuData.requestUpload();

    return S_OK;
}

EXPCMDSTATE ContextMenuCommandUpload::GetState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
    {
        return ECS_HIDDEN;
    }

    if (mContextMenuData.canRequestUpload())
    {
        return ECS_ENABLED;
    }

    return ECS_HIDDEN;
}

std::wstring ContextMenuCommandUpload::GetIcon() const
{
    return ICON;
}
