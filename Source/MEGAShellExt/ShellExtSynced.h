#pragma once

#include <windows.h>
#include <shlobj.h>

class ShellExtSynced : public IShellIconOverlayIdentifier
{
public:
	ShellExtSynced(void);
	~ShellExtSynced(void);
};


