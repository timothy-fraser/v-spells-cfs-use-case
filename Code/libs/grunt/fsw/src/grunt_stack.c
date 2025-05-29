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

/* The Grunt virtual machine has two stacks: an argument stack to
 * which we push/pop Boolean, number, and string arguments for Grunt
 * instructions, and a control stack where we push and pop return
 * address program counter values for call/return instructions.  The
 * argument stack starts at stack array index 0 and grows up.  The
 * control stack starts at array index GRUNT_STACK_SIZE - 1 and grows
 * down.
 *
 * g_argument_count is the number of arguments on the argument stack.
 * g_control_count is the number of program counter values on the
 * control stack.
 *
 *              Argument Stack          Control Stack
 * empty stack: g_arugment_count == 0   g_control_count == 0
 * push         g_argument_count++;     g_control_count++;
 * pop          g_argument_count--;     g_control_count--;
 * topmost:     g_argument_count - 1    GRUNT_STACK_SIZE - 1 - g_control_count
 * full:        g_argument_count + g_control_count >= GRUNT_STACK_SIZE
 *
 * Note that, if we want to prohibit loops, it is important to keep
 * program counter values on a separate control stack that does not
 * support dup and roll operations.
 */


/* The unit tests assume GRUNT_STACK_SIZE is an even number. */
#define GRUNT_STACK_SIZE (2*16)      /* max number of elements on stack */
static grunt_value_t g_stack[GRUNT_STACK_SIZE];       /* the two stacks */
static int g_argument_count;     /* count of elements on argument stack */
static int g_control_count;       /* count of elements on control stack */


void
grunt_stack_init(void) {
	g_argument_count = 0;
	g_control_count  = 0;
} /* grunt_stack_init() */


int
grunt_stack_arg_push(const grunt_value_t *p_arg) {

	/* No program counter values allowed on the arg stack. */
	if (!((p_arg->type == gt_bool)||(p_arg->type == gt_num)||
		(p_arg->type == gt_str))) {
		return GRUNT_ERROR_INTERPRETERBUG;
	}
	
	if ((g_argument_count + g_control_count + 1) > GRUNT_STACK_SIZE) {
		return GRUNT_ERROR_OUTOFBOUNDS;
	}
	
	memcpy(&(g_stack[g_argument_count]), p_arg, sizeof(grunt_value_t));
	g_argument_count++;
	return 0;  /* OK! */
	
} /* grunt_stack_arg_push() */


int
grunt_stack_arg_pop(grunt_value_t *p_arg) {

	if (g_argument_count == 0) return GRUNT_ERROR_OUTOFBOUNDS;

	g_argument_count--;
	memcpy(p_arg, &(g_stack[g_argument_count]), sizeof(grunt_value_t));
	return 0;  /* OK! */
	
} /* grunt_stack_arg_pop() */


/* grunt_stack_arg_dup()
 *
 * in:     n       - number of elements to duplicate
 * out:    g_stack - top n arg stack elements duplicated
 *         g_argument_count - incremented by n
 * return: value     condition
 *         -----     -------------------
 *         GRUNT_ERROR_INTERPRETERBUG - n == 0; you can't dup nothing
 *         GRUNT_ERROR_OUTOFBOUNDS - fewer than n elements on stack
 *         GRUNT_ERROR_OUTOFBOUNDS  - not enough room to dup n elements
 *         0                          - success
 *
 * Duplicates the top n elements on the arg stack.
 * Example: DUP 2  ; x y z -- x y z y z
 *
 * Callers must ensure n > 0.
 */

int
grunt_stack_arg_dup(grunt_rep_t n) {

	if (n == 0) return GRUNT_ERROR_INTERPRETERBUG;

	if (g_argument_count < n) return GRUNT_ERROR_OUTOFBOUNDS;
	
	if ((g_argument_count + g_control_count + n) > GRUNT_STACK_SIZE) {
		return GRUNT_ERROR_OUTOFBOUNDS;
	}
	
	memcpy(&(g_stack[g_argument_count]), &(g_stack[g_argument_count - n]),
	       (n * sizeof(grunt_value_t)));
	g_argument_count += n;
	return 0;  /* OK! */
	
} /* grunt_stack_arg_dup() */


/* grunt_stack_arg_roll()
 *
 * in:     n       - number of elements to  roll topward
 * out:    g_stack - top n arg stack elements rolled topward by one
 * return: value                       condition
 *         ----------                  ---------------
 *         GRUNT_ERROR_INTERPRETERBUG  n < 2; you can't roll 0 or 1 elements
 *         GRUNT_ERROR_OUTOFBOUNDS  fewer than n elements on arg stack
 *         0                           success
 *
 * Rolls the topmost n elements on the arg stack topward by one step.
 * Example: ROL 3  ; w x y z -- w z x y
 *
 * Callers must ensure n >= 2.
 */

int
grunt_stack_arg_roll(grunt_rep_t n) {

	grunt_value_t temp;  /* bounce topmost stack element through here */

	if (n < 2) return GRUNT_ERROR_INTERPRETERBUG;

	if (g_argument_count < n) return GRUNT_ERROR_OUTOFBOUNDS;

	memcpy(&temp, &(g_stack[g_argument_count - 1]), sizeof(grunt_value_t));
	memcpy(&(g_stack[g_argument_count - n + 1]),
	       &(g_stack[g_argument_count - n]),
	       ((n - 1) * sizeof(grunt_value_t)));
	memcpy(&(g_stack[g_argument_count - n]), &temp, sizeof(grunt_value_t));

	return 0;  /* OK! */
	
} /* grunt_stack_arg_roll() */


int
grunt_stack_ctl_push(const grunt_value_t *p_arg) {

	/* Only program counter values allowed on the control stack. */
	if (!(p_arg->type == gt_pc)) return GRUNT_ERROR_INTERPRETERBUG;

	if ((g_argument_count + g_control_count + 1) > GRUNT_STACK_SIZE) {
		return GRUNT_ERROR_OUTOFBOUNDS;
	}
	
	memcpy(&(g_stack[((GRUNT_STACK_SIZE - 1) - g_control_count)]), p_arg,
	       sizeof(grunt_value_t));
	g_control_count++;
	return 0;  /* OK! */
	
} /* grunt_stack_ctl_push() */


int
grunt_stack_ctl_pop(grunt_value_t *p_arg) {

	if (g_control_count == 0) return GRUNT_ERROR_OUTOFBOUNDS;

	g_control_count--;
	memcpy(p_arg, &(g_stack[((GRUNT_STACK_SIZE - 1) - g_control_count)]),
		sizeof(grunt_value_t));
	return 0;  /* OK! */
	
} /* grunt_stack_ctl_pop() */
