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

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>


/* Constants for the Linux /proc file for the POSIX message queue max
 * depth setting and a minimum value that should allow this program to
 * operate correctly.
 */
#define MSG_MAX_FILE "/proc/sys/fs/mqueue/msg_max"
#define MSG_MAX_SAFE 50
#define BUF_LEN  32  /* size of buffer to hold value in string form */


/* warn_pipe_depth()
 *
 * in:     nothing
 * out:    nothing
 * return: nothing
 *
 * This function attempts to confirm that the simulated spacecraft's
 * message pipe depth is large enough to avoid Pipe Overflow errors
 * during runtime.  These errors look like this in the simulated
 * spacecraft's console output:
 *
 *   EVS Port1 66/1/CFE_SB 25: Pipe Overflow,MsgId 0x808,pipe
 *   TO_LAB_TLM_PIPE,sender VSA_APP
 *
 * Pipe overflows can occur when apps send too many events to TO_LAB
 * at the same time.  For example, the combination of all app
 * housekeeping telemetry messages plus three or more validation
 * function telemetry messages will reliably overflow TO_LAB's
 * telemetry pipe with my dev box's default configuration.
 *
 * TO_LAB telemetry pipe overflows can cause TO_LAB to fail to deliver
 * telemetry messages to this test program.  This failure can cause
 * this test program to declare spurious test failures in cases where
 * the tests should pass - a misleading result that might prompt a
 * painful debugging wild goose chase.
 *
 * The simulated spacecraft's pipe depth is determined by the kernel's
 * POSIX/System V message queue depth parm.  By default, this function
 * emits a warning message to the console that scolds the user about
 * making sure this parm is set to a large enough value.  It will skip
 * emitting this message if it can confirm the parm is correctly set
 * by performing a Linux-specific check via /proc.
 *
 */

void
warn_pipe_depth(void) {

	FILE *fp;           /* for parm file in /proc */
	char buf[BUF_LEN];  /* parm value as a NUL terminated string */
	unsigned long val;  /* parm value as an unsigned long */
	
	do {  /* Using do/while/break as poor man's try/catch */
	
		if (!(fp = fopen(MSG_MAX_FILE, "r"))) break;
		if (!(fgets(buf, BUF_LEN, fp))) break;
		errno = 0;
		val = strtoul(buf, NULL, 10);
		if (errno) break;
		if (val < MSG_MAX_SAFE) break;

		/* If we reach here, the message queue depth is
		 * configured to a safe value; return without printing
		 * the warning message.
		 */
		return;

	} while (0);

	/* If we reach here, either we couldn't determine the message
	 * queue depth parm value or we determined it was set too low.
	 * Print the warning message.
	 */
	printf("WARN: Configure your kernel's POSIX message queue "
		"depth to at least %u\n", MSG_MAX_SAFE);

	return;

} /* warn_pipe_depth() */
