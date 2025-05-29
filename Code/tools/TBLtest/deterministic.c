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

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "cfe.h"
#include "cfe_evs_extern_typedefs.h"   /* for CFE_EVS_EventType_* enum */
#include "cfe_tbl_eventids.h"          /* for TBL event IDs */
#include "cfe_tbl_msg.h"               /* for TBL command codes */
#include "to_lab_msg.h"                /* for TO_LAB command codes */
#include "to_lab_events.h"             /* for TO_LAB event IDs */
#include "vs_tablestruct.h"            /* for common VS table constants */
#include "vs_eventids.h"               /* for common VS event ID constants */
#include "vs_ground.h"                 /* for app name constants */

#include "cmd.h"
#include "tlm.h"
#include "file.h"
#include "send.h"
#include "expect.h"
#include "perf.h"
#include "tbltest.h"
#include "deterministic.h"


/* initialize()
 *
 * in:     app_name   - name of app to test
 *         app_perfid - performance ID of app to test
 * out:    nothing
 * return  0 on success, else -1
 *
 * Tell the cFS TO/TO_LAB telemetry forwarding service to start
 * forwarding the telemetry our tests need to determine pass/fail
 * results and confirms that it complied.
 *
 * Tell ES to log performance events from our app when we turn
 * performance monitoring on during tests.
 *
 * Call this function before calling any other function in this
 * module.
 */

static int
initialize(const char *app_name, uint32 app_perfid) {

	/* ES does not respond with telemetry if it successfully
	 * processes the following two commands.  Consequently, these
	 * send_*() calls aren't followed by expect_*() calls.
	 */ 
	send_perfmon(app_name, app_perfid);

	/* Tell TO_LAB to turn on the telemetry output we need to
	 * determine the pass/fail result of all our subsequent tests.
	 * Expect TO_LAB to comply.
	 */
	send_tlmon();
	return expect_tlmon_success();

} /* initialize() */


/* test_control_flow_valid_table()
 *
 * in:     app_name   - name of app to test
 *         app_perfid - app validation function perf ID
 *         tbl_name   - name of test table
 * out:    nothing
 * return: 0 on pass, -1 on fail.
 *
 * Tests the two path through the table load-validate-activate
 * sequence.
 
 * Although this test incidentally confirms that our app's validation
 * function will correctly recognize this particular table image as
 * valid, its main purpose is to confirm the following two paths:
 *
 *   (1) Confirm that the cFE TBL Table service properly refuses to
 *       activate a table image without first running our app's table
 *       validation function on it to confirm its validity, and
 *
 *   (2) confirm that it proceeds with activation after invoking our
 *       app's validation function and finding that the image is valid.
 *
 */

static int
test_control_flow_valid_table(const char *app_name, uint32 app_perfid,
	const char *tbl_name) {

	puts("FILE: create file containing a valid table image.");
	file_init(tbl_name, TABLE_DESCRIPTION);
	file_set_entry(0, VS_PARM_BAT, 0x00,
		VS_PARM_ANIMAL_MIN, VS_PARM_ANIMAL_MAX);
	file_set_entry(1, VS_PARM_EAST, 0x00,
		VS_PARM_DIRECTION_MIN, VS_PARM_DIRECTION_MAX);
	file_output(PATH_TO_CF TABLE_FILENAME);
	file_print();

	send_perfstart();
	do {  /* using do/while/break as poor man's try/catch */

		send_load();
		if (expect_load_success(tbl_name))     break;
		send_activate(tbl_name);
		if (expect_activate_failure(tbl_name)) break;
		send_validate(tbl_name);
		if (expect_validate_success(app_name, tbl_name, 2, 0, 2))
			break;
		send_activate(tbl_name);
		if (expect_activate_success(app_name, tbl_name)) break;

		send_perfstop();
		perf_print(app_perfid);
		/* If we reach here, the test as a whole passed. */
		return 0;  /* PASS */
		
	} while (0);

	send_perfstop();
	
	/* If we reach here, some test step failed. */
	return -1;  /* FAIL */
	
} /* test_control_flow_valid_table() */


