# Challenge:  Extract a cFS Table Validation Function Spec

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
```


This is the first in a series of challenge that focus on the table
load-validate-activate workflow of NASA's Core Flight System (cFS).
See [tbl-overview.md](tbl-overview.md) for an overview of this
workflow.

The `cFS-draco-rc5` directory contains:

1) the source for a fairly recent version of cFS that is known to work
   on GNU/Linux systems running in VMs,

2) the source for a V-SPELLS Alpha (VSA) app under
   `CFS/apps/vsa` that implements a table validation function in C
   and the cFS API calls to properly use it,

3) the source for a `tbltest` program that tests the VSA table
   validation functions under `CFS/tools/TBLtest`,

4) an English-language specification of all possible cFS table
   structures in [tbl-structure.md](tbl-structure.md).

See [build.md](build.md) for instructions on building cFS, its apps,
and the `tbltest` test program.  See [tbltest.md](tbltest.md) for
instructions on runnge `tbltest` and understanding its output.

Specific problems:

1) Use your novel V-SPELLS tools and techniques to extract a
    functional specification of the VSA table validation function from
    the above legacy C source. A good spec would describe both the
    internal logic of the validation function and its interface with
    the rest of the system. It would provide enough detail to enable a
    developer to reimplement the validation function in some DSL with
    confidence that their reimplimentation is compatible with the
    legacy C implementation.

    The next challenge in the series provides an English-language
    functional specification of the VSA table validation function
    authored manually without the help of V-SPELLS tools and
    techniques. You can judge the quality of your extracted spec by
    comparing it to this manually-authored one. Points appearing in
    both specs are a sign of good quality. Points appearing in the
    manual spec but not the extracted represent omissions. Points
    appearing in the extracted spec but not the manual one must be
    carefully considered: they might represent improvements over the
    manual spec or they might be extraneous.

2) Teams desiring to demonstrate the end-to-end V-SPELLS workflow by
    solving the entire series of cFS challenges in order may wish to
    go further: Use your V-SPELLS tools and techniques to generalize
    the functional spec you extracted for the VSA table validation
    function. Produce a functional specification for a Domain-Specific
    Language (DSL) designed specifically for implementing table
    validation functions.

    Teams should rely on the specification of possible table
    structures found in [tbl-structure.md](tbl-structure.md) to know
    what tables can look like.  They should assume that the VSA table
    validation function demonstrates at least one example of all the
    different kinds of checks a table validation DSL must support.

    The next challenge in the series provides an English-language
    functional specification for such a DSL authored manually without
    the help of V-SPELLS tools and techniques.  You can judge the
    quality of your DSL spec by comparing it with this one.

