**************
Under the Hood
**************

Knowledge of the following low-level details is not required to
successfully develop MEGA client applications. It is provided as a
reference to those who are interested in fully understanding how
MEGA's API works.


Communication Protocol
======================

The MEGA API is based on a simple HTTP/JSON request-response
scheme. Requests are submitted as arrays of command objects and can be
initiated by both the client and the server, effectively resulting in
bidirectional RPC capability. To see the raw request flow in the SDK's
megaclient sample app, use the debug command.


Cryptography
============

All symmetric cryptographic operations are based on AES-128. It
operates in cipher block chaining mode for the file and folder
attribute blocks and in counter mode for the actual file data. Each
file and each folder node uses its own randomly generated 128 bit
key. File nodes use the same key for the attribute block and the file
data, plus a 64 bit random counter start value and a 64 bit meta MAC
to verify the file's integrity.

Each user account uses a symmetric master key to ECB-encrypt all keys
of the nodes it keeps in its own trees. This master key is stored on
MEGA's servers, encrypted with a hash derived from the user's login
password.

File integrity is verified using chunked CBC-MAC. Chunk sizes start at
128 KB and increase to 1 MB, which is a reasonable balance between
space required to store the chunk MACs and the average overhead for
integrity-checking partial reads.

In addition to the symmetric key, each user account has a 2048 bit RSA
key pair to securely receive data such as share keys or file/folder
keys. Its private component is stored encrypted with the user's
symmetric master key.


Shared Folders
==============

The owner of the folder is solely responsible for managing access;
shares are non-transitive (shares cannot be created on folders in
incoming shares). All participants in a shared folder gain
cryptographic access through a common share-specific key, which is
passed from the owner (theoretically, from anyone participating in the
share, but this would create a significant security risk in the event
of a compromise of the core infrastructure) to new participants
through RSA. All keys of the nodes in a shared folder, including its
root node, are encrypted to this share key. The party adding a new
node to a shared folder is responsible for supplying the appropriate
node/share-specific key. Missing node/share-specific keys can only be
supplied by the share owner.


Unauthenticated Delivery
========================

MEGA supports secure unauthenticated data delivery. Any fully
registered user can receive files or folders in their inbox through
their RSA public key.


Login Sessions
==============

Each login starts a new session. For regular accounts, this involves
the server generating a random session token and encrypting it to the
user's private key. The user password is considered verified if it
successfully decrypts the private key, which then decrypts the session
token.

To prevent remote offline dictionary attacks on a user's password, the
encrypted private key is only supplied to the client if a hash derived
from the password is presented to the server.


API Request Flow and Execution
==============================

API requests flow in two directions:

* Client --> server and
* Server --> client.

Client-server requests are issued as HTTP ``POST`` with a raw JSON
payload. A request consists of one or multiple commands and is
executed as a single atomic, isolated and consistent transaction - in
the event of a request-level error response, no data was
altered. Requests are idempotent -- sending the same request multiple
times is equivalent to sending it once, which makes it safe to retry
them, e.g. in case of intermittent network issues. Each request must
therefore be tagged with a session-unique identifier (e.g., a sequence
number) to prevent inadvertent cache hits caused by preceding
identical requests.

While a request is executed, all users that may be affected by it are
locked. This includes the requesting user and all users that are in a
shared folder relationship and/or in the contact list. A request may
return the error code ``EAGAIN`` in the event of a failed locking
attempt or a temporary server-side malfunction. The request is likely
to complete when retried. Client applications must implement
exponential backoff (with user-triggerable immediate retry) and should
inform the user of a possible server or network issue if the
``EAGAIN`` condition persists or no response is received for more than
a few seconds.

A successfully executed request returns an array of result objects,
with each result appearing in the same array index location as the
corresponding command.


Request URL Format
------------------

Target URL:

``https://g.api.mega.co.nz/cs?id=sequence_number&ak=appkey&[&sid=sessionid|&n=node]``

