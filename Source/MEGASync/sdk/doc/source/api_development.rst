***************
API Development
***************

Development Process
===================

The release of the C++ SDK is merely a starting point, and it will
evolve over time. We'd like to hear from developers actually using it
in their applications - if you find bugs, design flaws or other
shortcomings, please get in touch at developers@mega.co.nz. MEGA is
hiring, so be prepared to receive an offer if your feedback indicates
that you are a bright mind.

A developer forum and source code repository will be made available
soon.


Future Enhancements
===================

The API in its current form is a starting point and will evolve over
time.


Allow for Existing Files to be Opened for Writing
-------------------------------------------------

Currently, files cannot be modified after creation. This limitation
will be overcome by using encrypted delta files.


Allow for Integrity-Checked Partial Reads
-----------------------------------------

Currently, a file's integrity is verified only after it was downloaded
in full. Chunk MAC verification will enable applications to ensure the
integrity of partial reads.
