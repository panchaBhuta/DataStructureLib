as per https://stackoverflow.com/questions/4239993/determining-endianness-at-compile-time

If you can't find what predefined macros your compiler uses from its documentation,
you can also try coercing it to spit out its full list of predefined macros and guess from there what will work


Compiler                   C macros                         C++ macros
============================================================================================
Clang/LLVM                 clang -dM -E -x c /dev/null      clang++ -dM -E -x c++ /dev/null
GNU GCC/G++                gcc   -dM -E -x c /dev/null      g++     -dM -E -x c++ /dev/null
Hewlett-Packard C/aC++     cc    -dM -E -x c /dev/null      aCC     -dM -E -x c++ /dev/null
IBM XL C/C++               xlc   -qshowmacros -E /dev/null  xlc++   -qshowmacros -E /dev/null
Intel ICC/ICPC             icc   -dM -E -x c /dev/null      icpc    -dM -E -x c++ /dev/null
Microsoft Visual Studio (none)                              (none)
Oracle Solaris Studio      cc    -xdumpmacros -E /dev/null  CC      -xdumpmacros -E /dev/null
Portland Group PGCC/PGCPP  pgcc  -dM -E                     (none)

Finally, to round it out, the Microsoft Visual C/C++ compilers are the odd ones out and don't have any of the above.
Fortunately, they have documented their predefined macros here[ https://learn.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2012/b0084kay(v=vs.110)?redirectedfrom=MSDN ]

======================================================================

also refer  https://stackoverflow.com/questions/142508/how-do-i-check-os-with-a-preprocessor-directive
show GCC defines on Windows:

gcc -dM -E - <NUL:

on Linux:

gcc -dM -E - </dev/null

======================================================================

