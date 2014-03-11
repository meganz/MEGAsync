*****************************
SDK Sample Client Application
*****************************

Compiling and Linking ``megaclient``
====================================

To successfully build the ``megaclient`` CLI (command line interface)
client, you need the following:

* A POSIX or Win32 compliant environment
* A C++ compiler with STL
* ``libcurl`` **(built with c-ares support)**
* A recent version of FreeImage
* ``libcrypto++``
* ``libreadline``

At this time, the SDK does not come with an ``autoconf`` script, so
you may have to manually adapt the Makefile to your system.


Using ``megaclient``
====================

Similar to the UNIX ``ftp`` command, ``megaclient`` displays a prompt
and accepts commands to log into a MEGA account or exported folder
link, list folder contents, create folders, copy, move and delete
files and folders, view inbound and outbound folder shares, establish,
modify and delete outbound folder shares, upload and download files,
export links to folders and files, import file links, view the
account's status and change its password.


Command Reference
=================

``login`` -- Log into an Account or Folder Link
-----------------------------------------------

To log into an account, specify the account's e-mail address and
optionally the password (for which you will be prompted otherwise).

To log into an exported folder link, specify the full link, including
the key (prompting for the key has not been implemented yet).

Note that exported folders cannot be written to.


``mount`` -- Display Available File System Trees
------------------------------------------------

This command takes no parameters and displays the paths of your
available file system trees (typically ``/`` for the main tree,
``//in`` for the inbox, ``//bin`` for the rubbish bin and
``email:sharename`` for inbound shares from user ``email``.


``ls`` -- List Files and Folders
--------------------------------

``ls`` lists the contents of the current folder (if given with the
qualifier ``-R``, it does so recursively) or the specified path.

Paths can be relative or absolute. Valid absolute paths start with one
of the prefixes displayed by the mount command.

File and folder properties are displayed along with their names: File
size and available file attributes, or exported folder links and
outbound folder shares.


``cd`` -- Change Current Working Directory
------------------------------------------

``cd`` changes the current working directory to the specified folder
path. If no path is given, it changes to ``/``.


``pwd`` -- Display Current Folder
---------------------------------

``pwd`` displays the current folder as an absolute path.


``lcd`` -- Change Current Local Working Directory
-------------------------------------------------

``lcd`` changes the current local working directory to the specified path.


``get`` -- Add File(s) to the Download Queue
--------------------------------------------

``get`` queues the specified file for download. If you specify a
folder, all files contained in the folder are queued for download (but
not the contents of its subfolders).


``put`` -- Add File(s) to the Upload Queue
------------------------------------------

``put`` queues the specified file for upload. Patterns are supported
-- ``put *.jpg`` will upload all ``.jpg`` files present in the current
local directory.


``getq`` and ``putq`` -- Manage Download and Upload Queues
----------------------------------------------------------

The queued transfers are listed with an index, target path (uploads
only) and activity status. To cancel a transfer, specify its index.


``mkdir`` -- Create Folders
---------------------------

``mkdir`` creates an empty subfolder in the specified (or current)
folder. Although MEGA permits identical folder names, ``mkdir`` fails
if the folder already exists.


``cp``, ``mv`` -- Copy or Move, Rename Files and Folders
--------------------------------------------------------

The specified source path or folder is copied or moved to the
destination folder. If the destination indicates a new name in an
existing folder, a rename takes place along the way.


``rm`` -- Delete File or Folder
-------------------------------

``rm`` deletes the specified file or folder. If the folder contains
files or subfolders, these are recursively deleted as well. All
affected outbound shares and exported folder links are canceled in the
process.

The deletion is final. To take advantage of the rubbish bin
functionality, use ``mv path //bin`` instead.


``share`` -- Manage Outbound Folder Shares
------------------------------------------

``share`` lists, creates, updates or deletes outbound shares on the
specified folder. The folder cannot be in an inbound share.

* To list the existing shares on a folder, use ``share path``.
* To cancel a folder share to a user, use ``share path email``.
* To create or modify a folder share, use ``share path email access``.

Supported access levels are: Read-only (``r``), read/write (``rw``)
and full (``full``).


``export`` -- Create or Cancel File or Folder Link
--------------------------------------------------

``export`` creates a read-only file or folder link that contains the
related encryption key. To cancel and existing link, add the keyword
``del``.


``import`` -- Import an Exported File
-------------------------------------

``import`` adds the file described by the supplied link (importing
folder links is currently unsupported) to the current folder.


``putbps`` -- Set Upload Speed Limit
------------------------------------

Uploading through a DSL line can cause significant outbound packet
loss. You can limit the send rate by specifying an absolute maximum in
bytes per second, ``auto`` to have the server figure out your line
speed and leave approximately 10% of it idle, or ``none`` to transfer
at full speed (default: ``none``).

It is currently not possible to change the send rate of an active
upload. The setting will only affect subsequent uploads.


``whoami`` -- Display Account Details
-------------------------------------

``whoami`` displays various account quota, balances and the session
history.


``passwd`` -- Change Account Password
-------------------------------------

``passwd`` prompts for the current password and then asks for the new
password and its confirmation. No password quality checking is
performed.


``retry`` -- Immediately Retry all Pending Operations
-----------------------------------------------------

``retry`` resets all exponential backoff timers.


``recon`` -- Reconnect
----------------------

``recon`` tears down all existing server connections. This has no
effect on ongoing operations other than causing transfers to take
longer due to partially transferred chunks being discarded and having
to be resent.


``reload`` -- Wipe and Reload Account State
-------------------------------------------

``reload`` purges the local state and forces a full reload from the
server. This is useful in response to the detection of a race
condition-related inconsistency between the client's view and the
server state.


``logout`` -- Terminate Current Session
---------------------------------------

``logout`` purges all local session state.


``debug`` -- Toggle Debug Mode
------------------------------

Debug mode outputs HTTP connection activity and the raw JSON API
requests and responses.
