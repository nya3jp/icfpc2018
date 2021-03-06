Team Line Graph
===============

This repository contains source codes for ICFP Programming Contest 2018.

[Team Dashboard](http://gs.nya3.jp/icfpc2018/dashboard.html)


Members
-------

- [@lennmars](https://github.com/lennmars)
- [@nu4nu](https://github.com/nu4nu)
- [@nya3jp](https://github.com/nya3jp)
- [@ytsmiling](https://github.com/ytsmiling)


Programming languages
---------------------

- **C++** for main solutions
- **JavaScript** for visualizer
- **Python** for everything else


How to build and run our solution
---------------------------------

### Prerequisites

Bazel is required to build our solution. See this page for instructions:

https://docs.bazel.build/versions/master/install.html

### Build

```
bazel build //solver
```

### Run

Assembler (for FA* models)

```
bazel-bin/solver/solver --impl line_assembler --target path/to/FA000_tgt.mdl --output out.nbt
```

Deassembler (for FD* models)
```
bazel-bin/solver/solver --impl delete3 --source path/to/FD000_src.mdl --output out.nbt
```

Reassembler (for FR* models)
```
bazel-bin/solver/solver --impl reassemble_naive --disasm delete3 --asm line_assembler --source path/to/FR000_src.mdl --target path/to/FR000_tgt.mdl --output out.nbt
```

Index
-----

- [Simulator in C++](cxx_simulator/sim.cc)
- [Solver in C++](solver)
    - [Entry point](solver/solver.cc)
    - [Solver implementations](solver/impls)
    - [Solver framework ("Tasks")](solver/support/task.h)
    - [Assembler: "Line assembler"](solver/tasks/line_assembler.cc)
    - [Deassembler: "Deleter"](solver/impls/deleter3.cc)
    - [Reassembler: "Naive reassembler"](solver/impls/reassemble_naive.cc)
- [Solver in Python for day 1](solution)
- [Visualizer](visualizer)
- [Reference simulator local runner with Node.js](reference_simulator/main.js)
- [Hand-assembled solution in Python](handasm)
- [Hand-assembled solution in C++](solver/manuals)
- [Model/Trace database](data)
