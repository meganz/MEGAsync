**********************
Service Implementation
**********************

Interfaces
==========

The following interfaces need to be implemented by the application:

* ``FileAccess`` -- File opening/closing/reading/writing
* ``HttpIO`` -- HTTP requests with SSL support
* ``PrnGen`` -- Cryptographically strong pseudo-random number generator
* ``SymmCipher`` -- AES cryptography
* ``AsymmCipher`` -- RSA cryptography
* ``MegaApp`` -- Callbacks to the application

The SDK provides reference implementations of ``FileAccess`` (using
POSIX [#POSIX]_ calls), ``HttpIO`` (using cURL [#cURL]_) and of
``PrnGen``, ``SymmCipher`` and ``AsymmCipher`` (using Crypto++
[#Cryptopp]_). If you decide to use cURL in your application, please
ensure that it was built with c-ares [#c-ares]_ support for
asynchronous DNS requests. Some platforms (e.g. MacOS, Debian and Red
Hat Linux and derivatives) bundle cURL binaries that were compiled
with threaded-resolver support -- these will **not** work.


Usage
=====

To access MEGA, an application needs to instantiate three classes:

* its own implementation of the ``MegaApp`` interface
* its own implementation of the ``HttpIO`` interface
* ``MegaClient``

Then, the application must call ``MegaClient``'s ``wait()``
immediately before or instead of blocking for events itself. If
``wait()`` wishes to block, it calls ``HttpIO``'s ``waitio()``,
supplying a timeout. The application can either piggyback its own
wakeup criteria onto the socket events/timeout in ``waitio()``, or
record the criteria ``waitio()`` gets waken up by and include these in
its own blocking logic. The supplied SDK example (``megacli.cpp``)
uses the former approach, adding ``fileno(stdin)`` to the ``select()``
``fd`` set to process user input in real time.

The application must call ``MegaClient``'s ``exec()`` at least once
after every wakeup by the ``waitio()`` criteria (it doesn't hurt to
call it too often).


Data Structures
===============

A MEGA session consists of:

* nodes (``MegaClient::nodes``)
* users (``MegaClient::users``)
* active file transfers (``MegaClient::ft``)
* the root nodes' handles (``MegaClient::rootnodes``)
* the logged in user's own handle and e-mail address
  (``MegaClient::me``, ``MegaClient::myemail``)


Nodes
-----

A node has the following properties:

* handle (caveat: the application-side handle bit layout is
  endian-dependent -- do not transfer between systems using different
  CPU architectures!)
* type (file, folder, ...)
* its parent (if any)
* its children (if any)
* file size (if file node)
* file attributes (if file node)
* attributes (such as file/folder name)
* crypto key
* creation and modification times
* owner
* share information (share key, outgoing share peers, incoming share
  properties)
* a generic pointer to private application data
* a removed flag used to notify node deletion

Please refer to the supplied source code for details.


Users
-----

A third-party user is part of the session either because he is in a
contact relationship with the session user, or because he owns at
least one of the session's nodes. A user record is also created when a
share is added to a previously unregistered e-mail address. Users can
be referenced by their handle or by their e-mail address.

User properties:

* handle
* email address (case-insensitive unique key)
* name
* contact visibility
* shared node handles
* public key
* time the user was added as a contact
* user properties


File I/O
========

File Transfers
--------------

Multiple concurrent file transfers are supported. It is strongly
suggested not to run more than one large upload and one large download
in parallel to avoid network congestion (there will be little, if any,
speed benefit). A file transfer can aggregate multiple TCP channels
(recommended starting point: 4) for greater throughput. File transfers
can be aborted at any time by calling
``MegaClient::tclose()``. Significant local network congestion during
uploads is common with ADSL uplinks and can be prevented by enabling
an automatic or fixed rate limit.


Transfer Queuing
----------------

Applications that transfer batches of files should do using the
engine's transfer queuing functionality. It uses pipelining (new
transfers are dispatched approximately three seconds before the end of
the current transfer) to reduce or eliminate the dead time between
files. Failed transfers are retried with exponential backoff.


File Attributes
---------------

Files can have *attributes.* Only the original creator of a file can
update its attributes. All nodes referencing the same *encrypted* file
see the same attributes. Attributes carry a 16-bit type field. The
client access engine supports attaching file attributes during or
after the upload and their bulk retrieval.


Thumbnails
----------

All applications capable of uploading image files *should* add
thumbnails in the process (remember that there is no way for us to do
this on the server side). Thumbnails are stored as type 0 file
attributes and should be 120 x 120 pixel JPEGs compressed to around
3--4 KB. The sample application supplied with the SDK demonstrates how
to do this using the FreeImage [#FreeImage]_ library. As the
extraction of a thumbnail from a large image can take a considerable
amount of time, it is also suggested to perform this in separate
worker threads to avoid stalling the application.


Quota Enforcement
=================

There are two types of quota limitations an application can encounter
during its operation: Storage and bandwidth. MEGA, by policy, is quite
generous on both, which means that only a small fraction of your user
base will ever run out of quota, but it is essential that if it
happens, the situation is handled correctly -- the user needs to be
informed about the reason for his upload or download failing rather
than being left in the dark with what looks like a malfunction. Plus,
you can earn a share of the revenue generated by your application's
user base by linking to the Pro purchase page with your affiliate
[#MEGAaffiliate]_ code.


Storage Quota
-------------

An upload will be rejected with error ``EOVERQUOTA`` if there is not
enough storage quota available to complete it. Once the upload has
started, it will complete, even if you run out of disk space through
other means in the meantime (e.g. by your Pro status expiring). This
error will also occur if your application tries to send files to a
third-party account without sufficient quota.


Bandwidth Quota
---------------

A download attempt will be rejected with error ``EOVERQUOTA`` if the
bandwidth consumption during the past five to six hours plus the
residual size of all running downloads plus the size of the file to
download would exceed the current per-IP bandwidth limit (if any).

In contrast to uploads, running downloads *can* be interrupted with an
out-of-quota error under certain circumstances, which will trigger a
``quota_exceeded()`` callback, and the download will retry
automatically until bandwidth quota becomes available.


API Access Authorization
========================

To access the MEGA API, applications need to present a valid API key,
which can be obtained free of charge [#MEGAsdk]_. Please see the SDK
terms of service for details.


.. rubric:: Footnotes

.. [#POSIX] http://pubs.opengroup.org/onlinepubs/9699919799/
.. [#cURL] http://curl.haxx.se/
.. [#Cryptopp] http://www.cryptopp.com/
.. [#c-ares] http://c-ares.haxx.se/
.. [#FreeImage] http://freeimage.sourceforge.net/
.. [#MEGAaffiliate] https://mega.co.nz/#affiliates
.. [#MEGAsdk] https://mega.co.nz/#sdk
