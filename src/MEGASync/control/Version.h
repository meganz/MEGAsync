#ifndef VERSION_H
#define VERSION_H

#define VER_FILEVERSION 5, 7, 0, 0
#define VER_FILEVERSION_CODE 50700
#define VER_PRODUCTVERSION 5, 7, 0, 0
// Update scripts relying on this value if you move it
#define VER_PRODUCTVERSION_STR "5.7.0.0\0"

#define VER_BUILD_ID                0

#define VER_COMPANYNAME_STR         "Mega Limited\0"
#define VER_FILEDESCRIPTION_STR     "MEGAsync\0"
#define VER_INTERNALNAME_STR        "MEGAsync.exe\0"
#define VER_LEGALCOPYRIGHT_STR "Mega Limited 2025\0"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_ORIGINALFILENAME_STR    "MEGAsync.exe\0"
#define VER_PRODUCTNAME_STR         "MEGAsync\0"

/* SDK commit hash, 7 chars */
#define VER_SDK_ID "a014e4c"

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
                  "- Enhanced stability with isolated graphics processing: We’ve upgraded our " \
                  "app to run the graphics processor (GFX) in a separate process. This means " \
                  "that if any third-party graphics libraries encounter issues, the app remains " \
                  "stable and unaffected, ensuring a smoother, uninterrupted experience.\n" \
                  "- End of support for macOS 10.13 and 10.14: This update no longer supports " \
                  "macOS 10.13 and 10.14. To continue receiving updates and support, please " \
                  "upgrade to a more recent version of macOS.\n" \
                  "- The Settings dialogue has been redesigned.\n" \
                  "- Improved app issue detection and resolution.\n" \
                  "- Other bugs have been fixed and numerous improvements made.\n")

#endif // VERSION_H
