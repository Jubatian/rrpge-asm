
RRPGE Assembler
==============================================================================

.. image:: https://cdn.rawgit.com/Jubatian/rrpge-spec/00.013.002/logo_txt.svg
   :align: center
   :width: 100%

:Author:    Sandor Zsuga (Jubatian)
:Copyright: 2013 - 2015, GNU GPLv3 (version 3 of the GNU General Public
            License) extended as RRPGEvt (temporary version of the RRPGE
            License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
            root.




Introduction
------------------------------------------------------------------------------


The RRPGE Assembler is a simple 2 + 1 pass assembler suited for the
construction of RRPGE applications. It has no support for object files: it
generates an application binary directly from the sources.

The first two pass are the usual passes of an assembler, the role of the
second pass being the substitution of literals not available in the first
pass.

The third pass is used to apply binary data includes to complete the
application image.


Related projects
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- RRPGE home: http://www.rrpge.org
- RRPGE Specification: https://www.github.com/Jubatian/rrpge-spec
- RRPGE Emulator & Library: https://www.github.com/Jubatian/rrpge-libminimal
- RRPGE User Library: https://www.github.com/Jubatian/rrpge-userlib
- Example programs: https://www.github.com/Jubatian/rrpge-examples


Temporary license notes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Currently the project is developed under a temporary GPL compatible license.
The intention for later is to add some permissive exceptions to this license,
allowing for creating derivative works (most importantly, applications) under
other licenses than GPL.

For more information, see http://www.rrpge.org/community/index.php?topic=30.0




Invocation
------------------------------------------------------------------------------


When starting without parameters, the assembler will look for a "main.asm"
file in the current directory. Otherwise it can have a single parameter
specifying the assembly source to compile.




Opcode syntax
------------------------------------------------------------------------------


The assembler understands opcodes and addressing modes of the RRPGE CPU as
described in the appropriate sections of the RRPGE specification.

Note that the assembler accepts all opcode specifications and register names
in lowercase.

For BP relative addressing modes, compared to the specified 'bp +' format, the
assembler also supports a shorter '$'. For example reaching the 0th parameter
on the stack may be accomplished by: ::

    mov a, [$0]

This format works with any type of address specification, whitespaces are
optional after the '$'.




Assembler features
------------------------------------------------------------------------------


Comments
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The character ';' or '#' marks the beginning of a comment. After this
character the rest of the source line is ignored from the point of assembling.
Note that ';' or '#' does not begin a comment if it occurs in a string
literal.


Labels and symbols
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The assembler has a single unified symbol concept. Symbols can be defined in
two ways:

- Labels: They symbol will get the value of the current offset within the
  section. Section base offsets are handled appropriately during this pass.

- Equs: The symbol will get the value specified in the equation.

The assembler is two-pass, that is it is capable to deduct the value of the
symbol even if it will be defined only later in the source file (or in an
another included source).

Labels can be specified as the label symbol name placed on the beginning of
the line followed by a colon (':').

Equations can be specified using the following syntax: ::

    symbol_name equ value

Note that symbols are case sensitive.

Local symbols are supported. A local symbol can be defined by naming it
beginning with a dot ('.'). The local symbol is internally represented by
the name constructed by appending it's name (including the beginning '.') to
the last encountered global symbol definition.


Sections and offsets
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Six sections are understood by the assembler:

- code: The code memory, up to 64 KWords.
- data: Initialized data, up to about 62 KWords.
- head: Application header, up to 64 KWords.
- desc: Application descriptor, up to 20 Words.
- zero: Zero data, up to about 62 KWords (shared with data).
- file: Application binary for loading additional data.

The section to use can be specified by the following syntax: ::

    section code
    section data
    section head
    section desc
    section zero
    section file

Opcodes can only be placed into the code section. Data ('dw' and 'db'
keywords) can be defined in all of the code, data, head and desc sections. In
the zero section only space may be reserved for variables (using the 'ds'
keyword).

Initially (before explicitly specifying any section) the code section is
selected.

Each section has an offset associated to it where subsequent data or opcodes
will be placed. Switching between sections preserves these internal offsets.

These offsets may be set explicitly using the 'org' keyword: ::

    org offset

This sets the offset for the currently selected section.

In the application binary the sections are laid out in the following order:
head, desc, code, data, file. Section base offsets (notably for zero and file)
are calculated after the first pass, knowing the sizes of the sections, so the
second pass may rely on the calculated offsets referred by labels.


Data definition and allocation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Within the HEAD, DESC, CODE and DATA sections literal data may be defined
using the 'dw' and 'db' keywords, such as: ::

    dw 0x0001, 2, 3, 0b0101010110101010

Note that the native data width of the architecture is 16 bits. When using the
'db' keyword, Big Endian byte order is employed, and the data is padded to
word boundary when defining an odd number of elements.

Using the 'db' keyword strings may also be defined which is useful for
building the Application Header. Note the word padding.

Within the ZERO section data may only be allocated using the 'ds' keyword: ::

    ds wordcount

Note that contrary to most other assemblers the count of elements refer to
words, not bytes.


Literals
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In every location accepting immediate data the following literal formats may
be used:

- Decimal.
- Hexadecimal, prefixed by '0x'.
- Binary, prefixed by '0b'.
- String, enclosed within '' or "" (single or double quotation marks).

Note that negative numbers and arithmetic is not supported. Hexadecimal
literals are not case sensitive (both 'A' - 'F' and 'a' - 'f' are accepted).

Strings of one to four characters may be used everywhere as literals, then
their numeric value is taken in Big Endian order. Strings longer than four
characters are only accepted in a 'db'.

Note that no terminator is applied to the string (unlike for example the C
language's strings). If a terminating zero is necessary, it may be provided as
a separate data element after the string.

Within strings the following special characters are accepted:

- '\\t': Horizontal tab (0x09)
- '\\n': New line (0x0A)
- '\\r': Carriage return (0x0D)
- '\\'': Can escape a ' within a '' enclosed string.
- '\\"': Can escape a " within a "" enclosed string.
- '\\\\': Escapes a backslash (results one backslash).


Source includes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Another assembly sources may be included using an 'include' keyword: ::

    include "source.asm"

The inclusion happens at the location of the keyword, substituting the
included source at that location as-is.

Guarding against multiple inclusions is implemented, so subsequent inclusions
of the same source file are ignored. Note that the string literal after the
include keyword must match exactly for this to work.


Binary includes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Binary files may be included using the 'bindata' keyword. The syntax is as
follows: ::

    bindata "data.bin"

The binary data is then included as-is into the section currently selected.
Labels should be used to mark the beginning, and if needed, the end of a data
element.


Special keywords
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The special keywords 'AppAuth', 'AppName', 'Version', 'EngSpec' and 'License'
exist to assist positioning at the respective fields of the Application
Header. They are case sensitive.

The keywords are roughly equivalent to a 'section' followed by an appropriate
'org', however the line may continue after them (like after a label). Note
that they change section (to HEAD), so a 'section' keyword may be necessary
after them to switch to the desired section.




The "rrpge.asm" file
------------------------------------------------------------------------------


This is an equivalent of a (C language) header file containing a set of useful
symbols for assembly programs.




Recommendations for starting
------------------------------------------------------------------------------


The assembly project should contain a definition for the Application Header.

The Application Header needs to go into the HEAD section. The assembler
automatically fills the header's framing, you only need to provide the
contents for it. For example it may be done the following way: ::

    AppAuth db "Me"         ; Author (AppAuth)
    AppName db "Test app"   ; Name of application (AppName)
    Version db "00.001.000" ; Version of application (Version)
    EngSpec db "00.011.003" ; Compatible RRPGE version (EngSpec)
    License db "RRPGEvt"    ; License (License)
            db "\n", 0      ; Terminator

It is not necessary to fill in Application Descriptor if the defaults are OK.
By default, no inputs are selected, and a separate 32 KWord stack is used.

For more on the Application Header and Descriptor, check the RRPGE
specification.

Note that at least one instruction (in the CODE section) is necessary for the
application to compile.




Bugs
------------------------------------------------------------------------------


There are several things untested in there, however the most important parts
should be functional.

No FILE section support yet. First literal arithmetic has to be implemented to
make it useful (so it is possible to load the 32 bit offset values).

A global label should be specified before any local label or equ. Otherwise
the local symbol without parent will match any other local symbol with the
same name (both inheriting the same parent when comparing).
