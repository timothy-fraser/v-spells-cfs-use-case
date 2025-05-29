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

/* Tests operate in a series of send-expect steps where they first
 * invoke a send_*() function from send.c to send a command to the
 * simulated spacecraft and then invoke an expect_*() function from
 * this module to watch for a specific telemetry message in response.
 *
 * Tests specify a particular telemetry message that indicates the
 * test passed.  The spacecraft's many services and apps are quite
 * chatty.  In cases where the test passes, quite a few telemety
 * messages may arrive before the expected one.  The expect_*()
 * functions will print a brief description of each irrelevant message
 * to the console so the user can see that the simulation is making
 * progress.
 *
 * In cases where the test fails, the simulated spacecraft will never
 * send the expected message.  The expect_() functions will quit
 * waiting after a seeing a particular number of unsatisfactory
 * messages and declare test failure.  In these situations, one of the
 * unsatisfactory messages may have described the problem that caused
 * the test failure and the console output may help debugging.
 * 
 * These expect_*() functions emit "PASS"/"FAIL" output to the console
 * to help the user track the test results.
 *
 * All of the telemety messages our tests expect to see have topic ID
 * CFE_MISSION_EVS_LONG_EVENT_MSG_MSG and convey the name of the app
 * that originated the message along with an app-specific event type,
 * event id, and message string.  This module's function focus on
 * supporting this particular type of message.
 */

#include <assert.h>
#include <stdio.h>

#include "cfe.h"
#include "cfe_evs_extern_typedefs.h"   /* for CFE_EVS_EventType_* enum */
#include "cfe_evs_topicids.h"          /* for EVS LONG message topic ID */
#include "to_lab_events.h"             /* for TO_LAB event IDs */
#include "cfe_tbl_eventids.h"          /* for TBL event IDs */
#include "vs_ground.h"                 /* for app name constants */
#include "vs_eventids.h"               /* for common VS event ID constants */

#include "tlm.h"
#include "expect.h"
#include "tbltest.h"


/* ------------- module local definitions and functions ------------ */

/* If the spacecraft doesn't respond correctly to a test command
 * within this many telemetry messages, declare test failure.  This
 * number has to be fairly large to handle the case where the
 * telemetry response we want is held up behind the housekeeping
 * telemetry messages from a dozen apps.
 */
#define RECEIVE_LIMIT 128

/* When we pretty-print to the console, we take care to keep our
 * output within a single line of this many characters.
 */
#define CONSOLE_LINE_LENGTH 80


/* Many of our functions use this buffer to construct error message
 * strings that they expect to see in telemetry messages from a
 * combination of constant strings and strings passed as parameters.
 */  
static char message[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH];


/* print_evs_long()
 *
 * in:     prefix - prefix to start console line, "WANT" or "SEEN"
 *         appname, eventtype, eventid, message - fields from message
 * out:    nothing
 * return: nothing
 *
 * This function pretty-prints messages to the console, taking care
 * not to exceed the length of a console line for the sake of
 * readability.
 */
 
static void
print_evs_long(const char *prefix, const char *appname,
	tlm_eventtype_t eventtype, tlm_eventid_t eventid,
	const char *message) {

	char output[CONSOLE_LINE_LENGTH + 1];

	snprintf(output, CONSOLE_LINE_LENGTH, "%5s %12s %4s %5s %s",
		prefix, appname, tlm_eventtype_to_string(eventtype),
		tlm_eventid_to_string(appname, eventid), message);
	output[CONSOLE_LINE_LENGTH] = '\0';  /* ensure termination */

	puts(output);

} /* print_evs_long() */


/* expect()
 *
 * in:     want_appname, want_eventtype, want_eventid, want_message -
 *         the field values of the CFE_MISSION_EVS_LONG_EVENT_MSG_MSG
 *         telemetry message that indicates test pass.
 * out:    nothing
 * return: value    condition
 *         -----    ---------
 *           0      PASS, that is, received a matching message
 *          -1      FAIL, that is, didn't receive a matching message
 *                  within in the number of messages we're allowed
 *                  to wait through.
 *
 * The most general of the expect_*() functions.  Will receive
 * telemetry messages until one that matches the input parms arrives
 * (PASS), or too many non-matching messages arrive (FAIL).
 */

static int
expect(const char *want_appname, tlm_eventtype_t want_eventtype,
	tlm_eventid_t want_eventid, const char *want_message) {

	int receive_count;       /* counts messages received so far */

	/* The following vars hold important received message field values */
	tlm_topicid_t   seen_topicid;
	const char *    seen_appname;
	tlm_eventtype_t seen_eventtype;
	tlm_eventid_t   seen_eventid;
	const char *    seen_message;

	/* Describe the telemetry response we're looking for. */
	print_evs_long("WANT:", want_appname, want_eventtype, want_eventid,
		want_message);

	/* Receive and print telemetry messages until
         *  (PASS) we receive a telemetry message that matches the
	 *         conditions we want, or
	 *  (FAIL) we receive RECEIVE_LIMIT telemetry messages.
	 */
	for (receive_count = 0; receive_count < RECEIVE_LIMIT;
		receive_count++) {

		tlm_receive();
		seen_topicid = tlm_topicid();

		/* If this isn't a long-form EVS message, it can't be
		 * the message we want.  Print its topicid and move
		 * on to the next message.
		 */
		if (seen_topicid != CFE_MISSION_EVS_LONG_EVENT_MSG_MSG) {
			printf("SEEN: %66s\n",
			       tlm_topicid_to_string(seen_topicid));
			continue;
		}

		/* We've got a long-form EVS message, retrieve the
		 * fields we care about.
		 */
		seen_appname   = tlm_evs_appname();
		seen_eventtype = tlm_evs_eventtype();
		seen_eventid   = tlm_evs_eventid();
		seen_message   = tlm_evs_message();
		
		/* Pretty-print the message. */
		print_evs_long("SEEN:", seen_appname, seen_eventtype,
			seen_eventid, seen_message);
		
		if ((!strcmp(tlm_evs_appname(), want_appname)) &&
		    (tlm_evs_eventtype() == want_eventtype) &&
		    (tlm_evs_eventid() == want_eventid) &&
		    (!strcmp(tlm_evs_message(),	want_message))) {
			puts("PASS.");
			return 0;  /* PASS */
		}
		
	} /* For messages up to RECEIVE_LIMIT */

	puts("FAIL.");
	return -1;  /* FAIL */

} /* expect() */


/* ------------------ module exported functions ---------------------- */


/* expect_tlmon_success()
 *
 * in:     nothing
 * out:    nothing
 * return: 0 (PASS) if command succeeded, else -1 (FAIL).
 *
 * Use this function to confirm that the cFS TO/TO_LAB
 * Telemety-forwarding app turned its telemetry forwarding on as
 * commanded.
 *
 */
 
int
expect_tlmon_success(void) {
	
	return expect(TLM_NAME_TO, CFE_EVS_EventType_INFORMATION,
		TO_LAB_TLMOUTENA_INF_EID,
		"TO telemetry output enabled for IP 127.0.0.1");

} /* expect_tlmon_success() */


/* expect_load_success()
 *
 *         tbl_name      - full app.tbl name of table
 * out:    message       - overwrote with expected telemetry message
 * return: 0 (PASS) if command succeeded, else -1 (FAIL).
 *
 * Use this function to confirm that the cFE TLM Table Service loaded
 * our table file as commanded.
 */

int
expect_load_success(const char *tbl_name) {
	
	snprintf(message, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
		"Successful load of '%s' into '%s' working buffer",
		TABLE_FILENAME, tbl_name);
	assert(message[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH-1] == '\0');

	return expect(TLM_NAME_TBL, CFE_EVS_EventType_INFORMATION,
		CFE_TBL_FILE_LOADED_INF_EID, message);
	
} /* expect_load_success() */


/* expect_activate_success()
 *
 * in:     app_name      - name of app 
 *         tbl_name      - full app.tbl name of table
 * out:    message       - overwrote with expected telemetry message
 * return: 0 (PASS) if command succeeded, else -1 (FAIL).
 *
 * Use this function to confirm that the cFE TLM Table Service
 * activated a validated table image as commanded.
 */

int
expect_activate_success(const char *app_name, const char *tbl_name) {

	snprintf(message, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
		"%s Successfully Updated '%s'", app_name, tbl_name);
	assert(message[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH-1] == '\0');

	return expect(TLM_NAME_TBL, CFE_EVS_EventType_INFORMATION,
		CFE_TBL_UPDATE_SUCCESS_INF_EID, message);

} /* expect_activate_success() */


/* expect_activate_failure()
 *
 * in:     tbl_name      - full app.tbl name of table
 * out:    message       - overwrote with expected telemetry message
 * return: 0 (PASS) if command did *not* succeed, else -1 (FAIL).
 *
 * Use this function to confirm that the cFE TLM Table Service
 * *refused* to activate an unvalidated table image as commanded.
 */