:``sequence_number``: Session-unique number that is incremented per
    dispatched request (but not changed when requests are repeated in
    response to network issues or ``EAGAIN``).
:``appkey``: Application's key.
:``sessionid``: Session ID of the user session authentication mode.
:``node``: Node token of the file system tree authentication mode.


``POST`` JSON Request Payload Structure
---------------------------------------

The JSON object shall be sent as the payload of a raw ``POST``
request. No additional framing shall take place. The Content-Type HTTP
header is not processed, but should be set to application/json.

Its structure is an array of commands: ``[cmd1, cmd2, ...]``

with ``cmd = { a : command type, [argument : value]* }``.


``POST`` JSON Response Payload Structure
----------------------------------------

The response to the request is a raw JSON object of content-type
``application/json``.

It is structured as single number (e.g. -3 for ``EAGAIN``) in the case
of a request-level error or as an array of per-command return objects:
``[res1, res2, ...]``

To prevent infrastructure overload, dynamic rate limiting is in
effect. Before a request is executed, the total "weight" of the
commands it contains is computed and checked against the current
balance of the requesting IP address. If the total exceeds a defined
threshold, the request is rejected as a whole and must be repeated
with exponential backoff


Server-Client Requests
======================

As a server cannot reliably establish a connection to a client,
server-client requests have to be polled by the latter through a
blocking read loop.


Request URL Format
------------------

Target URL:

``https://g.api.mega.co.nz/sc?id=sequence_reference[&sid=sessionid|&n=node][&ssl=1]``

:``sequence_reference``: Tells the server which server-client
    request(s) to deliver next. It is initialized from the response to
    a file system tree fetch (``f`` command).
:``sessionid``: Session ID of the user session authentication mode.
:``node``: Node token of the file system tree authentication mode.
:``ssl=1``: Forces an HTTPS URL for the returned wait_url (which is
    needed for most browsers, but not in an application context).


``POST ``JSON Response Payload Structure
----------------------------------------

A request level error is received as a single number (e.g. -3 for
``EAGAIN``) or a raw JSON object with content-type
``application/json``. Its structure is as follows:

``{ a : [req1, req2, ...], [ sn : sequence_reference | w : wait_url ] }``

:``reqN``: N-th server-client request.
:``sequence_reference``: Updates the sequence reference that is used
    the invocation of the ``/cs`` request URL.
:``wait_url``: Requests that the client connects to this (potentially
    long) URL, which will block until new requests are ready for
    delivery. Once it disconnects (with an HTTP 200 OK response and a
    content-length of 0), the polling process shall loop back to
    fetching new requests, using the current ``sequence_reference``.


JSON Data Encoding
==================

As JSON is not binary clean, all non-ASCII data has to be encoded. For
binary data, the MEGA API uses a variation of base-64 with the
characters ``-_`` used instead of ``+/`` and the trailing ``=``
stripped (where necessary, the actual payload length is heuristically
inferred after decoding, e.g. by stripping trailing ``NULL``
bytes). Unicode text has to be encoded as UTF-8.


API Data Types
==============

The MEGA API uses the following major data types:


Node Handles
------------

Node handles are eight alpha-numeric characters in length and case
sensitive.


User Handles
------------

User handles consist of eleven base-64 characters.


Encryption Keys
---------------

Encryption keys are always base-64-encoded. The following key types
exist:

* Symmetric AES keys (22 characters)
* Folder node keys (22 characters)
* File node keys (43 characters)
* RSA public/private keys (2048 bit: 348/875 characters)
* Node and file keys in a share context are transmitted in a compound
  per-share format: ``sharehandle:key/sharehandle:key/...`` -- each
  key is encrypted to its corresponding share handle


File Encryption
===============

MEGA uses client-side encryption/decryption to end-to-end-protect file
transfers and storage. Data received from clients is stored and
transmitted verbatim; servers neither decrypt, nor re-encrypt, nor
verify the encryption of incoming user files. All cryptographic
processing is under the control of the end user.

