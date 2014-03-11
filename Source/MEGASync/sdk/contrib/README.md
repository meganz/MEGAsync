Contributions
=============

Eclipse
-------

Contains configurations, formatters and templates to ease the use of
the Eclipse IDE with the C/C++ Development Tools.

(See `contrib/Eclipse/README.md`)


Static Code Check
-----------------

### N'SIQ CppStyle

N'SIQ CppCheck is a static code and code style checker. It is
available as open source from here:

https://code.google.com/p/nsiqcppstyle/

Checks can simply be run using the given list of checks as following:

    nsiqcppstyle -f contrib/nsiq_filefilter.txt .

Or to integrate it into Jenkins CI (see also
https://code.google.com/p/nsiqcppstyle/wiki/HudsonIntegration):

    nsiqcppstyle --ci --output=xml -f contrib/nsiq_filefilter.txt .

A URL with further information on the different checks used is given
at the top of the file configuration file `nsiq_filefilter.txt`.
Further rules for checks can be implemented in simple Python files.


### Cppcheck

Cppcheck is a static code checker for C++ (Debian/Ubuntu package
`cppcheck`).  It can very easily be integrated into Eclipse through
the `cppcheclipse` extension (from Eclipse Marketplace).

For integration into `vim` use the file `vimcppcheck.vim` included.

For manual checks just run the make target `cppcheck`:

    make cppcheck


### Code Formatter

Automatic code formatting can be performed according to our specified
rules using the uncrustify tool (Debian/Ubuntu package `uncrustify`).
For other platforms from here:

http://uncrustify.sourceforge.net/

To format a single file `<file>` into `<file>.uncrustify`:

    uncrustify -c contrib/uncrustify.cfg <file>

To format many files *in place*

    find src/ -type f -name "*.cpp" -exec \
        uncrustify --replace \
        -c ~/workspace/megasdk/contrib/uncrustify.cfg {} \;
    find include/ -type f -name "*.h" -exec \
        uncrustify --replace \
        -c ~/workspace/megasdk/contrib/uncrustify.cfg {} \;

*Note:* Sometimes the uncrustify tool terminates with a segmentation
fault. In those cases there will be an empty (0 bytes) file
`<file>.uncrustify`.  These files will need to be sorted out manually!
