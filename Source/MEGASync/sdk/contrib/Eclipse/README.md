Eclipse Project Files
=====================

Configurations, formatters and templates to ease the use of the
Eclipse IDE with the C/C++ Development Tools.

Basic Project Configuration
---------------------------

* `_project` - Eclipse project file
* `_cproject` - CDT project configuration

Copy these two files directly to the project's root `.project` and `.cproject`.
 
 
 
Configure Project Environment
-----------------------------

Using these files:

* `code_templates.xml` - Skeleton templates for common code blocks.
* `editor_templates.xml` - C/C++ editor templates for other code blocks.
* `code_formatter.xml` - Mega coding style configuration.

Steps:

* Setting Mega code templates:
    * Edit Eclipse preferences (click _Window --> Preferences_)
    * Change to _C/C++ --> Code Style --> Code Templates_
    * Use the _Import_ button to import the file `code_templates.xml`
* Setting Mega C/C++ editor templates:
    * Edit Eclipse preferences (click _Window --> Preferences_)
    * Change to _C/C++ --> Editor --> Templates_
    * Use the _Import_ button to import the file `editor_templates.xml`
* Setting Mega coding style for the project:
    * Edit the project's properties (right click on _explorer --> Properties_)
    * Change _C/C++ General --> Formatter_:
    * Tick "Enable project specific settings"
        * Import file `code_formatter.xml`
        * Select important style "BSD/Allman [Mega flavour]" 
 
