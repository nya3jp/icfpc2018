README for Team Members
=======================


Prerequisites
-------------

Bazel is required to build C++ binaries. See this page for instructions:

https://docs.bazel.build/versions/master/install.html


Examples
--------

Example codes are in examples/ directory.

How to build:

```
$ bazel build //examples:minimal_main
$ bazel-bin/examples/minimal_main
```

How to test:

```
$ bazel test //examples:minimal_lib_test
```
