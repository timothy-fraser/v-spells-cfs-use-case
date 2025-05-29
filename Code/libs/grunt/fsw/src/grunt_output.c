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

#include "grunt_status.h"
#include "grunt.h"
#include "grunt_stack.h"
#include "grunt_output.h"


/* grunt_number_t is a 32-bit unsigned quantity. It's maximum (and
 * thus longest) value is 4,294,967,295, which requires 10 characters
 * + 1 for a NUL terminator to express.  NOTE: If you change the size
 * of grunt_number_t, you'll need to change this constant to match.
 *
 * This constant defines an adequate size for temporary buffers to
 * hold the string version of grunt_number_t numbers.
 */
#define NUMBER_BUFFER_SIZE 16       /* Need only 11, but 16 is word-aligned. */

/* Grunt programs define a "string table": an array of constant
 * strings.  Whent hey want to enqueue one of these strings to the
 * output queue during runtime, they refer to them by their index in
 * the string table array.
 */
static const char **g_string_table;      /* Grunt program's table of strings */
static grunt_rep_t g_num_strings;       /* number of strings in string table */

/* The Grunt interpreter maintains an "output queue" which holds an
 * initially-empty NUL-terminated C string.  Grunt programs build
 * strings to output by popping values from the stack and enqueing
 * themon (appending them to) this queue.  The Grunt interpreter also
 * has an "input queue", but despite their both being queues we've
 * kept their implementations separate since the input queue is
 * dequeue-only and this output queue is enqueue-only.
 */
#define OUTPUT_QUEUE_SIZE CFE_MISSION_EVS_MAX_MESSAGE_LENGTH
static char g_output_queue[OUTPUT_QUEUE_SIZE]; /* strings constructed here */
static grunt_rep_t g_tail_index;     /* index of terminating NUL in buffer */


/* ------------------- module local functions -------------------- */

static void
grunt_output_reset(void) {

	memset(g_output_queue, '\0', OUTPUT_QUEUE_SIZE);
	g_tail_index = 0;

} /* grunt_output_reset() */


/* grunt_output_enqueue()
 *
 * in:     string - string to append to string buffer
 *         length - length of string, not including terminating NUL
 * out:    g_output_queue - string (might be) appended
 *         g_tail_index     - (might be) incremented by length
 * return: value                    condition
 *         -------------            ---------------
 *         GRUNT_ERROR_OUTOFBOUNDS  There is not enough room for string
 *         0                        Success
 *
 * If the output queue has enough room for string, appends string to
 * output queue.  Otherwise, returns error and leaves output queue
 * unchanged.
 *
 */

static int
grunt_output_enqueue(const char *string, grunt_rep_t length) {

	/* If appending the indicated string would exceed the length
	 * of our output queue, report an overflow and refuse to
	 * append.
	 */
	if (g_tail_index + length > (OUTPUT_QUEUE_SIZE - 1)) {
		return GRUNT_ERROR_OUTOFBOUNDS;
	}

	memcpy(&(g_output_queue[g_tail_index]),	string,	length);
	g_tail_index += length;
	return 0;  /* OK! */

} /* grunt_output_enqueue() */


/* ------------------- module exported functions -------------------- */


void
grunt_output_init(const char *string_table[], grunt_string_t num_strings) {

	g_string_table = string_table;
	g_num_strings  = num_strings;
	grunt_output_reset();

} /* grunt_output_init() */


int
grunt_output_enqueue_boolean(grunt_boolean_t tf) {

	if (tf) {
		return grunt_output_enqueue("true", 4);
	} else {
		return grunt_output_enqueue("false", 5);
	}

} /* grunt_output_enqueue_boolean() */


int
grunt_output_enqueue_number(grunt_number_t u) {

	char string[NUMBER_BUFFER_SIZE];  /* receives string form of u */
	int length;       /* receives ideal length of string form of u */

	/* snprintf() will return the ideal length of the string
	 * representation of the number u given sufficient space.
	 * However, it will actually write no more than
	 * NUMBER_BUFFER_SIZE - 1 digit characters followed by NUL.
	 */
	if (-1 == (length = snprintf(string, NUMBER_BUFFER_SIZE, "%u", u)))
		return GRUNT_ERROR_INTERPRETERBUG;

	if (length > GRUNT_REP_MAX) return GRUNT_ERROR_INTERPRETERBUG;
	
	return grunt_output_enqueue(string, length);
	
} /* grunt_output_enqueue_number() */


int
grunt_output_enqueue_string(grunt_string_t string_index) {

	grunt_rep_t length;

	/* Make sure we're trying to append a string that is in our
	 * string table.
	 */
	if (!(string_index < g_num_strings)) return GRUNT_ERROR_INVALIDLITERAL;

	/* Get the length of the string.  Fail if string
	 * is too long for our chosen index variable type.
	 */
	if (GRUNT_REP_MAX < (length = strlen(g_string_table[string_index])))
		return GRUNT_ERROR_OUTOFBOUNDS;
		
	return grunt_output_enqueue(g_string_table[string_index], length);
	
} /* grunt_output_enqueue_string() */


void
grunt_output_flush(grunt_number_t event_type, grunt_number_t event_id) {
	
	CFE_EVS_SendEvent(event_id, event_type, "%s", g_output_queue);

	grunt_output_reset();
	
} /* grunt_output_flush() */
