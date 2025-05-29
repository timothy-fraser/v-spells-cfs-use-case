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

#include "vsa_tablestruct.h"
#include "vs_eventids.h"
#include "vsa_eventids.h"
#include "vsa_version.h"      /* for VSA_APP_NAME constant */
#include "vsa_table.h"


/* ---------- module private definitions and functions ----------- */

/* VSA_table_init() will ask TBL to initialize the table with values
 * loaded from this file.
 */
#define VSA_DEFAULT_TABLE_FILENAME			\
	"/cf/VSA_" VSA_RAW_TABLE_NAME "_default.tbl"


/* TBL expects our VSA_table_validate() function to return CFE_SUCCESS
 * if the table is valid and something other than CFE_SUCCESS if it is
 * invalid.  This constant is the "something other than".
 */
#define VSA_TABLE_INVALID_RESULT (~CFE_SUCCESS)


/* parm_id_to_string()
 *
 * in:     parm_id - numeric parm ID value
 * out:    nothing
 * return: a string represending parm_id.
 *
 * Use this function to convert numeric parm ID values to descriptive
 * strings for use in event messages.
 *
 */

static const char *
parm_id_to_string(uint8 parm_id) {

	switch (parm_id) {
	case VSA_PARM_UNUSED: return "Unused";
	case VSA_PARM_APE:    return "Ape";
	case VSA_PARM_BAT:    return "Bat";
	case VSA_PARM_CAT:    return "Cat";
	case VSA_PARM_DOG:    return "Dog";
	case VSA_PARM_NORTH:  return "North";
	case VSA_PARM_SOUTH:  return "South";
	case VSA_PARM_EAST:   return "East";
	case VSA_PARM_WEST:   return "West";
	default:              return "Invalid";
	}

} /* parm_id_to_string() */


/* pad_is_valid()
 *
 * in:     p_table - pointer to table image to validate
 *         i       - index of table entry to validate
 * out:    nothing
 * return: true if the padding in the i'th entry of *p_table is valid,
 *         else false.
 *
 * A Boolean predicate for testing the validity of an entry's padding
 * field.  Side effect: sends an error event if the padding is
 * invalid.
 *
 * Note that this function presumes it is validating an in-use entry
 * with a valid parm ID field value.
 *
 */

static bool
pad_is_valid(const vsa_table_t *p_table, unsigned int i) {

	const vsa_entry_t *p_entry;   /* points to indexed table entry */

	p_entry = &(p_table->entries[i]);
	
	if ((p_entry->pad[0] & p_entry->pad[1] & p_entry->pad[2]) == 0x00) {
		return true;
	}
	CFE_EVS_SendEvent(VSA_TBL_PAD_ERR_EID,
		CFE_EVS_EventType_ERROR,
		"Table entry %u parm %s padding not zeroed", (i+1),
		parm_id_to_string(p_entry->parm_id));

	return false;

} /* pad_is_valid() */


/* bounds_are_valid()
 *
 * in:     p_table - pointer to table image to validate
 *         i       - index of table entry to validate
 *         min     - minimum valid bound value
 *         max     - maximum valid bound value
 * out:    nothing
 * return: true if the bounds defined by the i'th entry of *p_table
 *         are valid, else false.
 *
 * A Boolean predicate for testing the validity of an entry's low and
 * high bounds fields.  Side effect: sends one or more error events if
 * they are invalid.
 *
 * Note that this function presumes it is validating an in-use entry
 * with a valid parm ID.  The spec for "valid" defines a different
 * min-max range for different parm IDs, it leaves min and max for
 * invalid IDs undefined.
 *
 */

