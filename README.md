1. Overview
-----------

The office-tools is Harmatan UI for viewing PDF and office files.
For PDF files the Poppler's QT4 rendering library is used.
KOffice is used for rendering office files.


2. Building
-----------
To build office-tools you need:
- DUI
- poppler-qt4
- kofficefrem (?)

2.1 Debug build
---------------
Building with debugging options:

    qmake
    make

2.2 Release build
-----------------
Building with release options:

    qmake "CONFIG+=release"
    make

2.3 Building without KOffice
----------------------------
If you are missing KOffice then you can build PDF viewer as following:

    qmake "CONFIG+=noOffice"
    make

3. Making documentation
-----------------------
To make the documentation you need 
- doxygen
- graphviz

And for customizing documentation use doxygen-gui/doxywizard.

Making the documentation:

    qmake
    make doc

4. Making unit testing
----------------------
Building with debugging options:

    qmake
    make install
    make test
