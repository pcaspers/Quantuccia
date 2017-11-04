
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

To build and run the test suite on Linux or OSX you can follow these simple steps. The only prerequisite is an installation of the boost header-only libraries.

```
cd test-suite
g++ -I.. -o quantlibtestsuite quantlibtestsuite.cpp
./quantlibtestsuite
```

