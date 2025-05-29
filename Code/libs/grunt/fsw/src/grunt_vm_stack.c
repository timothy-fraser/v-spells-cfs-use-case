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
#include "grunt_vm_stack.h"


int
grunt_vm_dup(grunt_rep_t reps) {

	/* The minimum number of reps is 1. */
	if (reps < 1) return GRUNT_ERROR_INVALIDLITERAL;

	return grunt_stack_arg_dup(reps);
	
} /* grunt_vm_dup() */


int
grunt_vm_pop(grunt_value_t *p_ra, grunt_rep_t reps) {

	grunt_rep_t i;
	int status;

	/* The minimum number of reps is 1. */
	if (reps < 1) return GRUNT_ERROR_INVALIDLITERAL;

	for (i = 0; i < reps; i++) {
		if ((status = grunt_stack_arg_pop(p_ra))) return status;
	}

	return 0;  /* OK! */
	
} /* grunt_vm_pop() */


int
grunt_vm_pushb(const grunt_value_t *p_literal) {

	/* PUSHB must have a Boolean argument. */
	if (p_literal->type != gt_bool) return GRUNT_ERROR_INVALIDLITERAL;

	return grunt_stack_arg_push(p_literal);
		
} /* grunt_vm_pushb() */


int
grunt_vm_pushn(const grunt_value_t *p_literal) {

	/* PUSHN must have a number argument. */
	if (p_literal->type != gt_num) return GRUNT_ERROR_INVALIDLITERAL;
	
	return grunt_stack_arg_push(p_literal);
		
} /* grunt_vm_pushn() */


int
grunt_vm_pushs(const grunt_value_t *p_literal) {

	/* PUSHS must have a string argument. */
	if (p_literal->type != gt_str) return GRUNT_ERROR_INVALIDLITERAL;
	
	return grunt_stack_arg_push(p_literal);
		
} /* grunt_vm_pushs() */


int
grunt_vm_roll(grunt_rep_t reps) {

	/* The minimum number of reps is 2. */
	if (reps < 2) return GRUNT_ERROR_INVALIDLITERAL;

	return grunt_stack_arg_roll(reps);
	
} /* grunt_vm_roll() */
