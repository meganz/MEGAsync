#ifndef VERSION_H
#define VERSION_H

#define VER_FILEVERSION 5, 11, 1, 0
#define VER_FILEVERSION_CODE 51101
#define VER_PRODUCTVERSION 5, 11, 1, 0
// Update scripts relying on this value if you move it
#define VER_PRODUCTVERSION_STR "5.11.1.0\0"

#define VER_BUILD_ID 0

#define VER_COMPANYNAME_STR         "Mega Limited\0"
#define VER_FILEDESCRIPTION_STR     "MEGAsync\0"
#define VER_INTERNALNAME_STR        "MEGAsync.exe\0"
#define VER_LEGALCOPYRIGHT_STR "Mega Limited 2025\0"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_ORIGINALFILENAME_STR    "MEGAsync.exe\0"
#define VER_PRODUCTNAME_STR         "MEGAsync\0"

/* SDK commit hash, 7 chars */
#define VER_SDK_ID "f60237a"

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
                      "- Added refinements to the Windows 11 Explorer context menu, delivering a " \
                      "more streamlined and polished user experience.\n" \
                      "- Resolved issue with incorrect default permission settings applied to " \
                      "files and folders created during synchronization.\n" \
                      "- Other bugs have been fixed and numerous improvements made.\n")

#endif // VERSION_H
