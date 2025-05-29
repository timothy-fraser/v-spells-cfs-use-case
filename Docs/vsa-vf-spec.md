# V-SPELLS App Table Validation Function Spec

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


This file is a functional specification for the V-SPELLS App table
validation function the V-SPELLS Apps Alpha (VSA) implements in C
and V-SPELLS App Charlie (VSC) implements in a DSL.

These apps have a single table named "Prm" that stores its operating
parameters. The table contains four parameter configuration entries.
Each entry specifies a low and high bound on the value of an imaginary
parameter. There are eight possible parameters to choose from; four
are named for animals, four for cardinal directions. There is also a
special "empty" parameter to indicate an empty table entry. The low
and high bound values must be drawn from a particular range. There is
one range for animal parameters and another range for direction
parameters. 

The source tree provides a collection of generic `vs` header files
under `apps/vs/fsw`.  The VSA and VSC headers refer to these generic
headers for the constant they hold in common.
`apps/vs/fsw/inc/vs_tblstruct.h` describes the Prm
table's structure and constants as follows:

```
#define VS_PARM_UNUSED 0x00
#define VS_PARM_APE    0x01
#define VS_PARM_BAT    0x02
#define VS_PARM_CAT    0x04
#define VS_PARM_DOG    0x08
#define VS_PARM_NORTH  0x10
#define VS_PARM_SOUTH  0x20
#define VS_PARM_EAST   0x40
#define VS_PARM_WEST   0x80

#define VS_PARM_ANIMAL_MIN    0x00000010
#define VS_PARM_ANIMAL_MAX    0x00001000
#define VS_PARM_DIRECTION_MIN 0x00010000
#define VS_PARM_DIRECTION_MAX 0x01000000

typedef struct {
	uint8 parm_id;
	uint8 pad[3];
	uint32 bound_low;
	uint32 bound_high;
} vs_entry_t;

#define VS_TABLE_NUM_ENTRIES 4

typedef struct {
	vs_entry_t entries[VSA_TABLE_NUM_ENTRIES];
} vs_table_t;
```

The V-SPELLS apps and the `tbltest` test suite that stands in for the ground
station in our experiments all depend on the table details above.
They also depend on the following constant values for returned status
codes and Event IDs in messages:

```
/* App-specific CFE_Status_t return codes */
#define VS_TABLE_INVALID_RESULT (~CFE_SUCCESS)

/* Event IDs for INFO messages */
#define VS_VALIDATION_INF_EID   0x0008 /* table validation statistics */

/* Event IDs for ERROR messages */
#define VS_TBL_ZERO_ERR_EID     0x2001 /* unused entry not zeroed */
#define VS_TBL_PARM_ERR_EID     0x2002 /* entry has invalid parm id value */
#define VS_TBL_PAD_ERR_EID      0x2004 /* entry has nonzero padding value */
#define VS_TBL_LBND_ERR_EID     0x2008 /* low bound out of range */
#define VS_TBL_HBND_ERR_EID     0x2010 /* high bound out of range */
#define VS_TBL_ORDER_ERR_EID    0x2020 /* high bound is smaller than low */
#define VS_TBL_EXTRA_ERR_EID    0x2040 /* in-use entry follows unused entry */
#define VS_TBL_REDEF_ERR_EID    0x2080 /* parm ID used by earlier entry */
```

A complete VSA or VSC app specification would include English-language
requirements mandating the above structures and constant values.  This
incomplete specification focuses only on the table validation
function; readers should take the above details as a given.

English-language requirements for the validation function follow.
Some of them ask the function to emit events.  The cFS framework
provdes the `CFE_EVS_SendEvent()` function to emit events; the content of
these events ultimately arrives at the ground station/test suite as
telemetry.

## V-SPELLS App Table Validation Function Requriements

1) The validation function must have an entry point that takes a
   `void *` pointer to the table image to validate and returns a
   `CFE_Status_t` result code.

2) Each table entry is either valid, invalid, or unused.  The
   validation function must examine all of the entries in the table in
   order of increasing index.  It must determine whether each entry is
   valid, invalid, or unused according to the rules defined below.

3) An entry is unused if and only if the value of its `parm_id` field
   is `VSA_PARM_UNUSED` and it respects the additional conditions
   described in Requirement 4 below.  An entry is valid if and only if
   the value of its `parm_id` field is one of `VSA_PARM_APE`,
   `VSA_PARM_BAT`, `VSA_PARM_CAT`, `VSA_PARM_DOG`, `VSA_PARM_NORTH`,
   `VSA_PARM_SOUTH`, `VSA_PARM_EAST`, or `VSA_PARM_WEST` and it
   respects the addditional conditions described in Requirement 5
   below.  If an entry is neither unused nor valid it is invalid.

