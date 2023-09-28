#ifndef VERSION_H
#define VERSION_H

#define VER_FILEVERSION             5, 0, 22, 0
#define VER_FILEVERSION_CODE        5022
#define VER_PRODUCTVERSION          5, 0, 22, 0
// Update scripts relying on this value if you move it
#define VER_PRODUCTVERSION_STR      "5.0.22.0\0"

#define VER_BUILD_ID                1

#define VER_COMPANYNAME_STR         "Mega Limited\0"
#define VER_FILEDESCRIPTION_STR     "MEGAsync\0"
#define VER_INTERNALNAME_STR        "MEGAsync.exe\0"
#define VER_LEGALCOPYRIGHT_STR      "Mega Limited 2023\0"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_ORIGINALFILENAME_STR    "MEGAsync.exe\0"
#define VER_PRODUCTNAME_STR         "MEGAsync\0"

#define VER_SDK_ID                  "dc22792"

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
"- Fingerprint missing issue can be manually solved.\n"                                            \
"- Sync and backups can be only paused. Disable and suspend options were removed.\n"               \
"- Some issues descriptions have been added/changed.\n"                                            \
"- The list of issues is auto refreshed if all of them are solved.\n"                              \
"- The removed local items go to the local sync debris folder.\n"                                  \
"- The removed remote items go to the MEGA SyncDebris bin folder.\n"                               \
"- Performance improved.\n"                                                                        \
"- Minor bugs fixed.\n"                                                                            \
)

#endif // VERSION_H
