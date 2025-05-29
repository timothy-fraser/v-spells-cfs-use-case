# Challenge:  Reimplement a cFS Table Validation Function in a DSL

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


This is the second in a series of challenge that focus on the table
load-validate-activate workflow of NASA's Core Flight System (cFS).
See [tbl-overview.md](tbl-overview.md) for an overview of this
workflow.

In addition to the files provided in the first challenge, this
branch provides:

1) an English-language functional specification of the VSA table
   validation function in [vsa-vf-spec.md](vsa-vf-spec.md),

2) an English-language functional specification for a Domain-Specific
   Language (DSL) for cFS table validation functions in
   [dsl-vf-spec.md](dsl-vf-spec.md), and

3) the source for a V-SPELLS Bravo (VSB) app under `CFS/apps/vsb` that
   is the same as the VSA app but has its validation function logic
   stubbed out.  If you use VSB as the basis for your solution to this
   challenge, you can avoid some app and test suite development effort.

Specific problems:

1) Use your novel V-SPELLS tools and techniques to create a DSL for
   programming cFS table validation functions that respects the
   specifications in [tbl-structure.md](tbl-structure.md) and
   [dsl-vf-spec.md](dsl-vf-spec.md).

2) Use your DSL to reimplement the VSA table validation function in a
   manner that respects the specification in
   [vsa-vf-spec.md](vsa-vf-spec.md) and passes the `tbltest`
   functional tests. A solution that provides safety properties by
   running the validation functions in a virtual machine or
   interpreter-based sandbox is sufficient. Teams can leave the
   flattening of their DSL solutions to native code to the third and
   final challenge in the series.

