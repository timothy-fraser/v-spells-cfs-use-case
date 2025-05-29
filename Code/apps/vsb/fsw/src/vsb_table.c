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
 * This file defines the app's table validation function.
 *
 */

#include "cfe.h"
#include "common_types.h"

#include "vs_ground.h"
#include "vs_tablestruct.h"

#include "vsb_tablestruct.h"
#include "vs_eventids.h"
#include "vsb_eventids.h"
#include "vsb_version.h"      /* for VSB_APP_NAME constant */
#include "vsb_table.h"


/* ---------- module private definitions and functions ----------- */

/* VSB_table_init() will ask TBL to initialize the table with values
 * loaded from this file.
 */
#define VSB_DEFAULT_TABLE_FILENAME			\
	"/cf/VSB_" VSB_RAW_TABLE_NAME "_default.tbl"


/* TBL expects our VSB_table_validate() function to return CFE_SUCCESS
 * if the table is valid and something other than CFE_SUCCESS if it is
 * invalid.  This constant is the "something other than".
 */
#define VSB_TABLE_INVALID_RESULT (~CFE_SUCCESS)

	
/* -------------------- module exported functions ------------------ */


/* VSB_table_validate()
 *
 * in:     TblData - pointer to table image to validate
 * out:    nothing
 * return  value               condition
 *         -----               ---------
 *         CFE_Success         Table image is valid
 *         VSB_TABLE_INVALID   Table image is not valid
 *
 * The CFE Table Service (TBL) will call this function to validate
 * table images.  It will provide a pointer to the image to validate
 * via the TblData parameter.
 *
 * Test programs can measure the runtime of this function by asking ES
 * to monitor the VSB_VF_PERF_ID perf ID.
 *
 */

static CFE_Status_t     /* static b/c fxn is exported by pointer not linker */
VSB_table_validate(void *TblData) {

	const vsb_table_t *p_table = (const vsb_table_t *)TblData; /* table */
	CFE_Status_t result = CFE_SUCCESS;   /* optmistically presume valid */
	unsigned int count_unused  = 0;    /* number of unused parm entries */
	unsigned int count_valid   = 0;     /* number of valid parm entries */
	unsigned int count_invalid = 0;   /* number of invalid parm entries */
	unsigned int i;                         /* index into table entries */
	
	/* Mark the start of validation function processing for
	 * performance monitoring.
	 */
	CFE_ES_PerfLogEntry(VSB_VF_PERF_ID);
	

	/*  Here is some wildly incorrect validation logic as an
	 *  example.  Correct logic might fit nicely here.
	 */
	for (i = 0; i < VSB_TABLE_NUM_ENTRIES; i++) {
		if (p_table->entries[ i ].parm_id == VS_PARM_UNUSED) {
			count_unused++;
		} else {
			count_valid++;
		}
	}
	
	/* Send validation function statistics event. */
	CFE_EVS_SendEvent(VSB_VALIDATION_INF_EID,
		CFE_EVS_EventType_INFORMATION, "Table image entries: "
		"%u valid, %u invalid, %u unused",
		count_valid, count_invalid, count_unused);

	/* Mark the stop of validation function processing for
	 * performance monitoring.
	 */
	CFE_ES_PerfLogExit(VSB_VF_PERF_ID);
	
	return result;

} /* VSB_table_validate() */


/* VSB_table_init()
 *
 * in:     nothing
 * out:    p_h_table - set to new table handle
 * return: CFE_Succes or CFE_Status_t error codes.
 *
 * The app's main intialization function VSB_Init() will call this
 * function to register the app's table with the CFE Table Service
 * (TBL) and ask TBL to load its initial valid empty table image.
 *
 */
 
CFE_Status_t
VSB_table_init(CFE_TBL_Handle_t *p_h_table) {

	CFE_Status_t result;  /* holds error codes returned by functions */

	/* Register our single vsb_table_t table with TBL. */
	if (CFE_SUCCESS != (result = CFE_TBL_Register(p_h_table,
		VSB_RAW_TABLE_NAME, sizeof(vsb_table_t), CFE_TBL_OPT_DEFAULT,
		VSB_table_validate))) {
		CFE_ES_WriteToSysLog("%s: CFE_TBL_Register() returned 0x%08X"
			"; %s will shutdown.\n", VSB_APP_NAME, result,
			VSB_APP_NAME);
		return result;
	}

	/* Load the default table values from the filesystem. */
	if (CFE_SUCCESS != (result = CFE_TBL_Load(*p_h_table,
		CFE_TBL_SRC_FILE, VSB_DEFAULT_TABLE_FILENAME))) {
		CFE_ES_WriteToSysLog("%s: CFE_TBL_Load() of %s returned 0x%08X"
			"; %s will shutdown.\n", VSB_APP_NAME,
			VSB_DEFAULT_TABLE_FILENAME, result,
			VSB_APP_NAME);
		return result;
	}

	return CFE_SUCCESS;

} /* VSB_table_init() */


