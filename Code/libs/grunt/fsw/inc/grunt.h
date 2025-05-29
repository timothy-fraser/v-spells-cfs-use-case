#ifndef _GRUNT_H_
#define _GRUNT_H_

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

/* These are the kinds of values Grunt programs can push/pop to/from
 * the stack.
 */
typedef bool   grunt_boolean_t;
typedef uint32 grunt_number_t;
typedef uint16 grunt_string_t;
typedef uint16 grunt_pc_t;

#define GRUNT_NUM_MAX UINT32_MAX
#define GRUNT_PC_MAX  UINT16_MAX

typedef enum {
	gt_bool,
	gt_num,
	gt_str,
	gt_pc,
} grunt_value_type_t;

typedef struct {
	grunt_value_type_t  type;
	union {
		grunt_boolean_t  b;
		grunt_number_t num;
		grunt_string_t str;
		grunt_pc_t      pc;
	} val;
} grunt_value_t;

/* Some Grunt instructions require a repitition count argument.
 * Unlike gt_num literal arguments, the instructions don't push these
 * repetition counts onto the stack.  Instead, the repetition counts
 * tell the Grunt interpreter how many times to do whatver the
 * instruction wants to do.  Example: "POP 3" pops the top 3 entries
 * off of the stack.
 */
typedef uint16 grunt_rep_t;        /* repetition count */
#define GRUNT_REP_MAX UINT16_MAX   /* maximum possible repetition count */

/* Grunt opcodes */
#define GRUNT_OP_ADD     0x01   /* ADD    no literal   */
#define GRUNT_OP_AND     0x02   /* AND    repititions  */
#define GRUNT_OP_CALL    0x03   /* CALL   literal PC   */
#define GRUNT_OP_DUP     0x04   /* DUP    repititions  */
#define GRUNT_OP_EQ      0x05   /* EQ     repititions  */
#define GRUNT_OP_FLUSH   0x06   /* FLUSH  no literal   */
#define GRUNT_OP_GT      0x07   /* GT     no literal   */
#define GRUNT_OP_HALT    0x08   /* HALT   no literal   */
#define GRUNT_OP_JMPIF   0x09   /* JMPIF  literal PC   */
#define GRUNT_OP_LT      0x0A   /* LT     no literal   */
#define GRUNT_OP_NOT     0x0B   /* NOT    no literal   */
#define GRUNT_OP_OR      0x0C   /* OR     repititions  */
#define GRUNT_OP_OUTPUT  0x0D   /* OUTPUT no literal   */
#define GRUNT_OP_POP     0x0E   /* POP    repititions  */
#define GRUNT_OP_PUSHB   0x0F   /* PUSH   literal Bool */
#define GRUNT_OP_PUSHN   0x10   /* PUSH   literal Num  */
#define GRUNT_OP_PUSHS   0x11   /* PUSH   literal Str  */
#define GRUNT_OP_INPUT   0x12   /* INPUT  repititions  */
#define GRUNT_OP_RETURN  0x13   /* RETURN no literal   */
#define GRUNT_OP_REWIND  0x14   /* REWIND repititions  */
#define GRUNT_OP_ROLL    0x15   /* ROLL   repititions  */
#define GRUNT_OP_SUB     0x16   /* SUB    no literal   */

typedef unsigned short grunt_opcode_t;   /* instruction opcodes */

/* This type represents Grunt instructions and their arguments. */
typedef struct {
	grunt_opcode_t op;               /* all instructions have an opcode */
	union {
		grunt_rep_t    rep;      /* and either a repetition count, */
		grunt_value_t  lit;      /* or a literal value. */
	} arg;
} grunt_instruction_t;

/* Some convenience macros for writing Grunt programs as constant arrays. */
#define ADD       { .op = GRUNT_OP_ADD }
#define AND(r)    { .op = GRUNT_OP_AND, .arg.rep = (r) }
#define CALL(sub) { .op = GRUNT_OP_CALL, \
		    .arg.lit = { .type = gt_pc, .val.pc = (sub) }}
#define DUP(r)    { .op = GRUNT_OP_DUP, .arg.rep = (r) }
#define EQ(r)     { .op = GRUNT_OP_EQ, .arg.rep = (r) }
#define FLUSH     { .op = GRUNT_OP_FLUSH }
#define GT        { .op = GRUNT_OP_GT }
#define HALT      { .op = GRUNT_OP_HALT }
#define INPUT(r)  { .op = GRUNT_OP_INPUT, .arg.rep = (r) }
#define JMPIF(l)  { .op = GRUNT_OP_JMPIF, \
		    .arg.lit = { .type = gt_pc, .val.pc = (l) }}
#define LT        { .op = GRUNT_OP_LT }
#define NOT       { .op = GRUNT_OP_NOT }
#define OR(r)     { .op = GRUNT_OP_OR, .arg.rep = (r) }
#define OUTPUT    { .op = GRUNT_OP_OUTPUT }
#define POP(r)    { .op = GRUNT_OP_POP, .arg.rep = (r) }
#define PUSHB(tf) { .op = GRUNT_OP_PUSHB, \
		    .arg.lit = { .type = gt_bool, .val.b = (tf) }}
#define PUSHN(n)  { .op = GRUNT_OP_PUSHN, \
		    .arg.lit = { .type = gt_num,  .val.num = (n) }}
#define PUSHS(s)  { .op = GRUNT_OP_PUSHS, \
		    .arg.lit = { .type = gt_str,  .val.str = (s) }}
#define RETURN    { .op = GRUNT_OP_RETURN }
#define REWIND(r) { .op = GRUNT_OP_REWIND, .arg.rep = (r) } 
#define ROLL(r)   { .op = GRUNT_OP_ROLL, .arg.rep = (r) }
#define SUB       { .op = GRUNT_OP_SUB }

int32 GRUNT_Init(void);

int32 GRUNT_Run(const grunt_instruction_t *, grunt_pc_t,
		const void *, grunt_rep_t,
		const char **, grunt_string_t);

#endif
