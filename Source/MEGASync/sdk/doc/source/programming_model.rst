*****************
Programming Model
*****************

Interaction
===========

The application submits requests to the client access engine through
nonblocking calls to methods of the ``MegaClient`` object and signals
events to the application by invoking methods of an object of its
implementation of the MegaApp interface.


Files and Folders
=================

Files and folders are represented by ``Node`` objects and referenced by
node handles. Nodes point to parent nodes, forming trees. Trees have
exactly one root node (circular linkage is not allowed). Node updates
(caused by the session's own actions, other sessions of the same
account or other accounts, e.g. through activity in a shared folder)
are notified in real time through a callback specifying the affected
nodes. Deleted nodes are first notified with their removed flag set
before being purged to give the application an opportunity to remove
them from the UI view.

There are at least three node trees per account: Root, incoming and
rubbish. Additional trees can originate from other users as shared
folders.


Users and Contacts
==================

Users are referenced by their user handle and/or their primary e-mail
address. The engine maintains a ``User`` object for every user account
that has appeared in the context of the current session: As a contact
or merely as the owner of a file system node. A visibility flag turns
a user into a contact if set.

User attributes can be used to store credentials such as avatar
pictures, address, date of birth etc. It is recommended to store
application-private user credentials AES-CBC encrypted.


Asynchronous Completion
=======================

All engine methods are nonblocking -- waiting for network
communication to complete is not an option. Instead, they merely
initiate the desired action, the outcome of which is eventually
signalled through a callback. The application should be robust against
such callbacks arriving after a long time (i.e., many seconds). Failed
requests (e.g. due to network issues) are retried automatically with
exponential backoff. The application receives notification of this
through a callback and should inform the user accordingly, along with
a UI element to manually initiate a retry.

Three types of operation are subject to acceleration by a mechanism
called "speculative instant completion": Node attribute updates, moves
and deletions. As these merely receive a highly predictable "OK" or
"failure" response from the API, there is some benefit in immediately
updating the local nodes and reloading the session state in the rare
event of an inconsistency. The engine loosely protects them with an
access check that is following the same semantics as the authoritative
check on the API server side.


Concurrency Considerations
==========================

Shared access to a resource (writable nodes accessible to the user's
account) based on a potentially outdated (due to network latency) view
is naturally prone to race conditions, leading to inconsistencies when
two parties make conflicting updates within the latency window. The
engine contains heuristics to detect these conflicts and will ask the
application to discard and reload its view if needed (the user should
be informed accordingly).


Event Processing and Threading
==============================

Due to its nonblocking nature, the MEGA client access engine
integrates extremely well with single-threaded applications (although
on platforms without a nonblocking DNS lookup facility, you may not
get around using a worker thread for name resolution). If you,
however, prefer to use multiple threads, you are welcome to do so - as
long as you ensure that no two threads get to enter the engine or
access its data structures at the same time.


Running the Engine
==================

There are three approaches to integrating the engine with the
application. The goal is to have the engine get the CPU (through
``MegaClient``'s ``exec()`` method) swiftly whenever one of its wakeup
triggers fires:

* block inside the engine's own blocking callback (which waits for
  socket I/O and timeouts) and include the application's own wakeup
  triggers
* record the engine's wakeup triggers and include them in the
  application's existing blocking facility
* dedicate a worker thread to the MEGA engine and interact with the
  application through e.g. a bidirectional message queue (inefficient,
  but the only option if, for some reason, you cannot modify the
  application's event processing)


File Name Conflicts
===================

Applications must be prepared to deal with file and folder name
clashes. In many scenarios, this is trivial -- the user sees all
copies and makes the decision which file he is interested in, mostly
based on its timestamp. Some applications, however, map a MEGA node
tree to a resource that uses file paths as unique keys, e.g. file
systems. In this case, we recommend that only the most recent node is
used.

File name characters that are not allowed on the host must be
urlencoded using ``%xx``. When writing files back to the server, valid
urlencoded sequences must be replaced with the encoded character. This
has the potential unwanted side effect of mangling file names that
originally contained valid ``%xx`` sequences, but this should be rare,
and they'll be unmangled when read back to the local machine.


Namespace Management: Node, File and User Attributes
====================================================

We kindly ask all application developers wishing to introduce new
node, file or user attributes to coordinate the numbering/naming and
formatting conventions with us to maximize cross-application
interoperability. Should you elect not to do so, please avoid
cluttering the namespaces and prefix the new attribute names with your
abbreviated company or application name.
