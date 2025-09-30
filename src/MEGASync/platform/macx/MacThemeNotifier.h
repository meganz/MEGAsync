#pragma once
extern "C"
{
    void* MacTheme_Create(void (*cb)(bool isDark));
    void MacTheme_Destroy(void* handle);
    void MacTheme_GetCurrentAppearance(bool* outDark);
}
