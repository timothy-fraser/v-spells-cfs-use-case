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
#include "grunt_output.h"
#include "grunt_vm_io.h"

int
grunt_vm_flush(grunt_value_t *p_ra, grunt_value_t *p_rb) {

	int status;

	/* Pop the Event ID */
	if ((status = grunt_stack_arg_pop(p_ra))) return status;
	if (p_ra->type != gt_num) return GRUNT_ERROR_INVALIDARGUMENT;

	/* Pop Event Type */
	if ((status = grunt_stack_arg_pop(p_rb))) return status;
	if (p_rb->type != gt_num) return GRUNT_ERROR_INVALIDARGUMENT;

	/* Flush */
	grunt_output_flush(p_ra->val.num, p_rb->val.num);

	return 0;  /* OK! */
	
} /* grunt_vm_flush() */


int
grunt_vm_input(grunt_value_t *p_ra, grunt_rep_t n) {

	int status;
	
	/* We support reads of only 4-, 2-, and 1-byte unsigned
	 * integers.  Asking for any other size is an error.
	 */
	if ((n != 4) && (n != 2) && (n != 1))
		return GRUNT_ERROR_INVALIDLITERAL;

	/* Dequeue the next input value and push it onto the stack. */
	if ((status = grunt_input_dequeue(p_ra, n))) return status;
	return grunt_stack_arg_push(p_ra);
	
} /* grunt_vm_input() */


int
grunt_vm_output(grunt_value_t *p_ra) {

	int status;

	/* Pop the top element off of the stack. */
	if ((status = grunt_stack_arg_pop(p_ra))) return status;

	/* Enqueue it on the output queue. */
	switch (p_ra->type) {
	case gt_bool:
		return grunt_output_enqueue_boolean(p_ra->val.b);
	case gt_num:
		return grunt_output_enqueue_number(p_ra->val.num);
	case gt_str:
		return grunt_output_enqueue_string(p_ra->val.str);
	default:
		/* You can't output elements of type gt_pc. */
		return GRUNT_ERROR_INVALIDARGUMENT;
	}

} /* grunt_vm_output() */


int
grunt_vm_rewind(grunt_rep_t reps) {

	return grunt_input_rewind(reps);
	
} /* grunt_vm_rewind() */
