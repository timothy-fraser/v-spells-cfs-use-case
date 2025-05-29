# Challenge:  Flatten a VM-based DSL Validation Function

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


This is the third and final challenge in a series that focuses on the
table load-validate-activate workflow of NASA's Core Flight System
(cFS).  See [tbl-overview.md](tbl-overview.md) for an overview of this
workflow.

In addition to the files provided by the previous challenge, this
branch provides:

1) the source for a V-SPELLS Charlie (VSC) app under `CFS/apps/vsc` that
   uses a table with the same structure and validity constraints as
   VSA but implements its validation function in a table validation DSL
   called "Grunt", and

2) the source for a library under `CFS/libs/grunt` that implements a
   VM interpreter for that Grunt DSL.

Specific problems:

1) Use your novel V-SPELLS tools and techniques to flatten the `grunt`
    interpreter VM and enable VSC's validation function to run in
    native code.

    You can gain some confidence your flattened version is correct by
    running the `tbltest` test suite. You can compare its time
    performance to the performance of VSA's C version on those
    tests.

2) Argue that your flattened validation function guarantees the same
    safety and liveness properties the VM interpreter version aims to
    provide, as described in [grunt-manual.md](grunt-manual.md).

