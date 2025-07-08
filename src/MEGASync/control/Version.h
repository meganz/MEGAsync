#ifndef VERSION_H
#define VERSION_H

// These numbers are used in different build project scripts. Keep this in
// mind if you want to change the format.
#define VER_MAJOR 5
#define VER_MINOR 14
#define VER_MICRO 0
#define VER_RC 1
// Format: "VER_MAJOR.VER_MINOR.VER_MICRO.VER_RC\0"
#define VER_PRODUCTVERSION_STR "5.14.0.1\0"
#define VER_PRODUCTVERSION VER_MAJOR, VER_MINOR, VER_MICRO, VER_RC
#define VER_FILEVERSION VER_MAJOR, VER_MINOR, VER_MICRO, VER_RC
#define VER_FILEVERSION_CODE (VER_MAJOR * 10000 + VER_MINOR * 100 + VER_MICRO)

#ifndef VER_BUILD_ID
#define VER_BUILD_ID 0
#endif

#define VER_COMPANYNAME_STR         "Mega Limited\0"
#define VER_FILEDESCRIPTION_STR     "MEGAsync\0"
#define VER_INTERNALNAME_STR        "MEGAsync.exe\0"
#define VER_LEGALCOPYRIGHT_STR "Mega Limited 2025\0"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_ORIGINALFILENAME_STR    "MEGAsync.exe\0"
#define VER_PRODUCTNAME_STR         "MEGAsync\0"

/* SDK commit hash, 7 chars */
#define VER_SDK_ID "9d81643"

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
                      "- Increased the number of parallel transfers.\n" \
                      "- It’s now easier to add syncs during setup.\n" \
                      "- Added support for proxy auto-detection in Linux.\n" \
                      "- We’ve fixed bugs and made the app more reliable.\n")

#endif // VERSION_H
