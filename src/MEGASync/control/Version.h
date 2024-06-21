#ifndef VERSION_H
#define VERSION_H

#define VER_FILEVERSION             5, 4, 0, 0
#define VER_FILEVERSION_CODE        50400
#define VER_PRODUCTVERSION          5, 4, 0, 0
// Update scripts relying on this value if you move it
#define VER_PRODUCTVERSION_STR      "5.4.0.0\0"

#define VER_BUILD_ID                0

#define VER_COMPANYNAME_STR         "Mega Limited\0"
#define VER_FILEDESCRIPTION_STR     "MEGAsync\0"
#define VER_INTERNALNAME_STR        "MEGAsync.exe\0"
#define VER_LEGALCOPYRIGHT_STR      "Mega Limited 2024\0"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_ORIGINALFILENAME_STR    "MEGAsync.exe\0"
#define VER_PRODUCTNAME_STR         "MEGAsync\0"

#define VER_SDK_ID                  "7648e68"

// Update scrips relying on this value if you move it
// Format: 1 item by line, starting from line following the #define
//#define VER_CHANGES_NOTES QT_TRANSLATE_NOOP("Preferences", \
//"- item 1\n"               \
//"- item 2\n\"              \
// [...]
//"- item n\n"              \
//)
#define VER_CHANGES_NOTES  QT_TRANSLATE_NOOP("Preferences", \
"- New support to Move or Rename sync conflicts.\n" \
"- New fail management for sync conflict resolution.\n" \
"- New UI improvements and fixes in sync conflict dialog.\n" \
"- New option to keep most recently modified file in local/remote sync conflict.\n" \
"- Sync stalls are now automatically resolved as soon as they are detected.\n" \
"- Bug fixes and other improvements.\n" \
)

#endif // VERSION_H
