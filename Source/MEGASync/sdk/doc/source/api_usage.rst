**************
MEGA API Usage
**************

.. go and use the C++ domain
   http://sphinx-doc.org/domains.html#id1
   see: Transferring Files


Method/Callback Overview
========================

.. image:: ../../api/html/classMegaClient__coll__graph.png
   :width: 100%
   :alt: Collaboration diagramme for the MegaClient class


Hashing a Password
------------------

Hashes a UTF-8-encoded password and stores the result in the supplied
buffer.

XXX is this really hashpw_key(const char* password, char* hash)

:Method:
    .. doxygenfunction:: MegaClient::pw_key
       :no-link:


Logging into an Account
-----------------------

Initiates a session login based on the user e-mail address (case
insensitive) and the corresponding password hash.

:Method:
    .. doxygenfunction:: MegaClient::login
       :no-link:
       :outline:

.. ``void login(const char* email, const char* hash)``

:Callback:
    ``login_result(error e)``

:Error codes:
    :``API_ENOENT``: Invalid e-mail address or password.
    :``API_EKEY``: Private key could not be decrypted.


Logging into an Exported Folder
-------------------------------

:Method:
    ``void folderaccess(const char* node, const char* key)``

:Callback:
    none (does not interact with the server, proceed with
    calling ``fetchnodes()``)


Fetching the Session's Nodes and Users
--------------------------------------

:Method:
    ``void fetchnodes()``

:Callback:
    ``fetchnodes_result(client, error e)``

