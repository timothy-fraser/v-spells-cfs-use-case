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

#include "cfe.h"
#include "cfe_evs_extern_typedefs.h"   /* for CFE_EVS_EventType_* enum */
#include "to_lab_events.h"             /* for TO_LAB event IDs */

#include "vs_ground.h"                 /* for app names, perf IDs */
#include "vs_tablestruct.h"            /* for raw table name */

#include "mqueue.h"
#include "cmd.h"
#include "tlm.h"
#include "expect.h"
#include "deterministic.h"


int
main(int argc, char *argv[]) {

	/* Warn about kernel POSIX message queue depth setting. */
	warn_pipe_depth();
	
	/* Initialize our command and telemetry sockets. */
	tlm_init();
	cmd_init();

	/* If there are no command-line arguments, run tests on VSA by
	 * default.
	 */
	if (argc == 1)
		return deterministic(VSA_APP_NAME, VSA_VF_PERF_ID,
			VSA_APP_NAME "." VS_RAW_TABLE_NAME);

	/* If there is one command-line argument, see if it is a flag
	 * that tells us which app to test.  If it does, test that
	 * app.
	 */
	if (argc == 2) {
		if (!strcmp("--vsa", argv[1])) {
			return deterministic(VSA_APP_NAME, VSA_VF_PERF_ID,
				VSA_APP_NAME "." VS_RAW_TABLE_NAME);
		} else 		if (!strcmp("--vsb", argv[1])) {
			return deterministic(VSB_APP_NAME, VSB_VF_PERF_ID,
				VSB_APP_NAME "." VS_RAW_TABLE_NAME);
		} else 		if (!strcmp("--vsc", argv[1])) {
			return deterministic(VSC_APP_NAME, VSC_VF_PERF_ID,
				VSC_APP_NAME "." VS_RAW_TABLE_NAME);
		}
	}

	/* If we wind up here there was something wrong with the
	 * command-line arugments.  Print a help message.
	 */
	fprintf(stderr, "Usage:\n");
	fprintf(stderr,"\ttbltest       : "
		"test %s TLM HK MID 0x%08X Perf ID 0x%08X\n",
		VSA_APP_NAME, VSA_TLM_HK_MID, VSA_VF_PERF_ID);
	fprintf(stderr,"\ttbltest --vsa : "
		"test %s TLM HK MID 0x%08X Perf ID 0x%08X\n",
		VSA_APP_NAME, VSA_TLM_HK_MID, VSA_VF_PERF_ID);
	fprintf(stderr,"\ttbltest --vsb : "
		"test %s TLM HK MID 0x%08X Perf ID 0x%08X\n",
		VSB_APP_NAME, VSB_TLM_HK_MID, VSB_VF_PERF_ID);
	fprintf(stderr,"\ttbltest --vsc : "
		"test %s TLM HK MID 0x%08X Perf ID 0x%08X\n",
		VSC_APP_NAME, VSC_TLM_HK_MID, VSC_VF_PERF_ID);
	return -1;
	
} /* main() */
