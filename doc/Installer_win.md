Guide to generate MEGA Desktop App installers on Windows
==============

[[_TOC_]]

# Requirements

## NSIS (Nullsoft Scriptable Install System)
[Website](https://nsis.sourceforge.io/Download)

## Extra plug-ins:
Extract archive in NSIS install dir, should be C:\Program Files (x86)\NSIS\
[Archive](https://mega.nz/file/YwFmTD5K#wni6lOitZlTAVxxlbfV0UaW1PmleeITH0Za_ti7GB6g)

## Qt 5.15
And the environment variable `MEGA_QTPATH` set to the Qt install dir. Example: `C:\Qt\5.15.11\x64` for the 64 bit version, or `C:\Qt\5.15.11\x86` for the 32 bit version.

## The third party libs installed (with VCPKG)
And the environment variables
- `MEGA_VCPKGPATH` set to the install dir. Example: `C:\mega\3rdParty_desktop`
- `MEGA_THIRD_PARTY_DLL_DIR` set to the subdirectory where the dlls are stored. Example: `bin`

## The Desktop App sources
Open a terminal and clone the Desktop repository:
```
$ mkdir c:\mega\
$ cd c:\mega\
$ git clone --recursive https://github.com/meganz/MEGAsync.git desktop
```

# Steps

## Get the sources
## Call the one-step script
Open a terminal, set the environment variables if necessary, cd to the sources root dir, and run `full_build_process.cmd`:

Usage:
```
"Usage: C:\mega\desktop\full_build_process.cmd [-help] [64|32/64 sign|nosign <cores number> [<suffix>]]"
Script building, signing and creating the installer(s)
It can take 0, 1, 3 or 4 arguments:
        - -help: this message
        - 0 arguments: use these settings: 32/64 sign 1
        - Architecture : 64 or 32/64 to build either for 64 bit or both 32 and 64 bit
        - Sign: sign or nosign if the binaries must be signed or not
        - Cores: the number of cores to build the project, or 0 for default value (4)
        - Suffix for installer: The installer will add this suffix to the version. [OPTIONAl]
MEGA_VCPKGPATH environment variable should be set to the root of the 3rd party dir.
MEGA_QTPATH environment variable should be set to the Qt install dir. Takes the value of MEGAQTPATH, or defaults to C:\Qt\5.15.11\x64
```

### Unsigned build:
```
$ .\full_build_process 64 nosign 8 myBuild
```
This will build only the 64 bit installer, using 8 cores, with version suffix "myBuild", and not sign the binaries.

### Signed build:
Work in progress. For now, please use the following scripts sequentially:
```
$ .\production_build.cmd
$ .\gather_built_products.cmd
$ .\make_uninstallers.cmd
```
Now, sign the files in the folders `sign64` and `sign32`, then copy the signed binaries to `built64` and `built32` respectively.

Then run:
```
$ .\make_installers.cmd
```