int
expect_activate_failure(const char *tbl_name) {
	
	snprintf(message, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
		"Cannot activate table '%s'. "
		"Inactive image not Validated", tbl_name);
	assert(message[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH-1] == '\0');

	return expect(TLM_NAME_TBL, CFE_EVS_EventType_ERROR,
		CFE_TBL_UNVALIDATED_ERR_EID, message);

} /* expect_activate_failure() */


/* expect_validate_success()
 *
 * in:     app_name      - name of app we expect to generate validation 
 *         tbl_name      - full app.tbl name of validated table
 *         count_valid   - number of expected valid entries
 *         count_invalid - number of expected invalid entries
 *         count_unused  - number of expected unused entries
 * out:    message       - overwrote with expected telemetry message
 * return: 0 (PASS) if validation found a valid image, else -1 (FAIL).
 *
 * Use this function to confirm that the cFE TLM Table Service invoked
 * our app's validation function on its table's inactive image and
 * found it to be valid.
 */

int
expect_validate_success(const char *app_name, const char *tbl_name,
	unsigned count_valid, unsigned count_invalid, unsigned count_unused) {


	do { /* using do/while/break as a poor man's try/catch */
	
		snprintf(message, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
			 "Table image entries: "
			 "%u valid, %u invalid, %u unused",
			 count_valid, count_invalid, count_unused);
		assert(message[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH-1] == '\0');
		if (expect(app_name, CFE_EVS_EventType_INFORMATION,
			VS_VALIDATION_INF_EID, message))
			break;

		snprintf(message, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
			 "%s validation successful for Inactive '%s'",
			 app_name, tbl_name);
		assert(message[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH-1] == '\0');
		if (expect(TLM_NAME_TBL, CFE_EVS_EventType_INFORMATION,
			CFE_TBL_VALIDATION_INF_EID, message)) 
			break;

		return 0;  /* PASS */

	} while (0);

	return -1;  /* FAIL */

} /* expect_validate_success() */


/* expect_validate_failure()
 *
 * in:     app_name      - name of app we expect to generate validation failure
 *         tbl_name      - full app.tbl name of table we expect to have failed
 *         count_valid   - number of expected valid entries
 *         count_invalid - number of expected invalid entries
 *         count_unused  - number of expected unused entries
 * out:    message       - overwrote with expected telemetry message
 * return: 0 (PASS) if validation found a *invalid* image, else -1 (FAIL).
 *
 * Use this function to confirm that the cFE TLM Table Service invoked
 * our app's validation function on its table's inactive image and
 * found it to be *invalid*.
 */

int
expect_validate_failure(const char *app_name, const char *tbl_name,
	unsigned count_valid, unsigned count_invalid, unsigned count_unused) {

	
	do { /* using do/while/break as a poor man's try/catch */
	
		snprintf(message, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
			 "Table image entries: "
			 "%u valid, %u invalid, %u unused",
			 count_valid, count_invalid, count_unused);
		assert(message[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH-1] == '\0');
		if (expect(app_name, CFE_EVS_EventType_INFORMATION,
			VS_VALIDATION_INF_EID, message))
			break;

		snprintf(message, CFE_MISSION_EVS_MAX_MESSAGE_LENGTH,
			 "%s validation failed for "
			 "Inactive '%s', Status=0xFFFFFFFF",
			 app_name, tbl_name);
		assert(message[CFE_MISSION_EVS_MAX_MESSAGE_LENGTH-1] == '\0');
		if (expect(TLM_NAME_TBL, CFE_EVS_EventType_ERROR,
			CFE_TBL_VALIDATION_ERR_EID, message))
			break;

		return 0;  /* PASS */

	} while (0);

	return -1;  /* FAIL */
	
} /* expect_validate_failure() */


/* expect_err()
 *
 * in:     want_app_name - name of app we expect to emit the err
 *         want_eventid  - error event ID we expect to see
 *         want_message  - message string we expect to see
 * out:    nothing
 * return: 0 (PASS) if the expected error message arrives, else -1 (FAIL).
 *
 * Use this function to confirm that our app's validation function
 * found and reported a particular kind of validation violation.
 *
 */

int
expect_err(const char *want_app_name, tlm_eventid_t want_eventid,
	const char *want_message) {

	return expect(want_app_name, CFE_EVS_EventType_ERROR,
		      want_eventid, want_message);

} /* expect_err() */
