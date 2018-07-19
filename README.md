Team CalmDownDear(仮)
=====================

[![CircleCI](https://circleci.com/gh/nya3jp/icfpc2018.svg?style=svg&circle-token=9e79cb8894982a0630fb7fbf3ed1cb04a15cb109)](https://circleci.com/gh/nya3jp/icfpc2018)


Members
-------

- @lennmars
- @nu4nu
- @nya3jp
- @ytsmiling


How to Build and Run
--------------------

Bazel is required to build C++ binaries. See this page for instructions:

https://docs.bazel.build/versions/master/install.html

After prerequisites are installed, our solution can be built and run by:

```
TODO(all): Update this
```

Notes for Team Members
----------------------

### Command Examples

Example codes are in `examples` directory.

#### Building

```
$ bazel build ...
or
$ bazel build //examples:minimal_main
```

#### Running

```
$ bazel-bin/examples/minimal_main
or
$ bazel run //examples:minimal_main
```

#### Testing

```
$ bazel test ...
or
$ bazel test //examples:minimal_lib_test
```