/* test_control_flow_invalid_table()
 *
 * in:     app_name   - name of app to test
 *         app_perfid - app validation function perf ID
 *         tbl_name   - name of test table
 * out:    nothing
 * return: 0 on pass, -1 on fail.
 *
 * Tests the failure path through the table load-validate-activate
 * sequence.  Although this test incidentally confirms that our app's
 * validation function will correctly recognize this particular table
 * image as *invalid*, its main purpose is to confirm that the cFE TBL
 * Table service properly invokes our app's table validation function
 * before activating a table image and *refuses to proceed* with
 * activation after finding that the image is valid.
 */

static int
test_control_flow_invalid_table(const char *app_name, uint32 app_perfid,
	const char *tbl_name) {

	puts("FILE: create file containing an invalid table image.");
	file_init(tbl_name, TABLE_DESCRIPTION);
	file_set_entry(0, VS_PARM_BAT, 0x00,
		VS_PARM_ANIMAL_MIN, VS_PARM_ANIMAL_MAX);
	file_set_entry(3, VS_PARM_APE, 0x00, VS_PARM_ANIMAL_MIN,
		VS_PARM_ANIMAL_MAX);  /* used follows unused  error */
	file_output(PATH_TO_CF TABLE_FILENAME);
	file_print();

	send_perfstart();
	do {  /* using do/while/break as poor man's try/catch */

		send_load();
		if (expect_load_success(tbl_name))     break;
		send_validate(tbl_name);
		if (expect_validate_failure(app_name, tbl_name, 1, 1, 2))
			break;
		
		send_activate(tbl_name);
		if (expect_activate_failure(tbl_name)) break;

		send_perfstop();
		perf_print(app_perfid);
		
		/* If we reach here, the test as a whole passed. */
		return 0;  /* PASS */
		
	} while (0);

	send_perfstop();
	
	/* If we reach here, some test step failed. */
	return -1;  /* FAIL */
	
} /* test_control_flow_invalid_table() */


/* test_zero_err()
 *
 * in:     app_name   - name of app to test
 *         app_perfid - app validation function perf ID
 *         tbl_name   - name of test table 
 * out:    nothing
 * return: 0 on pass, -1 on fail.
 *
 * Test table validity requirement: unused entries must be entirely zeroed.
 *
 */

static int
test_zero_err(const char *app_name, uint32 app_perfid, const char *tbl_name) {
	
	puts("FILE: create table image with nonzeroed unused entry.");
	file_init(tbl_name, TABLE_DESCRIPTION);
	file_set_entry(0, VS_PARM_APE, 0x00,
		VS_PARM_ANIMAL_MIN, VS_PARM_ANIMAL_MAX); /* valid */
	file_set_entry(1, VS_PARM_UNUSED, 0x00,
		VS_PARM_DIRECTION_MIN, VS_PARM_DIRECTION_MAX);
	file_output(PATH_TO_CF TABLE_FILENAME);
	file_print();

	send_perfstart();
	do {  /* using do/while/break as poor man's try/catch */

		send_load();
		if (expect_load_success(tbl_name))     break;
		send_validate(tbl_name);
		if (expect_err(app_name, VS_TBL_ZERO_ERR_EID,
			"Table entry 2 parm Unused not zeroed"))
			break;
		
		if (expect_validate_failure(app_name, tbl_name, 1, 1, 2))
			break;

		send_perfstop();
		perf_print(app_perfid);
		
		/* If we reach here, the test as a whole passed. */
		return 0;  /* PASS */
		
	} while (0);

	send_perfstop();
	
	/* If we reach here, some test step failed. */
	return -1;  /* FAIL */

} /* test_zero_err() */


/* test_parm_err()
 *
 * in:     app_name   - name of app to test
 *         app_perfid - app validation function perf ID
 *         tbl_name   - name of test table 
 * out:    nothing
 * return: 0 on pass, -1 on fail.
 *
 * Test table validity requirement: all entry parm IDs must be valid.
 *
 */

static int
test_parm_err(const char *app_name, uint32 app_perfid, const char *tbl_name) {
	
	puts("FILE: create table image with invalid parm ID.");
	file_init(tbl_name, TABLE_DESCRIPTION);
	file_set_entry(0, VS_PARM_BAT, 0x00,
		VS_PARM_ANIMAL_MIN, VS_PARM_ANIMAL_MIN); /* valid */
	file_set_entry(1, (VS_PARM_APE|VS_PARM_NORTH), 0x00,
		VS_PARM_DIRECTION_MIN, VS_PARM_DIRECTION_MAX);
	file_output(PATH_TO_CF TABLE_FILENAME);
	file_print();

	send_perfstart();
	do {  /* using do/while/break as poor man's try/catch */

		send_load();
		if (expect_load_success(tbl_name))     break;
		send_validate(tbl_name);
		if (expect_err(app_name, VS_TBL_PARM_ERR_EID,
			"Table entry 2 invalid Parm ID"))
			break;
		
		if (expect_validate_failure(app_name, tbl_name, 1, 1, 2))
			break;

		send_perfstop();
		perf_print(app_perfid);
		
		/* If we reach here, the test as a whole passed. */
		return 0;  /* PASS */
		
	} while (0);

	send_perfstop();
	
	/* If we reach here, some test step failed. */
	return -1;  /* FAIL */

} /* test_parm_err() */


/* test_pad_err()
 *
 * in:     app_name   - name of app to test
 *         app_perfid - app validation function perf ID
 *         tbl_name   - name of test table
 * out:    nothing
 * return: 0 on pass, -1 on fail.
 *
 * Test table validity requirement: all entry padding must be zeroed.
 *
 */

static int
test_pad_err(const char *app_name, uint32 app_perfid, const char *tbl_name) {
	
	puts("FILE: create table image with nonzero padding.");
	file_init(tbl_name, TABLE_DESCRIPTION);
	file_set_entry(0, VS_PARM_CAT, 0x00,
		VS_PARM_ANIMAL_MAX, VS_PARM_ANIMAL_MAX); /* valid */
	file_set_entry(1, VS_PARM_APE, 0x42, VS_PARM_ANIMAL_MIN,
		VS_PARM_ANIMAL_MAX);
	file_output(PATH_TO_CF TABLE_FILENAME);
	file_print();

	send_perfstart();
	do {  /* using do/while/break as poor man's try/catch */

		send_load();
		if (expect_load_success(tbl_name))     break;
		send_validate(tbl_name);
		if (expect_err(app_name, VS_TBL_PAD_ERR_EID,
			"Table entry 2 parm Ape padding not zeroed"))
			break;
		
		if (expect_validate_failure(app_name, tbl_name, 1, 1, 2))
			break;

		send_perfstop();
		perf_print(app_perfid);
		
		/* If we reach here, the test as a whole passed. */
		return 0;  /* PASS */
		
	} while (0);

	send_perfstop();
	
	/* If we reach here, some test step failed. */
	return -1;  /* FAIL */

} /* test_pad_err() */


/* test_lbnd_err()
 *
 * in:     app_name   - name of app to test
 *         app_perfid - app validation function perf ID
 *         tbl_name   - name of test table
 * out:    nothing
 * return: 0 on pass, -1 on fail.
 *
 * Test table validity requirement: all in-use entry low bound
 * fields must be within the range allowed for their paricular parm
 * ID.
 *
 */

static int
test_lbnd_err(const char *app_name, uint32 app_perfid, const char *tbl_name) {
	
	puts("FILE: create table image with low bound out of range.");
	file_init(tbl_name, TABLE_DESCRIPTION);
	file_set_entry(0, VS_PARM_DOG, 0x00,
		VS_PARM_ANIMAL_MIN,
		(VS_PARM_ANIMAL_MAX - VS_PARM_ANIMAL_MIN) / 2); /* valid */
	file_set_entry(1, VS_PARM_APE, 0x00, VS_PARM_DIRECTION_MIN,
		VS_PARM_ANIMAL_MAX);
	file_output(PATH_TO_CF TABLE_FILENAME);
	file_print();

	send_perfstart();
	do {  /* using do/while/break as poor man's try/catch */

		send_load();
		if (expect_load_success(tbl_name))     break;
		send_validate(tbl_name);
		if (expect_err(app_name, VS_TBL_LBND_ERR_EID,
			"Table entry 2 parm Ape invalid low bound"))
			break;

		if (expect_validate_failure(app_name, tbl_name, 1, 1, 2))
			break;

		send_perfstop();
		perf_print(app_perfid);
		
		/* If we reach here, the test as a whole passed. */
		return 0;  /* PASS */
		
	} while (0);

	send_perfstop();
	
	/* If we reach here, some test step failed. */
	return -1;  /* FAIL */

} /* test_lbnd_err() */


/* test_hbnd_err()
 *
 * in:     app_name   - name of app to test
 *         app_perfid - app validation function perf ID
 *         tbl_name   - name of test table
 * out:    nothing
 * return: 0 on pass, -1 on fail.
 *
 * Test table validity requirement: all in-use entry high bound
 * fields must be within the range allowed for their paricular parm
 * ID.
 *
 */

static int
test_hbnd_err(const char *app_name, uint32 app_perfid, const char *tbl_name) {
	
	puts("FILE: create table image with high bound out of range.");
	file_init(tbl_name, TABLE_DESCRIPTION);
	file_set_entry(0, VS_PARM_NORTH, 0x00,
		VS_PARM_DIRECTION_MIN, VS_PARM_DIRECTION_MAX); /* valid */
	file_set_entry(1, VS_PARM_APE, 0x00, VS_PARM_ANIMAL_MIN,
		VS_PARM_DIRECTION_MAX);
	file_output(PATH_TO_CF TABLE_FILENAME);
	file_print();

	send_perfstart();
	do {  /* using do/while/break as poor man's try/catch */

		send_load();
		if (expect_load_success(tbl_name))     break;
		send_validate(tbl_name);
		if (expect_err(app_name, VS_TBL_HBND_ERR_EID,
			"Table entry 2 parm Ape invalid high bound"))
			break;
		
		if (expect_validate_failure(app_name, tbl_name, 1, 1, 2))
			break;

		send_perfstop();
		perf_print(app_perfid);
		
		/* If we reach here, the test as a whole passed. */
		return 0;  /* PASS */
		
	} while (0);

	send_perfstop();
	
	/* If we reach here, some test step failed. */
	return -1;  /* FAIL */

} /* test_hbnd_err() */


/* test_order_err()
 *
 * in:     app_name   - name of app to test
 *         app_perfid - app validation function perf ID
 *         tbl_name   - name of test table
 * out:    nothing
 * return: 0 on pass, -1 on fail.
 *
 * Test table validity requirement: all in-use entry low bound fields
 * must be no larger than their high bound fields.
 *
 */

static int
test_order_err(const char *app_name, uint32 app_perfid, const char *tbl_name) {
	
	puts("FILE: create table image with bounds out of order.");
	file_init(tbl_name, TABLE_DESCRIPTION);
	file_set_entry(0, VS_PARM_SOUTH, 0x00,
		VS_PARM_DIRECTION_MIN, VS_PARM_DIRECTION_MIN); /* valid */
	file_set_entry(1, VS_PARM_APE, 0x00, VS_PARM_ANIMAL_MAX,
		VS_PARM_ANIMAL_MIN);
	file_output(PATH_TO_CF TABLE_FILENAME);
	file_print();

	send_perfstart();
	do {  /* using do/while/break as poor man's try/catch */

		send_load();
		if (expect_load_success(tbl_name))     break;
		send_validate(tbl_name);
		if (expect_err(app_name, VS_TBL_ORDER_ERR_EID,
			"Table entry 2 parm Ape invalid bound order"))
			break;
		
		if (expect_validate_failure(app_name, tbl_name, 1, 1, 2))
			break;

		send_perfstop();
		perf_print(app_perfid);
		
		/* If we reach here, the test as a whole passed. */
		return 0;  /* PASS */
		
	} while (0);

	send_perfstop();
	
	/* If we reach here, some test step failed. */
	return -1;  /* FAIL */

} /* test_order_err() */


/* test_extra_err()
 *
 * in:     app_name   - name of app to test
 *         app_perfid - app validation function perf ID
 *         tbl_name   - name of test table
 * out:    nothing
 * return: 0 on pass, -1 on fail.
 *
 * Test table validity requirement: all entries following a valid
 * unused entry must also be unused.
 *
 */

static int
test_extra_err(const char *app_name, uint32 app_perfid, const char *tbl_name) {
	
	puts("FILE: create table image with used entry following unused.");
	file_init(tbl_name, TABLE_DESCRIPTION);
	file_set_entry(0, VS_PARM_EAST, 0x00,
		VS_PARM_DIRECTION_MAX, VS_PARM_DIRECTION_MAX); /* valid */
	file_set_entry(2, VS_PARM_APE, 0x00, VS_PARM_ANIMAL_MIN,
		VS_PARM_ANIMAL_MAX);
	file_output(PATH_TO_CF TABLE_FILENAME);
	file_print();

	send_perfstart();
	do {  /* using do/while/break as poor man's try/catch */

		send_load();
		if (expect_load_success(tbl_name))     break;
		send_validate(tbl_name);
		if (expect_err(app_name, VS_TBL_EXTRA_ERR_EID,
			"Table entry 3 parm Ape follows an unused entry"))
			break;
		
		if (expect_validate_failure(app_name, tbl_name, 1, 1, 2))
			break;

		send_perfstop();
		perf_print(app_perfid);
		
		/* If we reach here, the test as a whole passed. */
		return 0;  /* PASS */
		
	} while (0);

	send_perfstop();
	
	/* If we reach here, some test step failed. */
	return -1;  /* FAIL */

} /* test_extra_err() */


/* test_redef_err()
 *
 * in:     app_name   - name of app to test
 *         app_perfid - app validation function perf ID
 *         tbl_name   - name of test table
 * out:    nothing
 * return: 0 on pass, -1 on fail.
 *
 * Test table validity requirement: each parm ID other than
 * VS_PARM_UNUSED can appear in at most one table entry.
 *
 */

static int
test_redef_err(const char *app_name, uint32 app_perfid, const char *tbl_name) {
	
	puts("FILE: create table image with West parm defined twice.");
	file_init(tbl_name, TABLE_DESCRIPTION);
	file_set_entry(0, VS_PARM_WEST, 0x00,
		(VS_PARM_DIRECTION_MAX - VS_PARM_DIRECTION_MIN) / 2,
		VS_PARM_DIRECTION_MAX); /* valid */
	file_set_entry(1, VS_PARM_WEST, 0x00,
		VS_PARM_DIRECTION_MIN, VS_PARM_DIRECTION_MAX);
	file_output(PATH_TO_CF TABLE_FILENAME);
	file_print();

	send_perfstart();
	do {  /* using do/while/break as poor man's try/catch */

		send_load();
		if (expect_load_success(tbl_name))     break;
		send_validate(tbl_name);
		if (expect_err(app_name, VS_TBL_REDEF_ERR_EID,
			"Table entry 2 parm West redefines earlier entry"))
			break;
		
		if (expect_validate_failure(app_name, tbl_name, 1, 1, 2))
			break;

		send_perfstop();
		perf_print(app_perfid);
		
		/* If we reach here, the test as a whole passed. */
		return 0;  /* PASS */
		
	} while (0);

	send_perfstop();
	
	/* If we reach here, some test step failed. */
	return -1;  /* FAIL */

} /* test_redef_err() */


/* test_all_inuse_err()
 *
 * in:     app_name   - name of app to test
 *         app_perfid - app validation function perf ID
 *         tbl_name   - name of test table
 * out:    nothing
 * return: 0 for pass, -1 for fail
 *
 * Test: confirm that the validation function emits multiple error
 * messages when it finds multiple errors and emits them in the proper
 * order, at least for the errors that can occur in in-use entries.
 *
 */

