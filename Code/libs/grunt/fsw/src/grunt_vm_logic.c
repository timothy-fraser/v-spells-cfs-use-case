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
#include "grunt_vm_logic.h"


/* grunt_vm_and_or()
 *
 * in:     n  - operate on the top n elements of the arg stack
 *         and_flag - true for logical and, false for logical or
 * out:    *p_ra    - clobbered; holds result of and/or operation
 *         *p_rb    - clobbered
 * return: value        condition
 *         ------       ---------------
 *         GRUNT_ERROR_INVALIDLITERAL  - need n to be at least 2
 *         GRUNT_ERROR_INVALIDARGUMENT - all topmost n args must be Booleans
 *         0                           - success
 *         Plus grunt_stack_arg_push/pop() error codes.
 *
 * Pops n Booleans off of the arg stack, and/or's them together, and
 * pushes the result onto the arg stack.  and_flag set causes logical
 * and, and_flag clear causes logical or.
 *
 */

int
grunt_vm_and_or(grunt_value_t *p_ra, grunt_value_t *p_rb, grunt_rep_t n,
	bool and_flag) {

	grunt_rep_t i;
	int status;

	/* The minimum number of reps is 2. */
	if (n < 2) return GRUNT_ERROR_INVALIDLITERAL;

	/* Pop the first argument into p_ra and confirm it is a Boolean. */
	if ((status = grunt_stack_arg_pop(p_ra))) return status;
	if (p_ra->type != gt_bool) return GRUNT_ERROR_INVALIDARGUMENT;

	/* Pop the subsequent arguments into p_rb, confirm they are
	 * also Boolean, and logical-and/or them into p_ra.
	 */
	for (i = 1; i < n; i++) {
		if ((status = grunt_stack_arg_pop(p_rb))) return status;
		if (p_rb->type != gt_bool) return GRUNT_ERROR_INVALIDARGUMENT;

		p_ra->val.b = (and_flag ?
			p_ra->val.b && p_rb->val.b :
			p_ra->val.b || p_rb->val.b);
	}

	/* Push the value accumulated in g_ra. */
	return grunt_stack_arg_push(p_ra);
	
} /* grunt_vm_and_or() */


/* grunt_vm_eq()
 *
 * in:     n     - operate on the top n elements of the arg stack
 * out:    *p_ra - clobbered ; will contain result of the eq operation
 *         *p_rb - clobbered
 * return: value        condition
 *         ------       ---------------
 *         GRUNT_ERROR_INVALIDLITERAL  - need n to be at least 2
 *         GRUNT_ERROR_INVALIDARGUMENT - all topmost n args must be numbers
 *         0                           - success
 *         Plus grunt_stack_arg_push/pop() error codes.
 *
 * Pops n numbers off of the arg stack and compares them.  If they are
 * all equal, it pushes true onto the arg stack.  If they are not all
 * equal, it pushes false onto the arg stack.
 */

int
grunt_vm_eq(grunt_value_t *p_ra, grunt_value_t *p_rb, grunt_rep_t n) {

	grunt_rep_t i;
	bool equal_flag = true;  /* optimistically presume all n equal */
	int status;

	/* The minimum number of reps is 2. */
	if (n < 2) return GRUNT_ERROR_INVALIDLITERAL;

	/* Pop the first argument into g_ra and confirm it is a number. */
	if ((status = grunt_stack_arg_pop(p_ra))) return status;
	if (p_ra->type != gt_num) return GRUNT_ERROR_INVALIDARGUMENT;

	/* Pop the subsequent arguments into p_rb, confirm they are
	 * also numbers, and compare them to p_ra.  Keep comparing
	 * even after we find a non-equal number; we need to pop all
	 * n of our arguments.
	 */
	for (i = 1; i < n; i++) {
		if ((status = grunt_stack_arg_pop(p_rb))) return status;
		if (p_rb->type != gt_num) return GRUNT_ERROR_INVALIDARGUMENT;

		if (p_ra->val.num != p_rb->val.num) equal_flag = false;
	}

	/* Push the result. */
	p_ra->type = gt_bool;
	p_ra->val.b = equal_flag;
	return grunt_stack_arg_push(p_ra);
	
} /* grunt_vm_eq() */


/* grunt_vm_lt_gt()
 *
 * in:     lt_flag - true for lt, false for gt
 * out:    *g_ra   - clobbered; will contain the result of the lt/gt op
 *         *g_rb   - clobbered
 * return: value        condition
 *         ------       ---------------
 *         GRUNT_ERROR_INVALIDARGUMENT - topmost 2 args must be numbers
 *         0                           - success
 *         Plus grunt_stack_arg_push/pop() error codes.
 *
 * Pops 2 numbers off of the arg stack, compares them according to
 * less-than/greater-than, and pushes the result onto the arg stack.  Note
 * that the order of the arguments on the arg stack can be confusing:
 *
 *  LT  ;  7 11 -- true
 *  GT  ;  7 11 -- false
 */

int
grunt_vm_lt_gt(grunt_value_t *p_ra, grunt_value_t *p_rb, bool lt_flag) {

	bool result;  /* true/false result of lt or gt comparison */
	int status;
	
	/* LT and GT demand that the top two elements of the arg stack be
	 * numbers.  Pop them and confirm that they are indeed numbers.
	 */
	if ((status = grunt_stack_arg_pop(p_rb))) return status;
	if (p_rb->type != gt_num) return GRUNT_ERROR_INVALIDARGUMENT;
	if ((status = grunt_stack_arg_pop(p_ra))) return status;
	if (p_ra->type != gt_num) return GRUNT_ERROR_INVALIDARGUMENT;

	/* Do the lt/gt comparison and push the result on the arg stack. */
	result = (lt_flag ? p_ra->val.num < p_rb->val.num :
		p_ra->val.num > p_rb->val.num);
	p_ra->type = gt_bool;
	p_ra->val.b = result;
	return grunt_stack_arg_push(p_ra);
	
} /* grunt_vm_lt_gt() */


/* grunt_vm_not()
 *
 * in:     nothing
 * out:    *g_ra   - clobbered; will contain the result of the not operation
 * return: value        condition
 *         ------       ---------------
 *         GRUNT_ERROR_INVALIDARGUMENT - topmost arg must be a Boolean
 *         0                           - success
 *         Plus grunt_stack_arg_push/pop() error codes.
 *
 * Pops a Boolean off the arg stack, NOTs it, an pushes the result back
 * onto the arg stack.
 */

int
grunt_vm_not(grunt_value_t *p_ra) {

	int status;
	
	/* RETURN demands that the top element of the arg stack be a
	 * Boolean.  Pop it and confirm that it is indeed a Boolean.
	 */
	if ((status = grunt_stack_arg_pop(p_ra))) return status;
	if (p_ra->type != gt_bool) return GRUNT_ERROR_INVALIDARGUMENT;

	/* NOT the boolean value and push it back on the arg stack. */
	p_ra->val.b = !p_ra->val.b;
	return grunt_stack_arg_push(p_ra);
	
} /* grunt_vm_not() */
	