Upon successful completion, also calls `nodes_updated()``.


Retrieving user Account Details, Quota and History
--------------------------------------------------

:Method:
    ``void getaccountdetails(AccountDetails* result, int storage, int transfer, int pro, int transactions, int purchases, int sessions)``

Updates the supplied ``AccountDetails`` structure with information on
current storage and transfer utilization and quota, Pro status, and
the transaction and session history. You can specify which types of
information you are interested in by setting the related flag to a
non-zero value.


Changing the Logged in Account's Password
-----------------------------------------

:Method:
    ``error changepw(const char* currentpwhash, const char* newpwhash)``

:Callback:
    ``changepw_result(error e)``

:Returns:
    :``API_EACCESS``: If no user session exists.


Updating a Node's Attributes (with instant completion)
------------------------------------------------------

:Method:
    ``error setattr(Node* node, const char** newattr)``

:Returns:
    :``API_EACCESS``: If node not writable..

:Callback:
    ``setattr_result(handle nodehandle, error e)``

The node's current attributes are pushed to the server. The optional
``newattr`` parameter specifies attribute deltas as
``NULL``-terminated sequence of attribute name/attribute C-string
pointer pairs. In ``setattr_result()``, ``nodehandle`` is the node's
handle.


Moving a Node to a New Parent Folder (with instant completion)
--------------------------------------------------------------

:Method:
    ``error rename(Node* node, Node* newparent)``

:Returns:
    :``API_EACCESS``: If node's parent or newparent are not
        ritable (with full and read/write access, respectively) or the
        move would be between different user accounts.
    :``API_ECIRCULAR``: If a circular linkage would result..

:Callback:
    ``rename_result(handle nodehandle, error e)``

The node (along with all of its children) is moved to the new parent
node, which must be part of the same user account as the node
itself. You cannot move a mode to its own subtree.


Deleting a Node Tree (with instant completion)
----------------------------------------------

:Method:
    ``error unlink(Node* node)``

:Returns:
    :``API_EACCESS``: if the node's parent is not writable
        (with full access)

:Callback:
    ``unlink_result(handle nodehandle, error e)``

The node and all of its sub-nodes are deleted. The node's parent must
be writable (with full access). Affected outbound shares are canceled.


Transferring Files
------------------

Uploads and downloads are started with ``topen()``, which returns a
*transfer descriptor* identifying the transfer. Multiple transfers can
run in parallel, and each transfer can use multiple TCP connections in
parallel. Efficient transfer queuing with pipelining is available.
Uploads can be speed-limited using an absolute or a dynamic cap.

Progress information is conveyed through a callback:

.. cpp:function:: transfer_update(int td, m_off_t bytes, m_off_t size, dstime starttime)

    Provides progress information.

    .. cpp:member:: td

        Identifies the transfer.

    .. cpp:member:: bytes

         Denotes the number of bytes transferred so far.

    .. cpp:member:: size

        Indicates the total transfer size.

    .. cpp:member:: starttime

        Is the time the transfer started.

A running transfer can be aborted at any time by calling ``tclose(int
td)``. Failure-indicating callbacks perform the ``tclose()``
implicitly. The callback indicating a transient HTTP error,
``transfer_error(int td, int httpcode, int count)`` will call
``tclose()`` if the application returns a non-zero value.


Uploading
^^^^^^^^^

:Method: 
   ``int topen(const char* localfilename, int speedlimit, int connections)``

   :``speedlimit``: Maximum upload speed in bytes/second or -1 for
       approx. 90% of the line speed.
   :``connections``: Number of parallel connections to use (default: 3).

:Returns:
    A transfer descriptor on success.
    
    Errors:
    
    :``API_ETOOMANY``: If all transfer channels are busy
    :``API_ENOENT``: If file failed to open.

:Callback (completed, success):
    ``transfer_complete(int td, handle uploadhandle, const byte* uploadtoken, const byte* filekey, SymmCipher* filekey)``

:Callback (failure):
    ``transfer_failed(int td, error e)``


Downloading
^^^^^^^^^^^

:Method:
    ``int topen(handle nodehandle, const byte* key, m_off_t start, m_off_t len, int c)``
    
    :``nodehandle``: The handle of the node to download (if key is set,
        the handle part of the exported file link).
    :``key``: The base64-decoded key part of the exported file link if
        set.
    :``start`` and ``len``: Can be set to download only a slice of the file
        (default: 0, -1 for the full file).
    :``c``: Denotes the number of parallel TCP connections that this
        download should employ.

:Returns:
    A transfer descriptor on success.
    
    Errors:
    
    :``API_ETOOMANY``: If all transfer channels are busy.
    :``API_ENOENT``: If local file is not present.
    :``API_EACCESS``: If attempting to download a non-file.

:Callback (success on opening file for download):
   ``topen_result(int td, string* filename, const char* fa, int pfa)``

    :``td``: Identifies the transfer.
    :``filename``: Name of the file as specified by the node
        attribute ``'n'``.
    :``fa``: File attributes available for this file.
    :``pfa``: Flag that indicates whether the requesting user is
        allowed to write file attributes (i.e., is the file's owner).

:Callback (complete, succcess):
    ``transfer_complete(int td, chunkmac_map* macs, const char* fn)``

    :``td``: Identifies the transfer.
    :``macs``: Contains the MAC hash for each file chunk.
    :``fn``: The filename (UTF-8).

:Callback (failure):
    ``transfer_failed(int td, string& filename, error e)``

:Callback (reached transfer limit):
    ``transfer_limit(int td)``


Transfer Queuing
^^^^^^^^^^^^^^^^

The engine maintains separate upload and download queues, ``putq`` and
``getq``. Transfers are started by pushing transfer objects (classes
``FilePut`` and ``FileGet`` onto these queues). You no longer need to
call ``topen()``/``tclose()`` yourself, but you still need to process
the transfer callbacks. A transfer is considered failed if no bytes
were transmitted for at least ``XFERTIMEOUT`` deciseconds, in which
case it will be aborted and repeated indefinitely with exponential
backoff.

The main benefit of using the engine-supplied transfer queuing is not
the reduced application complexity, but the built-in overlapping
transfer pipelining that reduces the impact of transitions between
files on your overall throughput.


Adding Nodes
------------

Nodes can be added as children of a parent node that you have write
access to (at least read/write) or "dropped" into the inbox of any
registered full account, referenced by its e-mail address or user
handle. If adding the nodes would take the relevant account over its
storage limit, the addition fails.

File nodes can be created by either copying an existing file based on
its node handle (private or public), or by using the handle of a
completed upload.

Folder nodes should always be created from scratch with a fresh random
key.

The ``NewNode`` structure consists of the following fields:

:``nodehandle``: A random unique handle for new folder nodes or a
    valid node handle for file copying.
:``parenthandle``: Must be ``UNDEF`` or reference a folder nodehandle
    in this invocation of ``putnodes()``.
:``type``: Either ``FILENODE`` or ``FOLDERNODE``.
:``source``: ``NEW_NODE``, ``NEW_PUBLIC`` or ``NEW_UPLOAD``.
:``nodekey``: The AES key for this node, RSA-encrypted to the target
    user or AES-encrypted to the session user.
:``ctime`` and ``mtime``: The node's creation and last modification
    times.
:``attrstring``: The encrypted node attribute object.
:``uploadhandle`` and ``uploadtoken``: For upload-based file creation:
    Upload handle and token.

An array of ``NewNode`` structures is passed to ``putnodes()``. This
array may be needed after the call has already returned. Due to its
potentially large size, is not being copied internally, so **always**
allocate this array from the heap and free it in
``putnodes_result()``.


Adding Children to a Node
^^^^^^^^^^^^^^^^^^^^^^^^^

:Method:
    ``void putnodes(handle parenthandle, NewNode* newnodes, int numnodes)``

    :``parenthandle``: Handle of the new nodes' parent node.

The nodes are added below the node designated by ``parenthandle``.


Dropping Nodes into a User's Inbox
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:Method:
    ``void putnodes(const char* user, NewNode* newnodes, int numnodes)``

    :``user``: Recipient user's e-mail address.

The call will fail if the specified address does not denote a MEGA
account with a valid RSA key pair.


File Attributes
---------------

File attributes can be attached to an existing file by the original
uploader of the file only. They can also be attached to an upload
while it is still running, ensuring that they are present at the time
the upload completes.

Existing file attributes can be bulk-fetched for a set of nodes and a
given type. The attributes will be decrypted by the engine and fed to
the application through a series of callbacks.


Storing File Attributes
^^^^^^^^^^^^^^^^^^^^^^^

:Method:
    ``void putfa(SymmCipher* filekey, handle th, fatype t, const byte* data, unsigned len)``

    :``filekey``: Crypto key for the file (the attributes will be
        padded to the cipher's block size and then CBC-encrypted using
        that key).
    :``th``: Target handle -- either an existing node handle or an
        upload handle.
    :``t``: Attribute type (e.g. 0 for a 120 x 120 pixel JPEG
        thumbnail).
    :``data`` and ``len``: Attribute data.

:Callback:
    ``putfa_result(handle th, fatype t, error e)``


Retrieving File Attributes
^^^^^^^^^^^^^^^^^^^^^^^^^^

:Method:
    ``error getfa(Node* node, fatype t, int cancel)``

    Submits or cancels a file attribute retrieval request.

:Returns:
    :``API_ENOENT``: No such attribute for this node.

:Callback:
    ``int fa_failed(handle th, fatype t, int count)``

    Indicates that this attribute could not be fetched for ``count``
    times. It will be retried with exponential backoff. Returning a
    non-zero value will cancel the retrieval for this attribute.

:Callback:
    ``int fa_complete(Node* node, fatype t, const char* data, int len)``

    Passes the decrypted file attribute back to the application.


User Contact Management
-----------------------

Users maintain a *contact list* of peers they know.

:Method: 
    ``error addcontact(const char* user)``

    :``user``: User by e-mail address or handle. If the user has an
        existing MEGA account, he or she will appear in the local
        contact list immediately. Otherwise, an e-mail invitation will
        be sent.

Receiving a folder share from a user implicitly adds him or her to the
recipient's contact list.


Folder Sharing
--------------

Folder shares can be created with three access levels: ``RDONLY``
(readonly), ``RDWR`` (read/write) or ``FULL`` (full - rename and
delete allowed).

To remove an existing folder share, specify ``ACCESS_UNKNOWN``.

:Method:
    ``void setshare(Node* node, const char* user, accesslevel access)``

    :``node``: Must reference a folder node in the session user's own
        trees (you cannot share a node from an inbound share).
    :``user``: User identifier or e-mail address. If the user has a
        MEGA account with valid RSA key pair, the share is created and
        available instantly. Otherwise, he receives an invitation
        e-mail, and the share is only active after he signs up and
        successfully requested and received the share-specific AES key
        from you.
    :``access``: Specifies the desired new access level or share
        removal.

:Callback:
    ``share_result(error e)``
    
    The share request failed entirely.

:Callback:
    ``share_result(int index, error e)``

     For aggregate multi-peer share requests, indicates a problem with
     that particular peer. Currently, index is always 0, as the client
     access engine only issues single-peer requests.


Exporting Nodes
---------------

To enable unauthenticated access to a file or folder node, the node
has to be exported. An existing export can be removed. Nodes in
incoming shares cannot be exported. To decrypt the node and file data
of an exported file node, the related node key is required. Decrypt of
the node tree under an exported folder node occurs by way of the
node's sharekey.

:Method:
    ``error exportnode(Node* node, int del)``

:Returns:
    :``API_EACCESS``: Node is in an inbound share.

:Callback (success):
   ``exportnode_result(handle nodehandle, handle exportedhandle)``

:Callback (failure):
    ``exportnode_result(error e)``


Checking Session Status
-----------------------

:Method: ``int loggedin()``

:Returns:
    1 if account session exists, 0 otherwise.


Reloading Session State
-----------------------

This method should be invoked in response to a ``reload()`` callback.

:Method:
    ``void reload()``


Terminating User Session or Folder Login
----------------------------------------

:Method: 
   ``void logout()``


Verifying Node access
---------------------

:Method:
    ``int checkaccess(Node* node, accesslevel access)``

:Returns:
    1 if requested access is allowed, 0 otherwise.


Verifying node move
---------------------------------

:Method:
    ``error checkmove(Node* node, Node* newparent)``

:Returns:
    ``API_OK``, ``API_EACCESS`` or ``API_ECIRCULAR``


Exponential Backoff Handling
----------------------------

To prevent infrastructure overload during network or server outages,
all engine-initiated network actions are protected by a mechanism
called exponential backoff - the interval between two retries doubles
upon every failed attempt.

:Callback: 
   ``notify_retry(dstime dsdelta)``

    Indicates failure of the last interaction with the MEGA API
    servers and the time in deciseconds until the next attempt. The
    application should show the user a countdown and allow him to
    manually trigger a new attempt.

:Method: 
   ``void abortbackoff()``

    Resets all exponential backoff timers and immediately retries the
    pending operation(s).


Asynchronous Completion Tracking
--------------------------------

Application programmers may need to keep track of which callbacks
relate to which action.


Asynchronous Command Tracking
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To make a request trackable (only those methods that can result in
callbacks are trackable), call ``int nextreqtag()`` and record the
returned value for later cross-reference. In every callback that
pertains to this request, the ``restag`` member of the ``MegaClient``
object will hold the same value.


File Transfer Tracking
^^^^^^^^^^^^^^^^^^^^^^

File transfers are trackable by calling ``int nextreqtag()`` before
``topen()``. The returned value can then be found in ft[td].tag in the
``MegaClient`` object.


Prefix and Encrypt JSON Object (for use as node attribute string)
-----------------------------------------------------------------

:Method:
    ``makeattr(cipher, targetstring, jsonstring, length)``


Request for new ``FileAccess`` Object
-------------------------------------

XXX check signature

:Callback:
    ``FileAccess newfile()``

:Returns:
    Application-specific ``FileAccess`` object.


User update notification
---------------------------------

:Callback:
    ``users_updated(User** users, unsigned count)``

    Users were added or updated (users are never deleted during a
    session).


Node Update Notification
------------------------

:Callback:
    ``nodes_updated(Node** nodes, unsigned count)``

    Nodes were added, updated or removed.

:Callback: 
   ``notify_retry(time)``

    Specifies time in decisecons until the next retry.

:Callback:
    ``reload(reason)``

    Possible inconsistency, reload account state.


Error Codes
===========

.. doxygenenum:: error
   :no-link:
