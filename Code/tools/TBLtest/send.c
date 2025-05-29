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

/* The functions in this file use the general cmd_*() functions from
 * cmd.c to send the ground commands needed to test our specific app's
 * table validation function.  They also emit output to the console so
 * the user can follow along.
 */

#include <stdio.h>

#include "cfe.h"
#include "cfe_tbl_msg.h"               /* for TBL command codes */
#include "vs_ground.h"                 /* for app names and perf IDs */

#include "cmd.h"
#include "send.h"
#include "perf.h"
#include "tbltest.h"


/* send_perfmon()
 *
 * in:     app_name   - name of app whose performance we want ES to track
 *         app_perfid - performance ID we want ES to track
 * out:    nothing
 * return: nothing
 *
 * To make ES monitor a particular performance ID, you must properly
 * set both its perf filter mask and its perf trigger mask.  Doing so
 * requires sending multiple commands.  If ES encounters no errors, it
 * does not respond with telemetry.  This function bundles up all the
 * commands needed to make ES monitor the one perf ID we care about
 * and no others.  There is no corresponding telemetry response to
 * 'expect'.
 */

void
send_perfmon(const char *app_name, uint32 app_perfid) {

	uint32 word_num = (uint32)(app_perfid / 32);
	uint32 word_mask = 0x01 << (app_perfid % 32);

	/* Tell ES that, when we turn perf logging on, we want it to
	 * log perf events from our app.
	 */
	printf("INIT: Tell ES we care about %s performance.\n", app_name);
	printf("SENT:      CFE_ES  CMD  PCARE %s\n", app_name);

	/* Clear the lower three words of the ES perf filter mask mask
	 * to turn off all the default perf logging.
	 */
	cmd_es_setperffilter(0, 0x00);
	cmd_es_setperffilter(1, 0x00);
	cmd_es_setperffilter(2, 0x00);

	/* Clear the lower three words of the ES perf trigger mask to
	 * turn off all the default perf logging.
	 */
	cmd_es_setperftrigger(0, 0x00);
	cmd_es_setperftrigger(1, 0x00);
	cmd_es_setperftrigger(2, 0x00);

	/* Set our app's bit in the ES perf filter and trigger masks
	 * so that when we turn on logging, ES will log our app's
	 * events.
	 */
	cmd_es_setperffilter(word_num, word_mask);
	cmd_es_setperftrigger(word_num, word_mask);
	
} /* send_perfmon() */


/* send_perfstart()
 *
 * To measure validation execution time, a test must send this command
 * before sending the validate command.
 *
 */

void
send_perfstart(void) {

	/* Tell ES to start storing CFE_ES_PerfLogEntry/Exit() events. */
	puts("PERF: start storing performance events.");
	puts("SENT:      CFE_ES  CMD  PSTRT");
	cmd_es_perfstart();

} /* send_perfstart() */


/* send_perfstop()
 *
 * To measure validation execution time, a test must send this command
 * after receiving the validation function results telemetry.  Resist
 * the temptation to send this command immedately after sending the
 * validation command.  If you do so, ES will receive the perf stop
 * command before the app's housekeeping routine gets around to
 * actually executing the validation function and ES will not collect
 * any performance data.
 *
 */

void
send_perfstop(void) {

	/* Tell ES to stop storing CFE_ES_PerfLogEntry/Exit() events
	 * and write the events it saw out to its default file.
	 */
	puts("PERF: stop storing performance events.");
	puts("SENT:      CFE_ES  CMD  PSTOP");
	cmd_es_perfstop();

} /* send_perfstop() */


void
send_tlmon(void) {

	/* Tell TO/TO_LAB to turn telemetry output on */
	puts("INIT: tell TO_LAB to turn telemetry output on");
	puts("SENT:   TO_LAB_APP CMD  TLMON 127.0.0.1");
	cmd_to_tlmon();

} /* send_tlmon() */
	

void
send_load(void) {

	/* Tell TBL to load table */
	puts("TEST: load file into inactive image.");
	puts("SENT:      CFE_TBL CMD  LOAD  " TABLE_FILENAME);
	cmd_tbl_load(TABLE_FILENAME);

} /* send_load() */
	

/* send_validate()
 *
 * in:     tbl_name - full app.tbl name of table we want TBL to validate
 * out:    nothing
 * return: nothing
 *
 * Send command telling TBL to validate the table named by `tbl_name'.
 */

void
send_validate(const char *tbl_name) {

	/* Tell TBL to validate the table's inactive image. */
	printf("TEST: validate inactive image.\n");
	printf("SENT:      CFE_TBL CMD  VALID %s\n", tbl_name);
	cmd_tbl_validate(tbl_name, CFE_TBL_BufferSelect_INACTIVE);

} /* send_validate() */


/* send_activate()
 *
 * in:     tbl_name - full app.tbl name of table we want TBL to activate
 * out:    nothing
 * return: nothing
 *
 * Send command telling TBL to activate the table named by `tbl_name'.
 */

void
send_activate(const char *tbl_name) {

	/* Tell TBL to activate a valid inactive image. */
	printf("TEST: activate valid inactive image.\n");
	printf("SENT:      CFE_TBL CMD  ACTIV %s\n", tbl_name);
	cmd_tbl_activate(tbl_name);

} /* send_activate() */




