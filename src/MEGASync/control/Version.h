#ifndef VERSION_H
#define VERSION_H

#define VER_FILEVERSION             5, 0, 21, 0
#define VER_FILEVERSION_CODE        5021
#define VER_PRODUCTVERSION          5, 0, 21, 0
// Update scripts relying on this value if you move it
#define VER_PRODUCTVERSION_STR      "5.0.21.0\0"

#define VER_BUILD_ID                4

#define VER_COMPANYNAME_STR         "Mega Limited\0"
#define VER_FILEDESCRIPTION_STR     "MEGAsync\0"
#define VER_INTERNALNAME_STR        "MEGAsync.exe\0"
#define VER_LEGALCOPYRIGHT_STR      "Mega Limited 2023\0"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_ORIGINALFILENAME_STR    "MEGAsync.exe\0"
#define VER_PRODUCTNAME_STR         "MEGAsync\0"

#define VER_SDK_ID                  "46506b2"

// Update scrips relying on this value if you move it
// Format: 1 item by line, starting from line following the #define
//#define VER_CHANGES_NOTES QT_TRANSLATE_NOOP("Preferences", \
//"- item 1\n"               \
//"- item 2\n\"              \
// [...]
//"- item n\n"
//)
#define VER_CHANGES_NOTES  QT_TRANSLATE_NOOP("Preferences",                                        \
"- Sync Rework Extended-ALPHA test version.\n"                                                     \
"- Crash when moving among categories\n"                                                           \
"- Progress message when fixing issues\n"                                                          \
"- Multiselection fixed for view\n"                                                                \
"- New header for SymLink issues\n"                                                                \
"- New solved category\n"                                                                          \
"- New folders merge logic\n"                                                                      \
"- New links to help center\n"                                                                     \
"- Detect in an issue has been solved externally\n"                                                \
"- All the very latest code changes.\n"                                                            \
)

#endif // VERSION_H
