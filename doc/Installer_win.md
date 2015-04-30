Guide to generate MEGASync installers on Windows
==============

Required tools
--------------

**NSIS** (Nullsoft Scriptable Install System)
Download and install the tool to create installers.

[Binary](https://unsis.googlecode.com/files/nsis-2.46.5-Unicode-setup.exe)
[Website](http://www.scratchpaper.com/)

*Note*: some plugins for NSIS are required. You can directly overwrite the content of your NSIS folder in "Program files x86" with the following files [Download here](https://mega.co.nz/#!TEkgXI5L!V5BOuXojVG_Q-y5szwUFPdgIE-YzwhXCQ1hEBMdoBmM)

NOTA --> actualizaremos ese enlace para que tenga las traducciones (si están listas)

**HM NIS EDIT**
Download and install this graphical user interface to edit and configure NSIS.

[Binary](http://prdownloads.sourceforge.net/hmne/nisedit2.0.3.exe?download)
[Website](http://hmne.sourceforge.net/)

Configure the installer
--------------

 - Open the tool HM NIS Edit and load the file "installer.nsi" located in the <project_folder>.
 - Update the "Product version" if necessary.
 - Set the variable `QT_PATH` to point to the path where your QT SDK is installed.
 - Set the variables `BUILDPATH_X86` and `BUILDPATH_X64` to point to the paths where your binaries were built. Note that you need to set both variables despite you are building a MEGASync installer for x86, since both versions x86 and x64 of the MEGAShellExt are required to create the installer.

Extra info: in order to generate the 64 bits version, you will need the VC2010_x64 compiler. QT libs for 64 bits are not mandatory, since MEGAShellExt does not use QT at all.
If you are using QT Creator to build the DLL for 64 bits, you need to add "Additional arguments" to the "Build Steps" in the "Projects" section: `CONFIG+=BUILDX64`. This way, the x64 version of SDK libraries will be properly linked.

Create the uninstaller
--------------

 - In the HM NIS Edit, search for the define `BUILD_UNINSTALLER` and uncomment the line by deleting the #
	`!define BUILD_UNINSTALLER`
 - Click on **Compile** under the menu NSIS to create the Uninstaller generator.
 - Generate the "uninst.exe" by executing the file "UninstallerGenerator.exe". It should have been created in your <project_folder> after successfull compilation.
 - Undo the change by commenting back the line above.

Create the installer
--------------

 - In the HM NIS Edit, click on **Compile** under the menu NSIS to create the Uninstaller generator.
 - The "MEGAsyncSETUP.exe" file should be in your <project_folder> now.

Final comments
--------------

 - MEGASync is released exclusively in x86. Versions for 64 bits are generated exclusively on demand.
 - If the new installer is intended to be a public version (instead of a debug version for customers or internal usage), the file must be signed by using the MEGA certificate.
 
