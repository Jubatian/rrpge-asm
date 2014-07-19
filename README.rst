
RRPGE Assembler
==============================================================================

:Author:    Sandor Zsuga (Jubatian)
:Copyright: 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
            License) extended as RRPGEv2 (version 2 of the RRPGE License): see
            LICENSE.GPLv3 and LICENSE.RRPGEv2 in the project root.




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

- Labels: They symbol will get the value of the current 16 bit offset within
  the section.

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

Three sections are understood by the assembler:

- code: Refers to the code memory, up to 64 KWords.
- cons: Refers to the application header, up to 4 KWords.
- data: Refers to page 3 in the CPU's address space, up to 4 KWords.

The code and cons sections are compiled into the application binary as
appropriate. The data section can not hold any initialization values, by the
definition of the RRPGE system however this area is initialized to zero.

The section to use can be specified by the following syntax: ::

    section code
    section cons
    section data

Opcodes can only be placed into the code section. Data ('dw' and 'db'
keywords) can be defined in both the code and cons sections. In the data
section only space may be reserved for variables (using the 'ds' keyword).

Initially (before explicitly specifying any section) the code section is
selected.

Each section has an offset associated to it where subsequent data or opcodes
will be placed. Switching between sections preserves these internal offsets.

These offsets may be set explicitly using the 'org' keyword: ::

    org offset

This sets the offset for the currently selected section.


Data definition and allocation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Within the code and cons sections literal data may be defined using the 'dw'
and 'db' keywords, such as: ::

    dw 0x0001, 2, 3, 0b0101010110101010

Note that the native data width of the architecture is 16 bits. When using the
'db' keyword, Big Endian byte order is employed, and the data is padded to
word boundary when defining an odd number of elements.

Using the 'db' keyword strings may also be defined which is useful for
building the Application Header. Note the word padding.

Within the data section data may only be allocated using the 'ds' keyword: ::

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
- String, enclosed within '' or "".

Note that negative numbers and arithmetic is not supported. Hexadecimal
literals are not case sensitive (both 'A' - 'F' and 'a' - 'f' are accepted).

Strings of one to four characters may be used everywhere as literals, then
their numeric value is taken in Big Endian order. Strings longer than four
characters are only accepted in a 'db'.

Within strings the following special characters are accepted:

- '\t': Horizontal tab (0x09)
- '\n': New line (0x0A)
- '\r': Carriage return (0x0D)
- '\'': Can escape a ' within a '' enclosed string.
- '\"': Can escape a " within a "" enclosed string.
- '\\': Escapes a backslash (results one backslash).


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

    bindata "data.bin" page, offset

The offset may be omitted which case it will be evaluated to zero.

The page specifies the binary data page where the inclusion should start. Data
spanning multiple pages is supported. Note that page numbering starts with 0
with the first binary data page (so not including the Application Header and
the Code pages after it).

The page may be set to 'h' to request inclusion within the Application Header
as follows: ::

    bindata "data.bin" h, offset

This case the data must fit within the Application Header.




The "rrpge.asm" file
------------------------------------------------------------------------------


This is an equivalent of a header file containing a set of useful symbols for
assembly programs.




Recommendations for starting
------------------------------------------------------------------------------


The assembly project should contain a definition for the Application Header.
Check the appropriate section of the RRPGE specification to see how it should
be constructed.

For a proper Application Header the head may be built either as a binary
include or directly in the form of 'db' definitions in the 'cons' section. The
fields "AppAuth", "AppName", "Version", "EngSpec" and "License" are mandatory
so they should be filled up. The textual data may be omitted, this case after
the termination of the "License" field a zero (0x00) may be placed to indicate
it is empty.

The 0xBC0 - 0xBC4 range defining the basic properties of the application must
be filled up appropriately. The assembler will fail if you omit populating
this area. 0xBC4 may simply be set 0xF800 if no extra features of the header
are necessary.

In the 0xC00 - 0xFFF area an appropriate 64x64 icon may be loaded using a
'bindata' keyword. This is not necessary. Note that the application can not
use this area, so there is no point to place anything else than an icon here.




Bugs
------------------------------------------------------------------------------


There are several things untested in there, however the most important parts
should be functional.

Some error reports may be quirky, such as currently symbol redefinition is
only a warning, and prints the entire line with the symbol; and local jump and
call (jml, jfl) can not be checked for out of range addresses.

Register in first operand, special in second operand opcode forms are not
supported such as "mov a, xm" since this case the assembler assumes the second
operand to be an addressing mode specification (only allowing the eight
general purpose registers).