static bool
bounds_are_valid(const vsa_table_t *p_table, unsigned int i,
	uint32 min, uint32 max) {

	const vsa_entry_t *p_entry;       /* points to indexed table entry */
	bool result = true;     /* optimistically presume bounds are valid */
	
	p_entry = &(p_table->entries[i]);

	if (!((min <= p_entry->bound_low) && (p_entry->bound_low <= max))) {

		CFE_EVS_SendEvent(VSA_TBL_LBND_ERR_EID,
			CFE_EVS_EventType_ERROR,
			"Table entry %u parm %s invalid low bound", (i+1),
			parm_id_to_string(p_entry->parm_id));
		result = false;

	}

	if (!((min <= p_entry->bound_high) && (p_entry->bound_high <= max))) {

		CFE_EVS_SendEvent(VSA_TBL_HBND_ERR_EID,
			CFE_EVS_EventType_ERROR,
			"Table entry %u parm %s invalid high bound", (i+1),
			parm_id_to_string(p_entry->parm_id));
		result = false;

	}

	if (!(p_entry->bound_low <= p_entry->bound_high)) {

		CFE_EVS_SendEvent(VSA_TBL_ORDER_ERR_EID,
			CFE_EVS_EventType_ERROR,
			"Table entry %u parm %s invalid bound order", (i+1),
			parm_id_to_string(p_entry->parm_id));
		result = false;

	}

	return result;

} /* bounds_are_valid() */


/* unused_entry_is_valid()
 * 
 * in:     p_table - pointer to table image to validate
 *         i       - index of table entry to validate
 * out:    nothing
 * return: True if the i'th entry of *p_table is a valid unused entry,
 *         else false.
 *
 * A Boolean predicate for testing the validity of an unused entry.
 * Side effect: emits an error event if it is not.
 *
 * Note that this function presumes it is validating an entry with a
 * VSA_PARM_UNUSED parm ID.
 *
 */

static bool
unused_entry_is_valid(const vsa_table_t *p_table, unsigned int i) {

	const vsa_entry_t *p_entry;      /* points to indexed table entry */
	
	p_entry = &(p_table->entries[i]);

	/* Confirm that all fields are zeroed. */
	do {  /* using do/while/break as poor man's try/catch */
	
		if (p_entry->pad[0])     break;
		if (p_entry->pad[1])     break;
		if (p_entry->pad[2])     break;
		if (p_entry->bound_low)  break;
		if (p_entry->bound_high) break;

		return true;

	} while (0);
		
	CFE_EVS_SendEvent(VSA_TBL_ZERO_ERR_EID, CFE_EVS_EventType_ERROR,
		"Table entry %u parm %s not zeroed", (i+1),
		parm_id_to_string(p_entry->parm_id));

	return false;
	
} /* unused_entry_is_valid() */


/* inuse_entry_is_valid()
 *
 * in:     p_table      - pointer to table image to validate
 *         i            - index of table entry to validate
 *         saw_valid_unused_flag - see below
 *         parms_seen   - bitfield indicating Parm IDs seen earlier.
 *         min          - minimum valid bound value
 *         max          - maximum valid bound value
 * out     nothing
 * return: True if the i'th entry of *p_table is valid, else false.
 *
 * This Boolean predicate indicates whether or not an in-use entry is
 * valid.  It presumes the entry has a valid parm ID field.  Callers
 * must supply parameters to configure the rule for what constitutes a
 * valid in-use entry:
 *
 *   - min and max must be set to the proper range for the entry's parm ID.
 *   - If one of the earlier entries in the table is a valid unused
 *     entry, the saw_valid_unused_flag parm must be set.  Otherwise,
 *     it must be clear.
 *   - The parms_seen bitvector must indicate which valid parm IDs
 *     have been used in all earlier table entries, whether their
 *     entries turned out to be valid or not.
 *
 * Side effect: This predicate will emit one or more error events if
 * the entry is not valid.
 *
 */

static bool
inuse_entry_is_valid(const vsa_table_t *p_table, unsigned int i,
	bool saw_valid_unused_flag, uint8 parms_seen,
	uint32 min, uint32 max) {

	const vsa_entry_t *p_entry;     /* points to indexed table entry */
	bool result = true;     /* optimistically presume entry is valid */
	
	p_entry = &(p_table->entries[i]);

	if (!pad_is_valid(p_table, i))               result = false;
	if (!bounds_are_valid(p_table, i, min, max)) result = false;

	/* In-use entries that follow an unused entry are a problem. */
	if (saw_valid_unused_flag) {
		CFE_EVS_SendEvent(VSA_TBL_EXTRA_ERR_EID,
			CFE_EVS_EventType_ERROR,
			"Table entry %u parm %s follows an unused entry",
			(i+1), parm_id_to_string(p_entry->parm_id));
		result = false;
	}

	/* Entries that reuse a Parm ID used previously are a problem.
	 * Use the bits in parms_seen to check.  We've carefully
	 * defined the parm_id constants to enable this tracking.
	 */
	if (parms_seen & p_entry->parm_id) {
		CFE_EVS_SendEvent(VSA_TBL_REDEF_ERR_EID,
			CFE_EVS_EventType_ERROR,
			"Table entry %u parm %s redefines earlier entry",
			(i+1), parm_id_to_string(p_entry->parm_id));
		result = false;
	}

	return result;

} /* inuse_entry_is_valid() */

	
/* -------------------- module exported functions ------------------ */


