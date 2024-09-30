#ifndef VERSION_H
#define VERSION_H

#define VER_FILEVERSION 5, 5, 0, 0
#define VER_FILEVERSION_CODE 50500
#define VER_PRODUCTVERSION 5, 5, 0, 0
// Update scripts relying on this value if you move it
#define VER_PRODUCTVERSION_STR "5.5.0.0\0"

#define VER_BUILD_ID                2

#define VER_COMPANYNAME_STR         "Mega Limited\0"
#define VER_FILEDESCRIPTION_STR     "MEGAsync\0"
#define VER_INTERNALNAME_STR        "MEGAsync.exe\0"
#define VER_LEGALCOPYRIGHT_STR      "Mega Limited 2024\0"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_ORIGINALFILENAME_STR    "MEGAsync.exe\0"
#define VER_PRODUCTNAME_STR         "MEGAsync\0"

#define VER_SDK_ID "eb86660"

/* Update scrips relying on this value if you move it
Format: 1 item by line, starting from line following the #define
#define VER_CHANGES_NOTES QT_TRANSLATE_NOOP("Preferences",
"- item 1\n"
"- item 2\n\"
 [...]
"- item n\n"
)*/
#define VER_CHANGES_NOTES \
QT_TRANSLATE_NOOP("Preferences", \
                  "- You can now select multiple files and folders to remove from the Cloud " \
                  "drive when using the Download option.\n" \
                  "- Creating syncs and backups from the Settings menu is easier than ever with " \
                  "our revamped interface.\n" \
                  "- An installation issue which caused some Windows .dll files to be deleted " \
                  "has been resolved.\n" \
                  "- Third-party libraries have been updated.\n" \
                  "- Other bugs have been fixed and numerous improvements made.\n")

#endif // VERSION_H
