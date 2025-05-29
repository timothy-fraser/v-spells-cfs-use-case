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

/*
 * This file defines a valid empty table image for the app to load at
 * initialization time.  The build system creates an object file from
 * this C source, uses the elf2cfetbl utility to convert that object
 * file into a .tbl table image file, and then installs that .tbl
 * image file in the simulated spacecraft's filesystem.
 *
 * The build system does not include this code in the flight software
 * that runs aboard the spacecraft.
 */

#include "cfe_tbl_filedef.h"
#include "vs_tablestruct.h"
#include "vsb_tablestruct.h"

vsb_table_t vsb_table_default = {
	.entries = {
		{
			.parm_id = (unsigned char)VSB_PARM_UNUSED,
			.pad = { 0x00, 0x00, 0x00 },
			.bound_low  = 0,
			.bound_high = 0
		},
		{
			.parm_id = (unsigned char)VSB_PARM_UNUSED,
			.pad = { 0x00, 0x00, 0x00 },
			.bound_low  = 0,
			.bound_high = 0
		},
		{
			.parm_id = (unsigned char)VSB_PARM_UNUSED,
			.pad = { 0x00, 0x00, 0x00 },
			.bound_low  = 0,
			.bound_high = 0
		},
		{
			.parm_id = (unsigned char)VSB_PARM_UNUSED,
			.pad = { 0x00, 0x00, 0x00 },
			.bound_low  = 0,
			.bound_high = 0
		}
	}
};


/* Note: you must keep the table name and file name parms below
 * syncrhonized with the app's VSB_APP_NAME, VSB_RAW_TABLE_NAME, and
 * VSB_DEFAULT_TABLE_FILENAME constants.  Note that the filename here
 * is just the last component; it doesn't start with "/cf/".
 */
CFE_TBL_FILEDEF(vsb_table_default, VSB_APP.Prm, VSB default empty table,
	VSB_Prm_default.tbl)
