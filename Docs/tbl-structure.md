# Table Structure Specification

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


This file presents a grammar that describes all of the possible cFS
table structures we will consider in our V-SPELLS cFS challenge
problems. The grammar:

```
TABLE     ->  ENTRY[CONSTANT] ; Tables are fixed-length arrays of ENTRYs

ENTRY     ->  FIELD+          ; Each ENTRY has the same structure of
                              ; between 1 and 8 fields (inclusive).

FIELD     ->  ENUM            ; enumerated type values, flags set at runtime
          |   NUMBER          ; numeric values set at runtime
          |   PADDING         ; extra space included to word-align fields

ENUM      ->  UINT
NUMBER    ->  UINT
PADDING   ->  UINT

UINT      ->  UINT8           ; 1 byte unsigned integer
          |   UINT16          ; 2 byte unsigned integer
          |   UINT32          ; 4 byte unsigned integer

CONSTANT                      ; UINT8 literal 0 <= n <= 8 that is
                              ; known at compile time.
```


cFS tables are essentially memory buffers. The above grammar indicates
that tables are fixed-length arrays of between 1 and 8 fixed-length
entries. All entries in a given table have the same fixed sequence of
between 1 and 8 fixed-length unsigned integer fields. The number of
entries, their field structure, and the size of each field are known
at compile time. Only the values contained in the fields remaun unknown
until runtime.

The overall size of the table is also known at compile time. The cFS
source represents tables as C structs. C compilers may add implicit
bytes of padding between fields to acheive word alignment; the total
size of a C struct is thus not necessarily the sum of the sizes of its
fields. By convention, the table struct declarations in the cFS source
explicitly include padding bytes to achieve word alignment on their
architecture of choice. V-SPELLS teams may wish to take the same
approach.

An apology to cFS developers: Developers who have examined the source
of many cFS apps will note that the above grammar describes only the
most common table structures. Some apps on github have tables with
features beyond those described here. For example, some tables begin
with headers and end with footers that contain fields different from
the entries. Real cFS tables are not bound by the above "between 1
and 8" limits. Some tables contain subtables. Some have string fields,
and in the case of the SC app, these fields are variable-length. That
said, the simplified grammar above is sufficient for V-SPELLS teams to
demonstrate their tools and techniques.
