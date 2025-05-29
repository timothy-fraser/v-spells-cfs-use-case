#ifndef _VSB_TABLESTRUCT_H_
#define _VSB_TABLESTRUCT_H_

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

/* Raw table name for this app's single table.  TBL will make a
 * "cooked" version with the app name prepended.
 */
#define VSB_RAW_TABLE_NAME VS_RAW_TABLE_NAME


/* This file declares a type describing the in-memory format of the
 * app's table and defines some constant values related to its fields,
 * all in terms of the common types and constants from
 * vs_tablestruct.h.
 */

#define VSB_PARM_UNUSED VS_PARM_UNUSED
#define VSB_PARM_APE    VS_PARM_APE   
#define VSB_PARM_BAT    VS_PARM_BAT   
#define VSB_PARM_CAT    VS_PARM_CAT   
#define VSB_PARM_DOG    VS_PARM_DOG   
#define VSB_PARM_NORTH  VS_PARM_NORTH 
#define VSB_PARM_SOUTH  VS_PARM_SOUTH 
#define VSB_PARM_EAST   VS_PARM_EAST  
#define VSB_PARM_WEST   VS_PARM_WEST  

#define VSB_PARM_ANIMAL_MIN    VS_PARM_ANIMAL_MIN   
#define VSB_PARM_ANIMAL_MAX    VS_PARM_ANIMAL_MAX   
#define VSB_PARM_DIRECTION_MIN VS_PARM_DIRECTION_MIN
#define VSB_PARM_DIRECTION_MAX VS_PARM_DIRECTION_MAX

typedef vs_entry_t vsb_entry_t;

#define VSB_TABLE_NUM_ENTRIES VS_TABLE_NUM_ENTRIES

typedef vs_table_t vsb_table_t;


#endif