To allow for integrity-checked partial reads, a file is treated as a
series of chunks. To simplify server-side processing, partial uploads
can only start and end on a chunk boundary. Furthermore, partial
downloads can only be integrity-checked if they fulfil the same
criterion.

Chunk boundaries are located at the following positions:

0, 128K, 384K, 768K, 1280K, 1920K, 2688K, 3584K, 4608K, ... (every
1024 KB), EOF

A file key is 256 bits long and consists of the following components:

* A 128 bit AES-128 key ``k``
* The upper 64 bit ``n`` of the counter start value (the lower 64 bit
  are starting at 0 and incrementing by 1 for each AES block of 16
  bytes)
* A 64 bit meta-MAC ``m`` of all chunk MACs

A chunk MAC is computed as follows (this is essentially CBC-MAC, which
was chosen instead of the more efficient OCB over intellectual
property concerns):

* ``h := (n << 64) + n``
* For each AES block ``d``: ``h := AES(k, h XOR d)``
* A chunk is encrypted using standard counter mode:
* For each AES block ``d`` at block position ``p``: ``d' := d XOR
  AES(k, (n << 64) + p)``
* MAC computation and encryption can be performed in the same loop.

Decryption is analogous.

To obtain the meta-MAC ``m``, apply the same CBC-MAC to the resulting
block MACs with a start value of 0. The 64 bit meta-MAC ``m`` is
computed as ``((bits 0-31 XOR bits 32-63) << 64) + (bits 64-95 XOR
bits 96-127)``.


Uploads
=======

Uploads are performed by ``POST``-ing raw data to the target URL
returned by the API ``u`` command. If so desired, an upload can be
performed in chunks. Chunks can be sent in any order and can be of any
size, but they must begin and end on a chunk boundary. The byte offset
``x`` of a chunk within the file is indicated by appending ``/x`` to
the URL. Multiple chunks can be sent in parallel. After a chunk
completes, the server responds with a status message, which can be:

:Empty: Successful receipt.
:Completion handle: A 27-character base-64-encoded string that must be
    used in conjunction with the ``p`` (put node) API command to
    complete the upload.
:Error code: A (negative) number in decimal ASCII representation, typically
    requiring a restart of the upload from scratch.

The per-upload encryption key must be generated by a strong random
number generator. Using a weak one will undermine the confidentiality
and integrity of your data.


Downloads
=========

TCP throughput on high-latency links is adversely affected by slow
congestion window growth, insufficient send or receive buffer size and
(even mild) packet loss. All of these factors can be mitigated by
using multiple transfer connections in parallel. Client applications
are encouraged to offer users to configure up to six parallel
connections in each direction. The recommended default value is four.


HTTPS vs. HTTP
==============

All MEGA servers support HTTPS access -- this is due to many web
browsers enforcing a policy where HTTP requests cannot be made from an
HTTPS page at all (IE, Firefox 18+) or at least trigger a visual
warning (Chrome, Firefox until 17). However, only two types of
requests actually benefit from and therefore require HTTPS: The
loading of https://mega.co.nz/index.html and the API request
interface. Neither the hash-protected loading of static ``.html`` and
``.js`` components, nor the waiting for new server-client requests,
nor already encrypted and MAC'ed data transfers from and to the
storage cluster benefit from HTTPS in any meaningful way. Client
applications are therefore required to use SSL for access to the API
interface, but strongly discouraged from doing so for wait requests
and bulk file transfers.

MEGA's HTTPS access supports most ciphers/hashes and uses strong 2048
bit RSA where SSL is relevant (i.e. on the root HTML and the API
servers) and RC4/MD5 with CPU-saving 1024 bit RSA where it is not
(i.e. on static HTML and storage servers).

PFS ("perfect forward secrecy") is supported on the API servers only,
because secrecy is not required for public static content.