/* VSA_table_validate()
 *
 * in:     TblData - pointer to table image to validate
 * out:    nothing
 * return  value               condition
 *         -----               ---------
 *         CFE_Success         Table image is valid
 *         VSA_TABLE_INVALID   Table image is not valid
 *
 * The CFE Table Service (TBL) will call this function to validate
 * table images.  It will provide a pointer to the image to validate
 * via the TblData parameter.
 *
 * Some notes on table validation function output:
 *
 * When given an invalid table image, this validation function will
 * emit the following output in the following order:
 *
 *   (1) First, it will use CFE_EVS_SendEvent() to send a
 *       CFE_EVS_EventType_ERROR event describing each specific
 *       validity problem.  Each kind of problem has its own specific
 *       VSA_TBL_*_ERR_EID event ID.  Images with multiple problems
 *       should cause multiple events.  All VSA_TBL_*_ERR_EID events
 *       for entry 0 will come first, then those for entry 1, and so
 *       on.  Any events emitted for a given entry will appear in the
 *       order of their VSA_TBL_*_ERR_EID numeric constants.
 *
 *   (2) It will then use CFE_EVS_SendEvent() to send a
 *       CFE_EVS_EventType_INFORMATION event reporting the number of
 *       valid, invalid, and unsused table entries.  This event will
 *       use the VSA_VALIDATION_INF_EID event ID.
 *
 *   (3) Finally, it will return the VSA-specific
 *       VSA_TABLE_INVALID_RESULT CFE_Status_t code.
 *
 * Note that this validation function will always blame a problem on a
 * particular entry; it has no concept of a problem with the table
 * that isn't a particular entry's fault.  Consequently, a nonzero
 * invalid entry count in step #2 implies a VSA_TABLE_INVALID status
 * returned in step #3 and vice-versa.
 *
 * When given a valid table image, this validation function will emit
 * the number report event described in #2 above and return the
 * CFE_SUCCESS CFE_Status_t code.
 *
 * Key distinction: the VSA_TBL_*_ERR_EIDs inform the operator what
 * the specific error was, but don't control TBL's subsequent
 * activation behavior.  The CFE_Status_t return values control TBL's
 * subsequent activation behavior but indicate only valid or not-valid
 * without concern for what the specifc validity problem was.
 *
 * Test programs watching the event stream can trust this top-level
 * validation function and its various helper functions to send
 * CFE_EVS_EventType_ERROR events in the order described above.
 *
 * Test programs can measure the runtime of this function by asking ES
 * to monitor the VSA_VF_PERF_ID perf ID.
 *
 */

