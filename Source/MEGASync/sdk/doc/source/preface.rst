*********************************************
Welcome to the MEGA Client SDK Documentation!
*********************************************

Welcome to the MEGA SDK! We hope that it will prove useful to
developers who are interested in integrating MEGA support into their
applications.


What is the MEGA SDK good for?
==============================

The MEGA SDK consists of code and documentation that enables you to
make use of MEGA's API functionality at a comfortably high level of
abstraction. Its core component -- a code module called *client access
engine* -- maintains a current copy of the user's account in memory
(which includes all relevant files, folders, contacts and shares),
accepts commands from the application and notifies the application of
command results and other updates through callbacks.


How do I use the MEGA client access engine?
===========================================

The MEGA client access engine comes as a set of C++ classes and
interfaces. If you are using C++, you can simply add them to your
project. You then instantiate the ``MegaClient`` class (which holds
the session state) and pass it an instance of your implementation of
the ``HttpIO`` interface (which handles network requests and blocking)
and ``MegaApp`` (through which you receive the engine's callbacks).

The core code is reasonably platform independent (if you encounter any
issues with your specific C++ compiler environment, please let us
know). To illustrate practical usage, a sample application (a basic
ftp-style interactive console client) is included.


Why do you provide a code module rather than documenting the API?
=================================================================

Isn't a network API interface in detail description sufficient for me
to implement it myself?

Two reasons for a client implementation:

* Complexity/efficiency -- Since all of MEGA's crypto logic runs on
  the client side, you'd be looking at a project exceeding 5,000 lines
  of code. And, as natural language is rather inefficient when it
  comes to specifying algorithms, the documentation would be similarly
  voluminous.
* Consistency/interoperability -- Ambiguities in the specification or
  its imprecise interpretation would inevitably lead to undesired
  behavioural differences between implementations.


Thanks, but why C++?
====================

I am using C, Objective C, C#, Java, Scala, Python, Ruby, Perl, PHP,
VB, ...

The requirement to integrate with projects that compile to native code
rules out all languages that rely on specific interpreters or runtime
environments. C, being the "lingua franca" of nearly all modern
systems, would have been the obvious choice, but the code compactness
and readability benefits provided by C++'s syntactic sugar and
template library are well worth the minor additional integration
overhead. We will work with interested developers to add MEGA support
to their preferred environments by way of native code
modules/extensions (rather than by porting the functionality to the
target language itself). Please contact us at developers@mega.co.nz if
you are willing and able to contribute to a particular integration
effort.