static int
test_all_inuse_err(const char *app_name, uint32 app_perfid,
	const char *tbl_name) {
	
	puts("FILE: create table image with all in-use entry errors.");
	file_init(tbl_name, TABLE_DESCRIPTION);

	/* Start with all fields invalid.  This should trigger
	 * PARM_ERR but not PAD_ERR or any of the bounds errors even
	 * though the field values look like they deserve them.
	 */
	file_set_entry(0, (VS_PARM_DOG|VS_PARM_WEST), 0xFF,
		(VS_PARM_DIRECTION_MAX + 1), (VS_PARM_ANIMAL_MIN -1));

	/* Leaving entry 1 as a valid unused entry to cause in-use
	 * after unused errors in subsequent entries.
	 */

	/* Give entry 2 a valid parm ID but all the pad and bounds errors. */
	file_set_entry(2, VS_PARM_DOG, 0xFF,
		(VS_PARM_DIRECTION_MAX + 1), (VS_PARM_ANIMAL_MIN -1));

	/* Make entry 3 a repeat of entry 2 so that it triggers all of
	 * the same errors plus REDER_ERR.
	 */
	file_set_entry(3, VS_PARM_DOG, 0xFF,
		(VS_PARM_DIRECTION_MAX + 1), (VS_PARM_ANIMAL_MIN -1));
	
	file_output(PATH_TO_CF TABLE_FILENAME);
	file_print();

	send_perfstart();
	do {  /* using do/while/break as poor man's try/catch */

		send_load();
		if (expect_load_success(tbl_name))     break;
		send_validate(tbl_name);

		/* Confirm entry 1 errors. */
		if (expect_err(app_name, VS_TBL_PARM_ERR_EID,
			"Table entry 1 invalid Parm ID"))
			break;

		/* Confirm entry 3 errors. */
		if (expect_err(app_name, VS_TBL_PAD_ERR_EID,
			"Table entry 3 parm Dog padding not zeroed"))
			break;
		if (expect_err(app_name, VS_TBL_LBND_ERR_EID,
			"Table entry 3 parm Dog invalid low bound"))
			break;
		if (expect_err(app_name, VS_TBL_HBND_ERR_EID,
			"Table entry 3 parm Dog invalid high bound"))
			break;
		if (expect_err(app_name, VS_TBL_ORDER_ERR_EID,
			"Table entry 3 parm Dog invalid bound order"))
			break;
		if (expect_err(app_name, VS_TBL_EXTRA_ERR_EID,
			"Table entry 3 parm Dog follows an unused entry"))
			break;

		/* Confirm entry 4 errors - same as entry 3, plus REDEF. */
		if (expect_err(app_name, VS_TBL_PAD_ERR_EID,
			"Table entry 4 parm Dog padding not zeroed"))
			break;
		if (expect_err(app_name, VS_TBL_LBND_ERR_EID,
			"Table entry 4 parm Dog invalid low bound"))
			break;
		if (expect_err(app_name, VS_TBL_HBND_ERR_EID,
			"Table entry 4 parm Dog invalid high bound"))
			break;
		if (expect_err(app_name, VS_TBL_ORDER_ERR_EID,
			"Table entry 4 parm Dog invalid bound order"))
			break;
		if (expect_err(app_name, VS_TBL_EXTRA_ERR_EID,
			"Table entry 4 parm Dog follows an unused entry"))
			break;
		if (expect_err(app_name, VS_TBL_REDEF_ERR_EID,
			"Table entry 4 parm Dog redefines earlier entry"))
			break;

		/* Confirm overall validation failure and stats. */
		if (expect_validate_failure(app_name, tbl_name, 0, 3, 1))
			break;

		send_perfstop();
		perf_print(app_perfid);
		
		/* If we reach here, the test as a whole passed. */
		return 0;  /* PASS */
		
	} while (0);

	send_perfstop();
	
	/* If we reach here, some test step failed. */
	return -1;  /* FAIL */

} /* test_all_inuse_err() */


/* ------------------- module exported functions ----------------------- */

/* deterministic()
 *
 * in:     app_name   - name of app to test
 *         app_perfid - performance ID of app to test
 *         tbl_name   - name of test table
 * out:    nothing
 * return: 0 if all tests passed, -1 if at least one test failed
 *
 * Runs a deterministic series of table validation tests.
 * "Deterministic" means the function runs the same tests each time
 * it's invoked - they are not randomized, stochastic, or fuzz tests.
 *
 */

int
deterministic(const char *app_name, uint32 app_perfid, const char *tbl_name) {

	int result = 0;  /* optimistically presume all tests will pass */

	/* If initialization fails, quit without running further tests. */
	if (initialize(app_name, app_perfid)) return -1;

	if (test_control_flow_valid_table(app_name, app_perfid, tbl_name))
		result = -1;
	if (test_control_flow_invalid_table(app_name, app_perfid, tbl_name))
		result = -1;
	if (test_zero_err(app_name, app_perfid, tbl_name))      result = -1;
	if (test_parm_err(app_name, app_perfid, tbl_name))      result = -1;
	if (test_pad_err(app_name, app_perfid, tbl_name))       result = -1;
	if (test_lbnd_err(app_name, app_perfid, tbl_name))      result = -1;
	if (test_hbnd_err(app_name, app_perfid, tbl_name))      result = -1;
	if (test_order_err(app_name, app_perfid, tbl_name))     result = -1;
	if (test_extra_err(app_name, app_perfid, tbl_name))     result = -1;
	if (test_redef_err(app_name, app_perfid, tbl_name))     result = -1;
	if (test_all_inuse_err(app_name, app_perfid, tbl_name)) result = -1;

	if (result) {
		puts("At least one test failed.");
	} else {
		puts("All tests passed.");
	}

	return result;

} /* deterministic() */
