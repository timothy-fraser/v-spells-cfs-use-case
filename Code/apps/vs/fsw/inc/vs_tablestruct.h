#ifndef _VS_TABLESTRUCT_H_
#define _VS_TABLESTRUCT_H_

/* Copyright (c) 2024 Timothy Jon Fraser Consulting LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 */

/* Raw table name for all V-SPELLS (VS) app tables.  TBL will make a
 * "cooked" version with the app name prepended.
 */
#define VS_RAW_TABLE_NAME "Prm"


/* This file declares a type describing the in-memory format of the
 * app's table and defines some constant values related to its fields.
 *
 * The table contains four parameter configuration entries.  Each
 * entry specifies a low and high bound on the value of an imaginary
 * parameter.  There are eight possible parameters to choose from;
 * four are named for animals, four for cardinal directions.  There is
 * also a special "empty" parameter to indicate an empty table entry.
 *
 * The low and high bound values must be drawn from a particular
 * range.  There is one range for animal parameters and another range
 * for direction parameters.
 */

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
	vs_entry_t entries[VS_TABLE_NUM_ENTRIES];
} vs_table_t;


#endif
