#pragma once

#include <windows.h>
#include <shlobj.h>

class ShellExtSyncing : public IShellIconOverlayIdentifier
{
public:
	ShellExtSyncing(void);
	~ShellExtSyncing(void);
};

