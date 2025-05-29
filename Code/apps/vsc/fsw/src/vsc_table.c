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

#include "vsc_tablestruct.h"
#include "vs_eventids.h"
#include "vsc_eventids.h"
#include "vsc_version.h"      /* for VSC_APP_NAME constant */
#include "vsc_table.h"

#include "grunt.h"
#include "grunt_status.h"
#include "vsvf.h"

/* ---------- module private definitions and functions ----------- */

/* VSC_table_init() will ask TBL to initialize the table with values
 * loaded from this file.
 */
#define VSC_DEFAULT_TABLE_FILENAME			\
	"/cf/VSC_" VSC_RAW_TABLE_NAME "_default.tbl"


/* TBL expects our VSC_table_validate() function to return CFE_SUCCESS
 * if the table is valid and something other than CFE_SUCCESS if it is
 * invalid.  This constant is the "something other than".
 */
#define VSC_TABLE_INVALID_RESULT (~CFE_SUCCESS)

	
/* -------------------- module exported functions ------------------ */


/* VSC_table_validate()
 *
 * in:     TblData - pointer to table image to validate
 * out:    nothing
 * return  value               condition
 *         -----               ---------
 *         CFE_Success         Table image is valid
 *         VSC_TABLE_INVALID   Table image is not valid
 *
 * The CFE Table Service (TBL) will call this function to validate
 * table images.  It will provide a pointer to the image to validate
 * via the TblData parameter.
 *
 * Test programs can measure the runtime of this function by asking ES
 * to monitor the VSC_VF_PERF_ID perf ID.
 *
 */

static CFE_Status_t     /* static b/c fxn is exported by pointer not linker */
VSC_table_validate(void *TblData) {

	const vsc_table_t *p_table = (const vsc_table_t *)TblData; /* table */
	CFE_Status_t result = VSC_TABLE_INVALID_RESULT;  /* presume invalid */

	/* Mark the start of validation function processing for
	 * performance monitoring.
	 */
	CFE_ES_PerfLogEntry(VSC_VF_PERF_ID);
	
	if (GRUNT_HALT_TRUE == GRUNT_Run(vsvf_program, VSVF_NUM_INSTRUCTIONS,
		p_table, sizeof(vsc_table_t),
		vsvf_strings, VSVF_NUM_STRINGS)) {
		result = CFE_SUCCESS;
	}

	/* Mark the stop of validation function processing for
	 * performance monitoring.
	 */
	CFE_ES_PerfLogExit(VSC_VF_PERF_ID);
	
	return result;

} /* VSC_table_validate() */


/* VSC_table_init()
 *
 * in:     nothing
 * out:    p_h_table - set to new table handle
 * return: CFE_Succes or CFE_Status_t error codes.
 *
 * The app's main intialization function VSC_Init() will call this
 * function to register the app's table with the CFE Table Service
 * (TBL) and ask TBL to load its initial valid empty table image.
 *
 */
 
CFE_Status_t
VSC_table_init(CFE_TBL_Handle_t *p_h_table) {

	CFE_Status_t result;  /* holds error codes returned by functions */

	/* Register our single vsc_table_t table with TBL. */
	if (CFE_SUCCESS != (result = CFE_TBL_Register(p_h_table,
		VSC_RAW_TABLE_NAME, sizeof(vsc_table_t), CFE_TBL_OPT_DEFAULT,
		VSC_table_validate))) {
		CFE_ES_WriteToSysLog("%s: CFE_TBL_Register() returned 0x%08X"
			"; %s will shutdown.\n", VSC_APP_NAME, result,
			VSC_APP_NAME);
		return result;
	}

	/* Load the default table values from the filesystem. */
	if (CFE_SUCCESS != (result = CFE_TBL_Load(*p_h_table,
		CFE_TBL_SRC_FILE, VSC_DEFAULT_TABLE_FILENAME))) {
		CFE_ES_WriteToSysLog("%s: CFE_TBL_Load() of %s returned 0x%08X"
			"; %s will shutdown.\n", VSC_APP_NAME,
			VSC_DEFAULT_TABLE_FILENAME, result,
			VSC_APP_NAME);
		return result;
	}

	return CFE_SUCCESS;

} /* VSC_table_init() */