static CFE_Status_t     /* static b/c fxn is exported by pointer not linker */
VSA_table_validate(void *TblData) {

	const vsa_table_t *p_table = (const vsa_table_t *)TblData; /* table */
	unsigned int i;                    /* indexes parm entries in table */
	uint8 parm_id;    /* holds Parm ID of current entry for examination */
	bool saw_valid_unused_flag = false;     /* saw a valid unused entry */
	uint8 parms_seen = 0;  /*  indicates which parms have valid entries */
	CFE_Status_t result = CFE_SUCCESS;   /* optmistically presume valid */
	unsigned int count_unused  = 0;    /* number of unused parm entries */
	unsigned int count_valid   = 0;     /* number of valid parm entries */
	unsigned int count_invalid = 0;   /* number of invalid parm entries */

	/* Mark the start of validation function processing for
	 * performance monitoring.
	 */
	CFE_ES_PerfLogEntry(VSA_VF_PERF_ID);
	
	/* Validate each entry in the table. */
	for (i = 0; i < VSA_TABLE_NUM_ENTRIES; i++) {

		parm_id = p_table->entries[i].parm_id; 
		switch (parm_id) {
		case VSA_PARM_UNUSED:
			if (unused_entry_is_valid(p_table, i)) {
				count_unused++;
				saw_valid_unused_flag = true;
			} else {
				count_invalid++;
				result = VSA_TABLE_INVALID_RESULT;
			}
			break;
		
		case VSA_PARM_APE:
		case VSA_PARM_BAT:
		case VSA_PARM_CAT:
		case VSA_PARM_DOG:
			if (inuse_entry_is_valid(p_table, i,
				saw_valid_unused_flag, parms_seen,
				VSA_PARM_ANIMAL_MIN, VSA_PARM_ANIMAL_MAX)) {
				count_valid++;
			} else {
				count_invalid++;
				result = VSA_TABLE_INVALID_RESULT;
			}
			/* remember this entry's parm */
			parms_seen |= parm_id;
			break;

		case VSA_PARM_NORTH:
		case VSA_PARM_SOUTH:
		case VSA_PARM_EAST:
		case VSA_PARM_WEST:
			if (inuse_entry_is_valid(p_table, i,
				saw_valid_unused_flag, parms_seen,
				VSA_PARM_DIRECTION_MIN,
				VSA_PARM_DIRECTION_MAX)) {
				count_valid++;
			} else {
				count_invalid++;
				result = VSA_TABLE_INVALID_RESULT;
			}
			/* remember this entry's parm */
			parms_seen |= parm_id;
			break;

		default:
			CFE_EVS_SendEvent(VSA_TBL_PARM_ERR_EID,
				CFE_EVS_EventType_ERROR,
				"Table entry %u invalid Parm ID", (i+1));
			count_invalid++;
			result = VSA_TABLE_INVALID_RESULT;
		}

	} /* for all entries in table */
	
	/* Send validation function statistics event. */
	CFE_EVS_SendEvent(VSA_VALIDATION_INF_EID,
		CFE_EVS_EventType_INFORMATION, "Table image entries: "
		"%u valid, %u invalid, %u unused",
		count_valid, count_invalid, count_unused);

	/* Mark the stop of validation function processing for
	 * performance monitoring.
	 */
	CFE_ES_PerfLogExit(VSA_VF_PERF_ID);
	
	return result;

} /* VSA_table_validate() */


/* VSA_table_init()
 *
 * in:     nothing
 * out:    p_h_table - set to new table handle
 * return: CFE_Succes or CFE_Status_t error codes.
 *
 * The app's main intialization function VSA_Init() will call this
 * function to register the app's table with the CFE Table Service
 * (TBL) and ask TBL to load its initial valid empty table image.
 *
 */
 
CFE_Status_t
VSA_table_init(CFE_TBL_Handle_t *p_h_table) {

	CFE_Status_t result;  /* holds error codes returned by functions */

	/* Register our single vsa_table_t table with TBL. */
	if (CFE_SUCCESS != (result = CFE_TBL_Register(p_h_table,
		VSA_RAW_TABLE_NAME, sizeof(vsa_table_t), CFE_TBL_OPT_DEFAULT,
		VSA_table_validate))) {
		CFE_ES_WriteToSysLog("%s: CFE_TBL_Register() returned 0x%08X"
			"; %s will shutdown.\n", VSA_APP_NAME, result,
			VSA_APP_NAME);
		return result;
	}

	/* Load the default table values from the filesystem. */
	if (CFE_SUCCESS != (result = CFE_TBL_Load(*p_h_table,
		CFE_TBL_SRC_FILE, VSA_DEFAULT_TABLE_FILENAME))) {
		CFE_ES_WriteToSysLog("%s: CFE_TBL_Load() of %s returned 0x%08X"
			"; %s will shutdown.\n", VSA_APP_NAME,
			VSA_DEFAULT_TABLE_FILENAME, result,
			VSA_APP_NAME);
		return result;
	}

	return CFE_SUCCESS;

} /* VSA_table_init() */


