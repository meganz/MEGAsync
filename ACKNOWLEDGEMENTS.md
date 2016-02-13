## ACKNOWLEDGEMENTS

Here is the list of the external libraries used by MEGAsync.
We thank them all for their contributions:

#### QT
http://www.qt.io/

© 2016 The Qt Company Ltd.

###### Description:
QT is framework for cross-platform develppment

######  Usage:
QT is the framework used by MEGAsync to get cross platform compatibility 

######  License: 
Dual license: Open Source + Commercial

http://www.qt.io/licensing/

--------------------------------------------------------------------

#### Google Breakpad
https://chromium.googlesource.com/breakpad/breakpad/

Copyright (c) 2006, Google Inc.

###### Description:
Breakpad is a set of client and server components which implement a 
crash-reporting system.

###### Usage:
MEGAsync uses this library to capture crashes on Windows and OS X

###### License:
The BSD 3-Clause License

https://chromium.googlesource.com/breakpad/breakpad/+/master/LICENSE

--------------------------------------------------------------------

#### Bitcoin Core
https://github.com/bitcoin/bitcoin

Copyright (c) 2011-2013 The Bitcoin Core developers

###### Description:
Bitcoin is an experimental new digital currency that enables instant 
payments to anyone, anywhere in the world. Bitcoin uses peer-to-peer 
technology to operate with no central authority: managing transactions 
and issuing money are carried out collectively by the network. Bitcoin Core 
is the name of open source software which enables the use of this currency.

###### Usage:
MEGAsync uses some files in that repository to show desktop notifications 
on OS X and Linux. Specifically, MEGAsync uses these files:
- notificator.cpp 
- notificator.h 
- macnotificationhandler.mm 
- macnotificationhandler.h

from this folder: https://github.com/bitcoin/bitcoin/blob/master/src/qt/

###### License: 
MIT/X11 software license

http://www.opensource.org/licenses/mit-license.php

--------------------------------------------------------------------

#### C++ Windows Shell context menu handler (CppShellExtContextMenuHandler)
https://code.msdn.microsoft.com/windowsapps/CppShellExtContextMenuHandl-410a709a

Copyright (c) Microsoft Corporation.

###### Description:
Example code from Microsoft for the creation of a context menu for
Windows Explorer

###### Usage:
This example was the base for the implementation of the shell extension of
MEGAsync on Windows (MEGAShellExt project in this repository)

###### License:
Microsoft Public License.

https://opensource.org/licenses/MS-PL

--------------------------------------------------------------------

#### QtLockedFile
https://github.com/qtproject/qt-solutions/tree/master/qtlockedfile/src

Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).

###### Description:
Provides locking between processes using a file.

###### Usage:
MEGAsync uses this library to detect if it's already running and avoid to have
several instances running at once.

###### License: 
The BSD 3-Clause License

--------------------------------------------------------------------

#### MEGA C++ SDK
https://github.com/meganz/sdk

(c) 2013-2016 by Mega Limited, Auckland, New Zealand

###### Description:
MEGA --- The Privacy Company --- is a Secure Cloud Storage provider that protects 
your data thanks to end-to-end encryption. We call it User Controlled Encryption, 
or UCE, and all our clients automatically manage it.

All files stored on MEGA are encrypted. All data transfers from and to MEGA 
are encrypted. And while most cloud storage providers can and do claim the same, 
MEGA is different – unlike the industry norm where the cloud storage provider 
holds the decryption key, with MEGA, you control the encryption, you hold the keys, 
and you decide who you grant or deny access to your files.

This SDK brings you all the power of our client applications and let you create 
your own or analyze the security of our products.

###### Usage:
MEGAsync uses the MEGA C++ SDK to get all functionality that requires access
to MEGA servers.

###### License:
Simplified (2-clause) BSD License.

https://github.com/meganz/sdk/blob/master/LICENSE

--------------------------------------------------------------------

#### Dependencies of the MEGA C++ SDK
Due to the usage of the MEGA C++ SDK, MEGAsync requires some additional 
libraries. Here is a brief description of all of them:

#### c-ares:
Copyright 1998 by the Massachusetts Institute of Technology.

c-ares is a C library for asynchronous DNS requests (including name resolves)

http://c-ares.haxx.se/

License: MIT license

http://c-ares.haxx.se/license.html

#### libcurl
Copyright (C) 1998 - 2016, Daniel Stenberg, <daniel@haxx.se>, et al.

The multiprotocol file transfer library

https://curl.haxx.se/libcurl/

License:  MIT/X derivate license

https://curl.haxx.se/docs/copyright.html

#### Crypto++
Copyright (c) 1995-2013 by Wei Dai. (for the compilation) and public domain (for individual files)

Crypto++ Library is a free C++ class library of cryptographic schemes.

https://www.cryptopp.com/

License: Crypto++ Library is copyrighted as a compilation and (as of version 5.6.2) 

licensed under the Boost Software License 1.0, while the individual files in 
the compilation are all public domain.

#### OpenSSL
Copyright (c) 1998-2016 The OpenSSL Project.  All rights reserved.

A toolkit implementing SSL v2/v3 and TLS protocols with full-strength cryptography world-wide.

https://www.openssl.org/

License: OpenSSL License

https://github.com/openssl/openssl/blob/master/LICENSE

#### libuv
Copyright Joyent, Inc. and other Node contributors. All rights reserved.

libuv is a multi-platform support library with a focus on asynchronous I/O.

https://github.com/libuv/libuv

License: MIT

https://github.com/libuv/libuv/blob/v1.x/LICENSE

--------------------------------------------------------------------