4) An entry with `parm_id` `VSA_PARM_UNUSED` must have zero values in
   its `pad`, `bound_low`, and `bound_high` fields.

5) An entry with `parm_id` `VSA_PARM_APE`, `VSA_PARM_BAT`,
   `VSA_PARM_CAT`, `VSA_PARM_DOG`, `VSA_PARM_NORTH`, `VSA_PARM_SOUTH`,
   `VSA_PARM_EAST`, or `VSA_PARM_WEST` must respect the following
   additional conditions to be valid:

    A) The value of its `pad` field is zero.

    B) The value of its `bound_low` field is in range, as defined below.

    C) The value of its `bound_high` field is in range, as defined below.

    D) The value of its `bound_low` field is `<=` the value of its
     `bound_high` field.

    E) None of the entries that preceed it in the table are unused.

    F) Its `parm_id` value was not used in a prior valid or invalid entry.

6) For entries with `parm_id` values `VSA_PARM_APE`, `VSA_PARM_BAT`,
   `VSA_PARM_CAT`, or `VSA_PARM_DOG`, the value of a bound field is in
   range if and only if `VSA_PARM_ANIMAL_MIN <=` bound value `<=
   VSA_PARM_ANIMAL_MAX`.

7) For entries with `parm_id` values `VSA_PARM_NORTH`,
   `VSA_PARM_SOUTH`, `VSA_PARM_EAST`, or `VSA_PARM_WEST`, the value of a
   bound field is in range if and only if `VSA_PARM_DIRECTION_MIN <=`
   bound value `<= VSA_PARM_DIRECTION_MAX`.

8) To help human operators debug problems, the validation function
   must send an event message to EVS each time it encounters a
   condition that violates one of the rules defined by the above
   requirements for unused or valid entries. To provide the test suite
   with more predictable output, it must emit events for the table
   entries in index order. The events must be of type
   `CFE_EVS_EventType_ERROR` and have an Event ID and descriptive
   string corresponding to the rule violated, as defined below.

```
REQ  Event ID               Descriptive string
 3   VSA_TBL_PARM_ERR_EID   Table entry N invalid Parm ID
 4   VSA_TBL_ZERO_ERR_EID   Table entry N parm Unused not zeroed
 5A  VSA_TBL_PAD_ERR_EID    Table entry N parm P padding not zeroed
 5B  VSA_TBL_LBND_ERR_EID   Table entry N parm P invalid low bound
 5C  VSA_TBL_HBND_ERR_EID   Table entry N parm P invalid high bound
 5D  VSA_TBL_ORDER_ERR_EID  Table entry N parm P invalid bound order
 5E  VSA_TBL_EXTRA_ERR_EID  Table entry N parm P follows an unused entry
 5F  VSA_TBL_REDEF_ERR_EID  Table entry N parm P redefines earlier entry

In the descriptive string, replace "N" with the entry number 1, 2, 3,
or 4.  Replace P with a short parameter name as follows:

parm_id value   Short name
VSA_PARM_APE    Ape
VSA_PARM_BAT    Bat
VSA_PARM_CAT    Cat
VSA_PARM_DOG    Dog
VSA_PARM_NORTH  North
VSA_PARM_SOUTH  South
VSA_PARM_EAST   East
VSA_PARM_WEST   West
```

9) When a single entry violates multiple rules, the validation
   function must emit multiple events.  To provide the test suite with
   more easily predictable output, the validation function must emit
   its messages in the order shown in the above table.  For example,
   if the validation function finds an entry with both an invalid low
   and high bound, it must emit the low bound error message before the
   high bound error message.

10) After it has emitted all of the above events describing validity
   problems (if any), the validation function must emit one final
   event.  This event must have the VSA_VALIDATION_INF_EID event ID
   and be of type `CFE_EVS_EventType_INFORMATION`.  It must provide the
   descriptive string `Table image entries: V valid, I invalid, U
   unused` with V, I, and U replaced with 0, 1, 2, 3 or 4 to indicate
   the number of valid, invalid, and unused entries the validation
   function found, respectively.

12) If the validation function found the table contained no invalid
   entries, it must return `CFE_Success`.  Otherwise it must return
   `VSA_TABLE_INVALID_RESULT`.
