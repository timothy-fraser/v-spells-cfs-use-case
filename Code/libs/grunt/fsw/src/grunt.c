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

#include "grunt_version.h"
#include "vs_eventids.h"
#include "grunt_eventids.h"
#include "grunt.h"
#include "grunt_status.h"

#include "grunt_stack.h"
#include "grunt_input.h"
#include "grunt_output.h"
#include "grunt_vm_stack.h"
#include "grunt_vm_control.h"
#include "grunt_vm_logic.h"
#include "grunt_vm_arithmetic.h"
#include "grunt_vm_io.h"

static grunt_value_t g_ra;  /* register, often an accumulator */
static grunt_value_t g_rb;  /* register, often a bounce variable */

static grunt_pc_t    g_pc;  /* the program counter */


/* -------------------- local functions ---------------------------- */

static int
grunt_vm_step(const grunt_instruction_t *p_i) {

	/* Increment program counter so that the next fetch will get
	 * the next instruction in sequence unless the current
	 * instruction is a CALL, JMPIF, or RETURN.  These
	 * instructions may reset the program counter to some other
	 * target.  Note that p_i points to the current instruction;
	 * this increment (and potential subsequent reset) will impact
	 * the *next* instruction fetch.
	 */
	g_pc++;

	switch (p_i->op) {
	case GRUNT_OP_ADD:
		return grunt_vm_add_sub(&g_ra, &g_rb, true);
	case GRUNT_OP_AND:
		return grunt_vm_and_or(&g_ra, &g_rb, p_i->arg.rep, true);
	case GRUNT_OP_CALL:
		return grunt_vm_call(&g_ra, &g_pc, &(p_i->arg.lit));
	case GRUNT_OP_DUP:
		return grunt_vm_dup(p_i->arg.rep);
	case GRUNT_OP_EQ:
		return grunt_vm_eq(&g_ra, &g_rb, p_i->arg.rep);
	case GRUNT_OP_FLUSH:
		return grunt_vm_flush(&g_ra, &g_rb);
	case GRUNT_OP_GT:
		return grunt_vm_lt_gt(&g_ra, &g_rb, false);
	case GRUNT_OP_HALT:
		return grunt_vm_halt(&g_ra);
	case GRUNT_OP_INPUT:
		return grunt_vm_input(&g_ra, p_i->arg.rep);
	case GRUNT_OP_JMPIF:
		return grunt_vm_jmpif(&g_ra, &g_pc, &(p_i->arg.lit));
	case GRUNT_OP_LT:
		return grunt_vm_lt_gt(&g_ra, &g_rb, true);
	case GRUNT_OP_NOT:
		return grunt_vm_not(&g_ra);
	case GRUNT_OP_OUTPUT:
		return grunt_vm_output(&g_ra);
	case GRUNT_OP_OR:
		return grunt_vm_and_or(&g_ra, &g_rb, p_i->arg.rep, false);
	case GRUNT_OP_POP:
		return grunt_vm_pop(&g_ra, p_i->arg.rep);
	case GRUNT_OP_PUSHB:
		return grunt_vm_pushb(&(p_i->arg.lit));
	case GRUNT_OP_PUSHN:
		return grunt_vm_pushn(&(p_i->arg.lit));
	case GRUNT_OP_PUSHS:
		return grunt_vm_pushs(&(p_i->arg.lit));
	case GRUNT_OP_RETURN:
		return grunt_vm_return(&g_ra, &g_pc);
	case GRUNT_OP_REWIND:
		return grunt_vm_rewind(p_i->arg.rep);
	case GRUNT_OP_ROLL:
		return grunt_vm_roll(p_i->arg.rep);
	case GRUNT_OP_SUB:
		return grunt_vm_add_sub(&g_ra, &g_rb, false);
	}

	return GRUNT_ERROR_INVALIDOPCODE;

} /* grunt_vm_step() */


/* This routine reports runtime errors and interpreter bugs
 * encountered by the Grunt program.  Ideally, once you've debugged
 * your Grunt program, you won't get any of these.
 *
 * This routine does not report validity problems with the table the
 * Grunt program is validating - the Grunt program itself reports
 * those problems.
 */

void
grunt_vm_error(int status, grunt_pc_t pc) {

	const char *msg;

	switch (status) {
	case GRUNT_ERROR_INTERPRETERBUG:
		msg = "interpreter bug";
		break;
	case GRUNT_ERROR_INVALIDARGUMENT:
		msg = "invalid argument";
		break;
	case GRUNT_ERROR_INVALIDLITERAL:
		msg = "invalid literal";
		break;
	case GRUNT_ERROR_INVALIDOPCODE:
		msg = "invalid opcode";
		break;
	case GRUNT_ERROR_NOPROGRAM:
		msg = "no program";
		break;
	case GRUNT_ERROR_NOLOOPS:
		msg = "no loops";
		break;
	case GRUNT_ERROR_OUTOFBOUNDS:
		msg = "out of bounds";
		break;
	default:
		msg = "unknown error";
	}

	printf("DBG %u: program counter %u: %s\n", status, pc, msg);

} /* grunt_vm_error() */


void
grunt_vm_init(void) {
	g_pc = 0;  /* 0 is the index of the first instruction in Grunt. */
} /* grunt_vm_init() */


/* --------------------- exported functions -------------------- */

int32
GRUNT_Init(void) {

	grunt_vm_init();
	
	/* Report our successfull initialization. */
	OS_printf("%s initialized\n", GRUNT_VERSION_STRING);
	
	return CFE_SUCCESS;

} /* GRUNT_Init() */


int32
GRUNT_Run(const grunt_instruction_t *program, grunt_pc_t num_instructions,
	const void *p_data, grunt_rep_t data_size,
	const char *string_table[], grunt_string_t num_strings) {

	grunt_pc_t current_instruction;
	int status = 0;

	/* Initialize the VM to run the indicated Grunt program. */
	grunt_stack_init();
	grunt_input_init(p_data, data_size);
	grunt_output_init(string_table, num_strings);
	grunt_vm_init();
	
	/* This is the interpreter's main loop.  It interprets
	 * instructions until we reach a HALT or an error.
	 *
	 * Analysts seeking to reason about Grunt's termination
	 * behavior should note that Grunt's termination guarantee
	 * argument is based on its monotonically increasting program
	 * counter rather than on the bounds of this loop.
	 */
	do {
		current_instruction = g_pc;  /* save for error reporting */
		
		/* If we're about to try to execute an instruction
		 * beyond the end of the Grunt program, report an
		 * error and halt.  This condition can happen if we
		 * are passed a zero-length program or if the program
		 * counter runs off the end of the program before it
		 * hits a HALT instruction.
		 */
		if (!(g_pc < num_instructions)) {
			status = GRUNT_ERROR_NOPROGRAM;
			break;
		}
		
	} while (!(status = grunt_vm_step(&(program[g_pc])))); 

	/* If we reach here, the run loop terminated because
	 *   (A) the Grunt program reached a HALT instruction,
	 *   (B) the Grunt program had a run-time error, or
	 *   (C) our interpreter has a bug.
	 * Emit a debug message for cases B and C and return a status
	 * code indicating what happened.
	 */
	if (!((status == GRUNT_HALT_TRUE)||(status == GRUNT_HALT_FALSE)))
		grunt_vm_error(status, current_instruction);

	return status;

} /* GRUNT_Run() */

