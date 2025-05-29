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
#include "grunt_vm_control.h"


int
grunt_vm_call(grunt_value_t *p_ra, grunt_pc_t *p_pc,
	const grunt_value_t *p_literal) {

	int status;

	/* CALL must have a pc target literal argument.  Anything else
	 * is an error.
	 */
	if (p_literal->type != gt_pc) return GRUNT_ERROR_INVALIDLITERAL;

	/* CALLs must be forward in the program.  This restriction
	 * rules out loops.
	 */
	if (p_literal->val.pc < *p_pc) return GRUNT_ERROR_NOLOOPS;
	
	/* Push the current program counter onto the control stack. */
	p_ra->type   = gt_pc;
	p_ra->val.pc = *p_pc;
	if ((status = grunt_stack_ctl_push(p_ra))) return status;

	/* Set the program counter to the call target. */
	*p_pc = p_literal->val.pc;
	
	return 0;  /* OK! */
	
} /* grunt_vm_call() */


int
grunt_vm_halt(grunt_value_t *p_ra) {

	int status;

	/* Halt expects the topmost element on the arg stack to be a
	 * Boolean.  Pop a value from the top of the stack, confirm it
	 * is a Booelan, and return the proper status code.
	 */
	if ((status = grunt_stack_arg_pop(p_ra))) return status;

	if (p_ra->type != gt_bool) return GRUNT_ERROR_INVALIDARGUMENT;

	return (p_ra->val.b ? GRUNT_HALT_TRUE : GRUNT_HALT_FALSE);
	
} /* grunt_vm_halt() */


int
grunt_vm_jmpif(grunt_value_t *p_ra, grunt_pc_t *p_pc,
	const grunt_value_t *p_literal) {

	int status;

	/* JMPIF must have a program counter adjustment literal
	 * argument.  Its value must be at least 2.
	 */
	if (p_literal->type != gt_pc) return GRUNT_ERROR_INVALIDLITERAL;
	if (p_literal->val.pc < 2)    return GRUNT_ERROR_INVALIDLITERAL;
	
	/* JMPIF expects a Boolean value at the top of the arg stack. */
	if ((status = grunt_stack_arg_pop(p_ra))) return status;
	if (p_ra->type != gt_bool) return GRUNT_ERROR_INVALIDARGUMENT;

	/* If the boolean is false, we don't jump. */
	if (!(p_ra->val.b)) return 0;  /* OK, no jump! */

	/* Boolean is true, jump.  Jumps are *relative* to the current
	 * program counter.
	 */
	if (p_literal->val.pc > (UINT16_MAX - *p_pc))
		return GRUNT_ERROR_NOPROGRAM;
	/* Undo instruction dispatch's earlier increment, adjust pc. */
	*p_pc += (p_literal->val.pc - 1);
	
	return 0;  /* OK, jump! */
	
} /* grunt_vm_jmpif() */


int
grunt_vm_return(grunt_value_t *p_ra, grunt_pc_t *p_pc) {

	int status;
	
	if ((status = grunt_stack_ctl_pop(p_ra))) return status;
	*p_pc = p_ra->val.pc;        /* reset pc to recalled return target */
	return 0;  /* OK! */
	
} /* grunt_vm_return() */


