#ifndef VERSION_H
#define VERSION_H

#define VER_FILEVERSION             5, 2, 0, 0
#define VER_FILEVERSION_CODE        50200
#define VER_PRODUCTVERSION          5, 2, 0, 0
// Update scripts relying on this value if you move it
#define VER_PRODUCTVERSION_STR      "5.2.0.0\0"

#define VER_BUILD_ID                0

#define VER_COMPANYNAME_STR         "Mega Limited\0"
#define VER_FILEDESCRIPTION_STR     "MEGAsync\0"
#define VER_INTERNALNAME_STR        "MEGAsync.exe\0"
#define VER_LEGALCOPYRIGHT_STR      "Mega Limited 2024\0"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_ORIGINALFILENAME_STR    "MEGAsync.exe\0"
#define VER_PRODUCTNAME_STR         "MEGAsync\0"

#define VER_SDK_ID                  "f71b604"

// Update scrips relying on this value if you move it
// Format: 1 item by line, starting from line following the #define
//#define VER_CHANGES_NOTES QT_TRANSLATE_NOOP("Preferences", \
//"- item 1\n"               \
//"- item 2\n\"              \
// [...]
//"- item n\n"              \
//)
#define VER_CHANGES_NOTES  QT_TRANSLATE_NOOP("Preferences",                                        \
"- Sync rework Beta V2 is released.\n"                                                             \
"- A new onboarding wizard is introduced for any new sync or backup processes.\n"                  \
"- Changes are detected and stalled issues are updated automatically.\n"                           \
"- MEGA-supported language set is now available.\n"                                                \
"- User experience for batch fixing sync issues is improved.\n"                                    \
"- Minor bugs are fixed.\n"                                                                        \
"- Performance is improved.\n"                                                                     \
)

#endif // VERSION_H
