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
#include "grunt_input.h"


/* Grunt programs take CFS/CFE/TBL table images - essentially buffers
 * in memory - as input.  The Grunt interpreter has an "input queue"
 * that treats these buffers as queues and enables Grunt programs to
 * read them from beginning to end, dequeing values of specific sizes
 * one at a time.  It also allows them to "rewind" the head of the
 * queue so they can parse the same input multiple times.  The Grunt
 * interpreter also has an "output queue".  We're keeping their
 * implementations separate despite their both being queues since this
 * input queue is dequeue-only and the output queue is enqueue-only.
 */
static const char *g_input_queue      = 0;  /* the input data queue */
static grunt_rep_t g_input_queue_size = 0;  /* size of input data in bytes */
static grunt_rep_t g_head_index       = 0;  /* index of next char to dequeue */


int
grunt_input_rewind(grunt_rep_t n) {

	/* Make sure we don't rewind past the start of the queue */
	if (n > g_head_index) return GRUNT_ERROR_OUTOFBOUNDS;
	
	if (n == 0) {
		g_head_index = 0;   /* REWIND 0 means rewind to start */
	} else {
		g_head_index -= n;  /* REWIND >0 means rewind by that much */
	}

	return 0;   /* OK! */
	
} /* grunt_input_rewind() */


void
grunt_input_init(const void *p_data, grunt_rep_t size) {

	g_input_queue = (const char *)p_data;
	g_input_queue_size = size;
	grunt_input_rewind(0);
	
} /* grunt_input_init() */


int
grunt_input_dequeue(grunt_value_t *p_v, grunt_rep_t n) {

	/* Make sure we've been initialized. */
	if (!g_input_queue) return GRUNT_ERROR_INTERPRETERBUG;

	/* Grunt allows only 1-, 2-, or 4-byte reads.  Rule out this
	 * error condition before checking the bounds.
	 */
	if (!((n == 1)||(n == 2)||(n == 4))) return GRUNT_ERROR_INVALIDLITERAL;
	
	/* Avoid reading off the end of the input data queue. */
	if ((g_head_index + n) > g_input_queue_size)
		return GRUNT_ERROR_OUTOFBOUNDS;

	/* Read a number of the specified size. */
	p_v->type = gt_num;
	
	switch (n) {
	case 4:
		p_v->val.num = *((uint32_t *)(&(g_input_queue[g_head_index])));
		break;
	case 2:
		p_v->val.num = *((uint16_t *)(&(g_input_queue[g_head_index])));
		break;
	case 1:
		p_v->val.num = *((uint8_t *)(&(g_input_queue[g_head_index])));
		break;
	default:
		return GRUNT_ERROR_INTERPRETERBUG; /* checked this above */
	}

	g_head_index += n;  /* we've consumed n bytes of input */
	return 0;       /* OK! */

} /* grunt_input_dequeue() */
