
# V-SPELLS Core Flight System (cFS) Use Case


```
Copyright (c) 2024 Timothy Jon Fraser Consulting LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied.  See the License for the specific language governing
permissions and limitations under the License.

This material is based upon work supported by the Defense Advanced
Research Projects Agency (DARPA) and Naval Information Warfare Center
Pacific (NIWC Pacific) under Contract No. N66001-21-C-4021.

Any opinions, findings and conclusions or recommendations expressed in
this material are those of the author(s) and do not necessarily
reflect the views of DARPA or NIWC Pacific.

In the event permission is required, DARPA is authorized to reproduce
the copyrighted material for use as an exhibit or handout at
DARPA-sponsored events and/or to post the material on the DARPA
website.

This software distribution is ** Approved for Public Release,
Distribution Unlimited. ** (DARPA/PRC DISTAR Case #40345.)
```


This software implements a legacy software enhancement use case.  It
presents a series of challenge problems that researchers can use to
demonstrate their novel techniques and tools for:

  - understanding the peculiar abstractions of individual legacy
    software components and recovering their specifications,

  - designing Domain-Specific Languages (DSLs) tailored to those
    abstractions that guarantee safety and liveness properties the
    legacy components lacked,

  - replacing legacy components with safe and secure reimplementations
    using those DSLs, and

  - improving the performance of those reimplementations by converting
    DSL programs that would otherwise run in interpreters or virtual
    machines into lower-overhead versions expressed directly in native
    code.

This use case was originally developed to support DARPA's Verified
Security and Performance Enhancement of Large Legacy Software
(V-SPELLS) research program.  However, it may also be of interest to
the broader research community.


## Context

This use case is based on NASA's Open Source Core Flight System (cFS),
a framework for flight software that has flown on numerous spacecraft
since 2007.  NASA makes cFS available as Open Source software on
GitHub <https://github.com/nasa/cFS>.  This use case distribution does
not contain any NASA cFS code; it contains only the new use case code
that researchers must add to cFS to implement the challenge problems
and tests.  The build instructions linked below describe how to
retrieve the needed cFS components provided by NASA and combine them
with the new use case code provided here.

This use case is designed to work with the `cFS-draco-rc5` release of
cFS, which consists of roughly 40KLOC of C. The challenges focus on
the table validation feature of its Table Service (TBL), a
subcomponent that consists of roughly 2KLOC of C code.  See
[Docs/tbl-overview.md](Docs/tbl-overview.md) for an overview of this
feature.


## Usage

[Docs/build.md](Docs/build.md) provides instructions on how to
retrieve the needed NASA cFS components, integrate the new use case
code, and build the integrated system for use on a single Linux host.

[Docs/tbltest.md](Docs/tbltest.md) describes how to run the `tbltest`
test program to demonstrate the integrated system and its challenge
problems, and to test solutions.

This use case includes a series of three challenge problems that each
test a stage of the component-wise legacy software enhancement
workflow envisioned by V-SPELLS.  Each challenge is described in its
own file:

  - [Docs/challenge-1.md](Docs/challenge-1.md)
  - [Docs/challenge-2.md](Docs/challenge-2.md)
  - [Docs/challenge-3.md](Docs/challenge-3.md)

Researchers can attempt to solve one or more of these challenges
depending on their interests.  The use case includes tests and an
example solution for each challenge that researchers can use to help
evaluate the effectiveness of their own solutions.

