
[![Build Status](https://travis-ci.org/pcaspers/Quantuccia.svg?branch=master)](https://travis-ci.org/pcaspers/Quantuccia) (master)

[![Build Status](https://travis-ci.org/pcaspers/Quantuccia.svg?branch=v1.1)](https://travis-ci.org/pcaspers/Quantuccia) (v1.1)

[![Build Status](https://travis-ci.org/pcaspers/Quantuccia.svg?branch=v1.0)](https://travis-ci.org/pcaspers/Quantuccia) (v1.0)

Quantuccia: the header only essentials of QuantLib
===============================================================

The Quantuccia project provides the essential classes of QuantLib as a
self contained header only library. It can be used as a drop in
replacement for the heavier QuantLib as long as its scope is
sufficient.

## Build and run the test suite

To build and run the test suite on Linux or OSX you can follow these
simple steps. The only prerequisite is an installation of the boost
header-only libraries.

```
cd test-suite
g++ -I.. -o quantlibtestsuite quantlibtestsuite.cpp
./quantlibtestsuite
```

You can also use the make file though to do the same, i.e. do

```
cd test-suite
make
```

## Check for duplicate symbols

We strive to ensure that in different compilation units including the
same files from Quantuccia no duplicate symbol definitions occur
(typically due to forgotten inline statements or static definitions
during the conversion of source files from the original library). This
can be checked by

```
cd check
make
```

which will include all files in two compilation units and link them
together into one executable. If no linker error occurs (and if all
files are include through quantlib.hpp) there are no duplicate symbol
definitions.
