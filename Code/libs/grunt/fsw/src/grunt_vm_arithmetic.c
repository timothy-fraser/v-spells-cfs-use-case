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
#include "grunt_vm_arithmetic.h"


/*
 * ADD:   17 13 -- 30
 * ADD:  MAX  1 -- GRUNT_ERROR_OUTOFBOUNDS
 * SUB:   17 13 -- 4
 * SUB:    0  1 -- GRUNT_ERROR_OUTOFBOUDNS  
 */

int
grunt_vm_add_sub(grunt_value_t *p_ra, grunt_value_t *p_rb, bool add_flag) {

	int status;

	/* Add & sub demand that the top two elements of the arg stack be
	 * numbers.  Pop them and confirm that they are indeed numbers.
	 */
	if ((status = grunt_stack_arg_pop(p_rb))) return status;
	if (p_rb->type != gt_num) return GRUNT_ERROR_INVALIDARGUMENT;
	if ((status = grunt_stack_arg_pop(p_ra))) return status;
	if (p_ra->type != gt_num) return GRUNT_ERROR_INVALIDARGUMENT;

	/* Do the add/sub.  Report error on over/underflow. */
	if (add_flag) {

		/* Handle add */
		if (p_rb->val.num > (GRUNT_NUM_MAX - p_ra->val.num))
			return GRUNT_ERROR_OUTOFBOUNDS;  /* overflow */
		p_ra->val.num += p_rb->val.num;
		
	} else {

		/* Handle sub */
		if (p_ra->val.num < p_rb->val.num)
			return GRUNT_ERROR_OUTOFBOUNDS;  /* underflow */
		p_ra->val.num -= p_rb->val.num;
	       
	}
	
	/* Push the result on the arg stack. */
	return grunt_stack_arg_push(p_ra);
	
} /* grunt_vm_add_sub() */

