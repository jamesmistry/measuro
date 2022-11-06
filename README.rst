Measuro
=======

.. image:: https://github.com/jamesmistry/measuro/actions/workflows/ci.yaml/badge.svg

Measuro: A fast, easy-to-use header library for creating program metrics in 
modern C++.

Comprehensive Measuro documentation - including guidance for developers and
code examples - is located at:
<https://measuro.readthedocs.io/en/0.2.0/>

Key Features
------------

- Header-only library written in pure C++14 with no external runtime 
  dependencies
- An intuitive, thread-safe interface for manipulating numeric, string and 
  boolean metrics
- Sum numeric metrics together automatically using the special 'sum' metric 
  type
- Calculate the rate of change of a numeric metric automatically using the 
  special 'rate' metric type
- Minimise the performance impact of frequent metric updates using built-in 
  rate limiting options
- Easily output metrics in custom formats using the simple Renderer interface,
  or use the built-in plain text, JSON and Prometheus renderers
- Schedule asynchronous metric render operations to automatically write 
  formatted metrics to an output stream at regular intervals

Getting Started
---------------

**Prerequisites**

To use Measuro in your application you will need at minimum:

- cmake >= 2.6
- A fully-compliant C++14 compiler - g++ 6.1 or above recommended. Measuro has 
  not been tested with other compilers (though it will probably be easy to get 
  building in those that have implemented the standard).

  If you are using a version of g++ < 6.1, you will need to supply the 
  ``-std=c++14`` switch to enable C++14 mode.
- To link against pthread, e.g. -lpthread

It's recommended that you use the Measuro build system to generate a 
development package you can install using your system package manager. To do 
this you will need package building tools such as rpm-build or builddeb.

To run the Measuro test suite you will need:

- C++14 compiler, as above
- cmake >= 2.6
- gtest development package (unit testing framework)
- valgrind, for dynamic analysis
- cppcheck, for static analysis
- Python >= 2.6

**Installing**

It's recommended that you import Measuro as a sub-module for use in your
own applications. The Measuro header file can be included by adding the ``src``
directory in the Measuro repository root to your include search path. The header
file as pushed to the repository is ready for inclusion.

You can also generate a development package to install on your 
system. To do this on an RPM-based system, from the repository root::

    $ cmake ./ -DCPACK_GENERATOR=RPM

To do this on a DEB-based system, from the repository root::

    $ cmake ./ -DCPACK_GENERATOR=DEB

Then::

    $ make package

A package file will be generated in the repository root. You can install it
using your system's package manager, for example::

    $ sudo yum install measuro-devel-x.y-z.rpm

Or::

    $ sudo dpkg -i measuro-devel-x.y-z.deb

You can also copy the Measuro header file to your source tree. This is probably
the least preferable option. To do this, from the repository root::

    $ cp src/measuro.hpp /your/app/src

**Documentation**

Comprehensive Measuro documentation is located at: 
<https://measuro.readthedocs.io/en/0.2.0/>

To build the documentation in the Measuro repository locally, from the 
repository root::

    $ doxygen
    $ cd docs
    $ make html

In-code HTML documentation will be written to ``docs/code/html`` and 
user/developer HTML documentation will be written to ``docs/build/html``.

Running the Tests
-----------------

To run the static analysis, unit tests and system test, from the 
repository root::

    $ mkdir build && cd build
    $ cmake ../
    $ make
    $ make test

To run the benchmarking program, from the repository root::

    $ mkdir build && cd build
    $ cmake -DCMAKE_BUILD_TYPE=Release ../
    $ make measuro_benchmark_exe
    $ ./build/bin/measuro_benchmark_exe

Versioning
----------

Measuro versioning follows the SemVer specification: <http://semver.org/>

You can see the tagged Measuro versions at:
<https://github.com/jamesmistry/measuro/tags>

Licence
-------

This project is licensed under the MIT License - see the LICENSE file.

