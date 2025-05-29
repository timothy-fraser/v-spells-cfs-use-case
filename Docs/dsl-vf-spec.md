# Table Validation Function DSL Spec

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


This file is a functional specification for Domain-Specific Languages
(DSLs) designed specifically to support the programming of validation
functions for NASA Common Flight System (cFS) tables.

1) The DSL must enable programmers to program validation functions
   that can validate all of the possible table formats described
   in [tbl-structure.md](tbl-structure.md).
   
2) The DSL must enable programmers to program validation functions
   that implement any of the validation function features described
   in the following section.  

3) There must be some means of executing validation functions
   programmed in the DSL in a cFS-based flight software environment,
   possibly with the help of some interpreter, assembler, or
   compiler. This requirement entails that the executable validation 
   functions rely in the cFE APIs and their limited C library
   support.  In particular, it entails that validation functions
   rely on the cFE `CFE_EVS_SendEvent()` API to emit messages.
   
## Validation Function Features

This section describes the universe of possible validation function
features.

11) A validation function must take one and only one table as input.
   Each application has its own specific table format. The grammar in
   [tbl-structure.md](tbl-structure.md) describes the universe of
   possible table formats in terms of TABLEs, ENTRYs and FIELDs.

12) A validation function must count the number of unused ENTRYs in its
   TABLE. The definition of "unused ENTRY" is application-specific,
   but all such definitions are instances of one of the following
   patterns:

     - An ENTRY is unused if a particular one of its FIELDs is equal
       to a particular literal constant value. For example, "an
       ENTRY is unused if the value of its type field is zero. The
       values of its other FIELDs don't matter."

     - An ENTRY is unused if all of its FIELDs are equal to a
       particular literal constant value. For example, "an ENTRY is
       unused if the value of all its fields are zero."

     - An ENTRY is unused per some rule matching one of the above
       patterns. In addition, all ENTRYs following the first unused
       entry in the table's array are also unused, regardless of the
       values of their FIELDs.

13) A validation function must count the number of valid entries in
   its table. Unused entries are not valid. Applications add further
   constraints on what a "valid entry" is. These further
   constraints are application-specific, but all are instances of
   one of the following patterns:

     - The value of a particular FIELD is ==, <, <=, >, or >= some
       constant literal value. For example, "the ENTRY is valid if
       the value of its range FIELD is < 42."

     - The value of a particular FIELD is ==, <, <=, >, or >= one
       or more other FIELDs in the same ENTRY. For example, "the ENTRY is
       valid if the value of its start FIELD is <= the value of its
       end FIELD."

     - The value of a particular FIELD on one ENTRY is ==, <, <=, >,
       or >= the same FIELD in one or more previous ENTRYs.  For example,
       "the ENTRY is valid if the value of its start FIELD is >=
       the value of the start FIELDs of all previous entries."

     - One or more instances of the above patterns joined by Boolean
       operators AND, OR, and NOT. For example, "the ENTRY is valid
       if and only if the value of its range FIELD is < 42 and the
       value of its start FIELD is <= the value of its end FIELD."

     - No prior ENTRY in the table is unused.

14) A validation function must count of the number of invalid ENTRYs
   in its table. An ENTRY that is neither unused nor valid is an
   "invalid ENTRY."

15) A validation function must determine whether its TABLE is valid or
   invalid. A TABLE is valid if and only
   if it has no invalid ENTRYs. Note that TABLEs are either valid or
   invalid - unlike ENTRIES, there is no unused option for TABLEs. A
   TABLE whose ENTRIES are all unused is valid.

16) A validation function may optionally emit a message each time it
   finds a violation of the constraints on unused entries described
   in requirement 12 or the constraints on valid entries described in
   requirement 13. If emitted, these messages must be of type
   `CFE_EVS_EventType_ERROR`,
   and include an application-specific Event ID.  Optionally, these
   messages may also specify the index of an ENTRY,
   the value of one or more entry FIELDs, a constant error-message
   string known at compile time, or some combination of these things.
      
17) A validation function must emit an event of type
   CFE_EVS_EventType_INFORMATION with an application-specific Message
   ID that reports its counts of valid, invalid, and unused ENTRYs
   like so:

   ```
   "Table image entries: valid = V, invalid = I, unused = U"
   ```	

   Where V, I, and U are the counts of valid, invalid, and unused
   table entries, respectively.

18) A validation function must return a UINT32 error code. If the table
   is valid, it must return the constant value CFE_SUCCESS.
   Otherwise, it must return some application-specific constant value
   indicating either general table validation failure or some specific
   kind of table validation failure.


