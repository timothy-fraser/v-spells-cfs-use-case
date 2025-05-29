#ifndef _VSVF_H_
#define _VSVF_H_

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

/* This file contains the Grunt implementation of the V-SPELLS Charlie
 * app table validation function.  The Grunt program has three parts:
 *
 *  1. the vsvf_strings[] table of constant strings the program uses
 *     in its output messages,
 *  2. the vsvf_program[] array of the program's Grunt instructions, and
 *  3. a whole bunch of preprocessor macros that compute the addresses
 *     of jump and call targets within that program.
 *
 * This file needs the #3 macros because I authored the program
 * manually at the level of Grunt virtual machine instructions rather
 * than first authoring a Grunt compiler or assembler and using it to
 * generate the contents of this file automatically.
 */

static const char *vsvf_strings[] = {
	/* Strings for final info message */
	"Table image entries: ",  /* 0 */
	" valid, ",
	" invalid, ",
	" unused",

	/* Strings for error messages */
	"Table entry ",           /* 4 */
	" parm ",
	" not zeroed",
	" invalid Parm ID",
	" padding not zeroed",
	" invalid low bound",
	" invalid high bound",
	" invalid bound order",
	" follows an unused entry",
	" redefines earlier entry",

	/* Strings for pretty-printing Parm IDs */
	"Unused",           /* 14 */
	"Ape",
	"Bat",
	"Cat",
	"Dog",
	"North",
	"South",
	"East",
	"West",
	"Unknown",
};
#define VSVF_NUM_STRINGS 24


#define MAIN                00
#define MAIN_LOC            (33)
#define VALIDATE_ENTRY      (MAIN + MAIN_LOC)
#define VALIDATE_ENTRY_LOC  52
#define IS_UNUSED           (VALIDATE_ENTRY + VALIDATE_ENTRY_LOC)
#define IS_UNUSED_LOC       3
#define IS_ANIMAL           (IS_UNUSED + IS_UNUSED_LOC)
#define IS_ANIMAL_LOC       16
#define IS_DIRECTION        (IS_ANIMAL + IS_ANIMAL_LOC)
#define IS_DIRECTION_LOC    16
#define VALIDATE_UNUSED     (IS_DIRECTION + IS_DIRECTION_LOC)
#define VALIDATE_UNUSED_LOC 18
#define VALIDATE_INUSE      (VALIDATE_UNUSED + VALIDATE_UNUSED_LOC)
#define VALIDATE_INUSE_LOC  26
#define VALIDATE_PAD        (VALIDATE_INUSE + VALIDATE_INUSE_LOC)
#define VALIDATE_PAD_LOC    17
#define VALIDATE_BOUNDS     (VALIDATE_PAD + VALIDATE_PAD_LOC)
#define VALIDATE_BOUNDS_LOC 26
#define VALIDATE_RANGE      (VALIDATE_BOUNDS + VALIDATE_BOUNDS_LOC)
#define VALIDATE_RANGE_LOC  15
#define VALIDATE_ORDER      (VALIDATE_RANGE + VALIDATE_RANGE_LOC)
#define VALIDATE_ORDER_LOC  13
#define VALIDATE_EXTRA      (VALIDATE_ORDER + VALIDATE_ORDER_LOC)
#define VALIDATE_EXTRA_LOC  15
#define VALIDATE_REDEF      (VALIDATE_EXTRA + VALIDATE_EXTRA_LOC)
#define VALIDATE_REDEF_LOC  26
#define HANDLE_PARMERR      (VALIDATE_REDEF + VALIDATE_REDEF_LOC)
#define HANDLE_PARMERR_LOC  10
#define INC_UNUSED          (HANDLE_PARMERR + HANDLE_PARMERR_LOC)
#define INC_UNUSED_LOC      6
#define INC_VALID           (INC_UNUSED + INC_UNUSED_LOC)
#define INC_VALID_LOC       5
#define COMPUTE_INVALID     (INC_VALID + INC_VALID_LOC)
#define COMPUTE_INVALID_LOC 7
#define COMPUTE_RESULT      (COMPUTE_INVALID + COMPUTE_INVALID_LOC)
#define COMPUTE_RESULT_LOC  7
#define EMIT_INFO           (COMPUTE_RESULT + COMPUTE_RESULT_LOC)
#define EMIT_INFO_LOC       15
#define EMIT_ERROR_PARMERR  (EMIT_INFO + EMIT_INFO_LOC)
#define EMIT_ERROR_PARMERR_LOC 9
#define EMIT_ERROR          (EMIT_ERROR_PARMERR + EMIT_ERROR_PARMERR_LOC)
#define EMIT_ERROR_LOC      11
#define PARM_TO_STR         (EMIT_ERROR + EMIT_ERROR_LOC)
#define PARM_TO_STR_LOC     75

#define VSVF_NUM_INSTRUCTIONS (PARM_TO_STR + PARM_TO_STR_LOC)


static const grunt_instruction_t vsvf_program[] = {

	/* MAIN:
	 * -- valid?
	 *
	 * This is the entry point of the program.  It calls the
	 * VALIDATE_ENTRY subroutine to perform a validity check on
	 * each of the four entires.  The VALIDATE_ENTRY subroutine
	 * takes six parms:
	 *
	 * Saved-parmid-1, -2, and -3, or more succinctly, s1 s2 s3:
	 *   These three parms tell VALIDATE_ENTRY which Parm IDs
	 *   we've seen in the first three entries so that it can
	 *   perform its duplicate Parm ID check.  This main routine
	 *   is reponsible for setting these to VS_PARM_UNUSED initial
	 *   values and saving the actual Parm IDs seen by
	 *   VALIDATE_ENTRY as it returns them.
         *
         * Unused, Valid, or more succinctly u v: These two parms
         *   count how many valid unused and valid in-use entries
         *   we've seen so far.  VALIDATE_ENTRY is responsible for
         *   taking old values in and returning updated values based
         *   on the result of its validity check.
         *
	 * Entry, or more succinctly e: This is simply the entry
	 *   number 1 through 4, which VALIDATE_ENTRY uses in its
	 *   error messages.
	 */
	
	/* Validate entry #1. */
	PUSHN(0),                   /* -- unused */
	PUSHN(0),                   /* -- u valid */
	PUSHN(VS_PARM_UNUSED),      /* -- u v saved-parmid-1 */
	PUSHN(VS_PARM_UNUSED),      /* -- u v s1 saved-parmid-2 */
	PUSHN(VS_PARM_UNUSED),      /* -- u v s1 s2 saved-parmid-3 */
	PUSHN(1),                   /* -- u v s1 s2 s3 entry */
	CALL(VALIDATE_ENTRY),       /* -- u v s1 */

	/* Save copies of saved-parmid-1 for entry #3 and #4 checks. */
	DUP(1),                     /* -- u v s1 s1 */
	ROLL(4),                    /* -- s1 u v s1 */
	DUP(1),                     /* -- s1 u v s1 s1 */
	ROLL(4),                    /* -- s1 s1 u v s1 */

	/* Validate entry #2. */
	PUSHN(VS_PARM_UNUSED),      /* -- s1 s1 u v s1 s2 */
	PUSHN(VS_PARM_UNUSED),      /* -- s1 s1 u v s1 s2 s3 */
	PUSHN(2),                   /* -- s1 s1 u v s1 s2 s3 entry */
	CALL(VALIDATE_ENTRY),       /* -- s1 s1 u v s2 */

	/* Save copy of saved-parmid-2 for entry #4 check. */
	DUP(1),                     /* -- s1 s1 u v s2 s2 */
	ROLL(5),                    /* -- s1 s2 s1 u v s2 */

	/* Validate entry #3. */
	ROLL(3),                    /* -- s1 s2 s1 s2 u v */
	ROLL(4),                    /* -- s1 s2 v s1 s2 u */
	ROLL(4),                    /* -- s1 s2 u v s1 s2 */
	PUSHN(VS_PARM_UNUSED),      /* -- s1 s2 u v s1 s2 s3 */
	PUSHN(3),                   /* -- s1 s2 u v s1 s2 s3 entry */
	CALL(VALIDATE_ENTRY),       /* -- s1 s2 u v s3 */

	/* Validate entry #4. */
	ROLL(3),                    /* -- s1 s2 s3 u v */
	ROLL(5),                    /* -- v s1 s2 s3 u */
	ROLL(5),                    /* -- u v s1 s2 s3 */
	PUSHN(4),                   /* -- u v s1 s2 s3 entry */
	CALL(VALIDATE_ENTRY),       /* -- u v s4 */

	/* Compute invalid entry count and final valid? result for the
	 * table as a whole.
	 */
	POP(1),                     /* -- u v */
	CALL(COMPUTE_INVALID),      /* -- u i v */
	CALL(COMPUTE_RESULT),       /* -- valid? u i v */
	
	/* Emit valid-invalid-unused info message. */
	CALL(EMIT_INFO),            /* -- valid? */

	/* Return valid? result. */
	HALT,


	/* VALIDATE_ENTRY:
	 * old-unused old-valid saved-parmid-1 saved-parmid-2 saved-parmid-3
	 * entry -- new-unused new-valid parmid-from-entry
	 *
	 * This routine has three cases:
	 * (1) For VS_PARM_UNUSED entries, it calls the VALIDATE_UNUSED
	 *     subroutine and expects that subroutine to return an updated
	 *     unused count.
	 * (2) For other valid VS_PARM values, it calls the VALIDATE_INUSE
	 *     subroutine and expects that subroutine to return an unpdated
	 *     valid count.
	 * (3) For any other Parm ID values (that is, for invalid
	 *     values), it reports an error and leaves the unused and
	 *     valid counts unchanged.
	 */

	/* Read entry Parm ID.  Save a copy to return to caller. */
	INPUT(1),                     /* -- u v s1 s2 s3 e parmid */
	DUP(1),                       /* -- u v s1 s2 s3 e p p */
	ROLL(6),                      /* -- u v p s1 s2 s3 e p */

	/* Is this an unused entry? */
	DUP(1),                       /* -- u v p s1 s2 s3 e p p */
	CALL(IS_UNUSED),              /* -- u v p s1 s2 s3 e p unused? */
	NOT,                          /* -- u v p s1 s2 s3 e p not-unused? */
	JMPIF(9),                     /* -- u v p s1 s2 s3 e p */

	/* Validate unused entry. */
	ROLL(5),                      /* -- u v p p s1 s2 s3 e */
	ROLL(5),                      /* -- u v p e p s1 s2 s3 */
	POP(3),                       /* -- u v p e p */
	CALL(VALIDATE_UNUSED),        /* -- u v p valid? */
	JMPIF(2),                     /* -- u v p */
	RETURN,
	CALL(INC_UNUSED),             /* -- new-u v p */
	RETURN,

	/* Set up stack for validating in-use entries.  This is a lot
	 * of rolling to pass a copy of the unused entry count, which
	 * the VALIDATE_INUSE subroutine uses to control its reporting
	 * of "in-use follows unused entry" errors.
	 */
	ROLL(8),                      /* -- p u v p s1 s2 s3 e */
	ROLL(8),                      /* -- e p u v p s1 s2 s3 */
	ROLL(8),                      /* -- s3 e p u v p s1 s2 */
	ROLL(8),                      /* -- s2 s3 e p u v p s1 */
	ROLL(8),                      /* -- s1 s2 s3 e p u v p */
	ROLL(8),                      /* -- p s1 s2 s3 e p u v */
	ROLL(8),                      /* -- v p s1 s2 s3 e p u */
	DUP(1),                       /* -- v p s1 s2 s3 e p u u */
	ROLL(9),                      /* -- u v p s1 s2 s3 e p u */
	ROLL(3),                      /* -- u v p s1 s2 s3 u e p */
	
	/* Is this an animal entry? */
	DUP(1),                       /* -- u v p s1 s2 s3 u e p p */
	CALL(IS_ANIMAL),              /* -- u v p s1 s2 s3 u e p animal? */
	NOT,                          /* -- u v p s1 s2 s3 u e p not-animal? */
	JMPIF(8),                     /* -- u v p s1 s2 s3 u e p */

	/* Validate animal entry. */
	PUSHN(VS_PARM_ANIMAL_MAX),    /* -- u v p s1 s2 s3 u e p max */
	PUSHN(VS_PARM_ANIMAL_MIN),    /* -- u v p s1 s2 s3 u e p max min */
	CALL(VALIDATE_INUSE),         /* -- u v p valid? */
	JMPIF(2),                     /* -- u v p */
	RETURN,
	CALL(INC_VALID),              /* -- u new-v p */
	RETURN,

	/* Is this a direction entry? */
	DUP(1),                       /* -- u v p s1 s2 s3 u e p p */
	CALL(IS_DIRECTION),           /* -- u v p s1 s2 s3 u e p direction? */
	NOT,                          /* -- u v p s1 s2 s3 u e p not-dir? */
	JMPIF(8),                     /* -- u v p s1 s2 s3 u e p */

	/* Validate direction entry. */
	PUSHN(VS_PARM_DIRECTION_MAX), /* -- u v p s1 s2 s3 u e p max */
	PUSHN(VS_PARM_DIRECTION_MIN), /* -- u v p s1 s2 s3 u e p max min */
	CALL(VALIDATE_INUSE),         /* -- u v p valid? */
	JMPIF(2),                     /* -- u v p */
	RETURN,
	CALL(INC_VALID),              /* -- u new-v p */
	RETURN,

	/* If we reach here, we have a bad parm ID. */
	POP(1),                       /* -- u v p s1 s2 s3 u e */
	ROLL(5),                      /* -- u v p e s1 s2 s3 u */
	POP(4),                       /* -- u v p e */
	CALL(HANDLE_PARMERR),         /* -- u v p */
	RETURN,

	
	/* IS_UNUSED:
	 * parmid -- unused?
	 */
	PUSHN(VS_PARM_UNUSED),   /* -- parmid U */
	EQ(2),                   /* -- unused? */
	RETURN,

	
	/* IS_ANIMAL:
	 * parmid -- animal?
	 */
	DUP(1),                 /* -- parmid parmid */
	PUSHN(VS_PARM_APE),     /* -- parmid parmid A */
	EQ(2),                  /* -- parmid A? */
	ROLL(2),                /* -- A? parmid */
	DUP(1),                 /* -- A? parmid parmid */
	PUSHN(VS_PARM_BAT),     /* -- A? parmid parmid B */
	EQ(2),                  /* -- A? parmid B? */
	ROLL(2),                /* -- A? B? parmid */
	DUP(1),                 /* -- A? B? parmid parmid */
	PUSHN(VS_PARM_CAT),     /* -- A? B? parmid parmid C */
	EQ(2),                  /* -- A? B? parmid C? */
	ROLL(2),                /* -- A? B? C? parmid */
	PUSHN(VS_PARM_DOG),     /* -- A? B? C? parmid D */
	EQ(2),                  /* -- A? B? C? D? */
	OR(4),                  /* -- animal? */
	RETURN,

	
	/* IS_DIRECTION:
	 * parmid -- direction?
	 */
	DUP(1),                 /* -- parmid parmid */
	PUSHN(VS_PARM_NORTH),   /* -- parmid parmid N */
	EQ(2),                  /* -- parmid N? */
	ROLL(2),                /* -- N? parmid */
	DUP(1),                 /* -- N? parmid parmid */
	PUSHN(VS_PARM_SOUTH),   /* -- N? parmid parmid S */
	EQ(2),                  /* -- N? parmid S? */
	ROLL(2),                /* -- N? S? parmid */
	DUP(1),                 /* -- N? S? parmid parmid */
	PUSHN(VS_PARM_EAST),    /* -- N? S? parmid parmid E */
	EQ(2),                  /* -- N? S? parmid E? */
	ROLL(2),                /* -- N? S? E? parmid */
	PUSHN(VS_PARM_WEST),    /* -- N? S? E? parmid W */
	EQ(2),                  /* -- N? S? E? W? */
	OR(4),                  /* -- direction? */
	RETURN,

	
	/* VALIDATE_UNUSED:
	 * entry parmid -- valid?
	 */

	/* Read all fields and see if they are all zeroed. */
	INPUT(1),                /* -- e p pad0 */
	INPUT(2),                /* -- e p pad0 pad12 */
	INPUT(4),                /* -- e p pad0 pad12 lbnd */
	INPUT(4),                /* -- e p pad0 pad12 lbnd hbnd */
	PUSHN(0),                /* -- e p pad0 pad12 lbnd hbnd 0 */
	EQ(5),                   /* -- e p zeroed? */
	JMPIF(9),                /* -- e p */

	/* Not all zeroed.  Emit not-zeroed error message. */
	ROLL(2),                    /* -- p e */
	PUSHN(VS_TBL_ZERO_ERR_EID), /* -- p e eid */
	ROLL(3),                    /* -- eid p e */
	PUSHS(6),                   /* -- eid p e msg */
	ROLL(3),                    /* -- eid msg p e */
	CALL(EMIT_ERROR),           /* -- */
	PUSHB(false),               /* -- valid? */
	RETURN,

	/* All zeroed.  Proper unused entry. */
	POP(2),                     /* -- */
	PUSHB(true),                /* -- valid? */
	RETURN,


	/* VALIDATE_INUSE:
	 * s1 s2 s3 u e p max min -- valid?
	 */

	/* Validate padding. */
	ROLL(8),            /* -- min s1 s2 s3 u e p max */
	ROLL(8),            /* -- max min s1 s2 s3 u e p */
	DUP(2),             /* -- max min s1 s2 s3 u e p e p */
	CALL(VALIDATE_PAD), /* -- max min s1 s2 s3 u e p pad? */
	ROLL(9),            /* -- pad? max min s1 s2 s3 u e p */

	/* Roll the max and min parms to the top of the stack where we
	 * need them for the range and order checks.
	 */
	DUP(2),             /* pad? max min s1 s2 s3 u e p e p */
	ROLL(10),           /* pad? p max min s1 s2 s3 u e p e */
	ROLL(10),           /* pad? e p max min s1 s2 s3 u e p */
	ROLL(10),           /* pad? p e p max min s1 s2 s3 u e */
	ROLL(10),           /* pad? e p e p max min s1 s2 s3 u */
	ROLL(10),           /* pad? u e p e p max min s1 s2 s3 */
	ROLL(10),           /* pad? s3 u e p e p max min s1 s2 */
	ROLL(10),           /* pad? s2 s3 u e p e p max min s1 */
	ROLL(10),           /* pad? s1 s2 s3 u e p e p max min */

	/* Perform bound range and order checks. */
	CALL(VALIDATE_BOUNDS),  /* pad? s1 s2 s3 u e p bounds? */
	ROLL(7),                /* pad? bounds? s1 s2 s3 u e p */
	
	/* Run extra in-use entry after unused entry check. */
	DUP(2),                  /* pad? bounds? s1 s2 s3 u e p e p */
	ROLL(5),                 /* pad? bounds? s1 s2 s3 p u e p e */
	ROLL(5),                 /* pad? bounds? s1 s2 s3 e p u e p */
	ROLL(3),                 /* pad? bounds? s1 s2 s3 e p p u e */
	ROLL(3),                 /* pad? bounds? s1 s2 s3 e p e p u */
	CALL(VALIDATE_EXTRA),    /* pad? bounds? s1 s2 s3 e p extra? */
	ROLL(7),                 /* pad? bounds? extra? s1 s2 s3 e p */

	/* Run redefined parm check. */
	CALL(VALIDATE_REDEF),    /* pad? bounds? extra? redef? */

	/* Return valid if and only if all subroutines indicated valid. */
	AND(4),                  /* valid? */
	RETURN,

	
	/* VALIDATE_PAD:
	 * entry parmid -- pad-valid?
	 */
	INPUT(1),                   /* -- e p pad0 */
	INPUT(2),                   /* -- e p pad0 pad12 */
	PUSHN(0),                   /* -- e p pad0 pad12 0 */
	EQ(3),                      /* -- e p zeroed? */
	NOT,                        /* -- e p not-zeroed? */
	JMPIF(4),                   /* -- e p */

	POP(2),                     /* -- */
	PUSHB(true),                /* pad-valid? */
	RETURN,
	
	ROLL(2),                    /* -- p e */
	PUSHN(VS_TBL_PAD_ERR_EID),  /* -- p e eid */
	ROLL(3),                    /* -- eid p e */
	PUSHS(8),                   /* -- eid p e msg  */
	ROLL(3),                    /* -- eid msg p e */ 
	CALL(EMIT_ERROR),           /* -- */
	PUSHB(false),               /* -- pad-valid? */
	RETURN,


	/* VALIDATE_BOUNDS:
	 * e p max min -- bounds-valid?
	 *
	 * This subroutine makes several checks:
	 *   (1) lbnd is in the proper range,
	 *   (2) hbnd is in the proper range,
	 *   (3) lbnd <= hbnd.
	 */
	
	/* Read lbnd.
	 * Save copy of lbnd for later order check.
	 */
	DUP(4),                  /* -- e p max min e p max min */
	INPUT(4),                /* -- e p max min e p max min l */
	DUP(1),                  /* -- e p max min e p max min l l */
	ROLL(10),                /* -- l e p max min e p max min l */

	/* Confirm lbnd is in proper range.
	 * Save result of lbnd range check.
	 */
	PUSHN(VS_TBL_LBND_ERR_EID), /* -- l e p max min e p max min l eid */
	ROLL(6),                    /* -- l e p max min eid e p max min l */
	PUSHS(9),                /* -- l e p max min eid e p max min l msg */
	ROLL(6),                 /* -- l e p max min eid msg e p max min l */
	CALL(VALIDATE_RANGE),    /* -- l e p max min l? */
	ROLL(6),                 /* -- l? l e p max min */
	
	/* Read hbnd.
	 * Save copy of hbnd for later order check.
	 */
	DUP(4),                  /* -- l? l e p max min e p max min */
	INPUT(4),                /* -- l? l e p max min e p max min h */
	DUP(1),                  /* -- l? l e p max min e p max min h h */
	ROLL(11),                /* -- l? h l e p max min e p max min h */
	
	/* Confirm hbnd is in proper range.
	 * Save result of hbnd range check.
	 */
	PUSHN(VS_TBL_HBND_ERR_EID),
	                         /* -- l? h l e p max min e p max min h eid */
	ROLL(6),                 /* -- l? h l e p max min eid e p max min h */
	PUSHS(10),           /* -- l? h l e p max min eid e p max min h msg */
	ROLL(6),             /* -- l? h l e p max min eid msg e p max min h */
	CALL(VALIDATE_RANGE),    /* -- l? h l e p max min h? */
	ROLL(8),                 /* -- h? l? h l e p max min */

	/* Confirm lbnd <= hbnd. */
	POP(2),                  /* -- h? l? h l e p */
	ROLL(4),                 /* -- h? l? p h l e */
	ROLL(4),                 /* -- h? l? e p h l  */
	CALL(VALIDATE_ORDER),    /* -- h? l? o? */

	/* Combine results of individual checks and return. */
	AND(3),                  /* -- valid? */
	RETURN,


	/* VALIDATE_RANGE:
	 * eid error-message entry parmid max min bound -- bound-valid?
	 */
	DUP(1),                  /* -- eid msg e p max min b b */
	ROLL(4),                 /* -- eid msg e p b max min b */
	ROLL(2),                 /* -- eid msg e p b max b min */
	LT,                      /* -- eid msg e p b max lt? */
	ROLL(3),                 /* -- eid msg e p lt? b max */
	GT,                      /* -- eid msg e p lt? gt? */
	OR(2),                   /* -- eid msg e p invalid? */
	JMPIF(4),                /* -- eid msg e p */

	/* valid */
	POP(4),                  /* -- */
	PUSHB(true),             /* valid? */
	RETURN,

	/* invalid */
	ROLL(2),                    /* -- eid msg p e */
	CALL(EMIT_ERROR),           /* -- */
	PUSHB(false),               /* -- valid? */
	RETURN,

	
	/* VAILIDATE_ORDER:
	 * entry parmid hbnd lbnd -- order-valid?
	 */
	LT,                      /* -- e p not-valid? */
	JMPIF(4),                /* -- e p */

	/* valid */
	POP(2),                  /* -- */
	PUSHB(true),             /* -- valid? */
	RETURN,

	/* invalid */
	ROLL(2),                     /* -- p e */
	PUSHS(11),                   /* -- p e msg */
	ROLL(3),                     /* -- msg p e */
	PUSHN(VS_TBL_ORDER_ERR_EID), /* -- msg p e eid */
	ROLL(4),                     /* -- eid msg p e */
	CALL(EMIT_ERROR),            /* -- */
	PUSHB(false),                /* -- valid? */
	RETURN,


	/* VALIDATE_EXTRA:
	 * entry parmid unused -- valid?
	 *
	 * Any in-use entry that follows a proper unused entry is
	 * invalid.  Note that only *proper* unused entries count -
	 * not entries that merely begin with VS_PARM_UNUSED but have
	 * validity problems.  Use the count of unused entries seen so
	 * far to make this check.
	 */
	PUSHN(0),                /* -- e p unused 0 */
	EQ(2),                   /* -- e p valid? */
	NOT,                     /* -- e p not-valid? */
	JMPIF(4),                /* -- e p */

	/* valid */
	POP(2),                  /* -- */
	PUSHB(true),             /* -- valid? */
	RETURN,

	/* invalid */
	ROLL(2),                     /* -- p e */
	PUSHS(12),                   /* -- p e msg */
	ROLL(3),                     /* -- msg p e */
	PUSHN(VS_TBL_EXTRA_ERR_EID), /* -- msg p e eid */
	ROLL(4),                     /* -- eid msg p e */
	CALL(EMIT_ERROR),            /* -- */
	PUSHB(false),                /* -- valid? */
	RETURN,

	
	/* VALIDATE_REDEF:
	 *
	 * saved-parmid-1 saved-parmid-2 saved-parmid-3 entry parmid --
	 * redef-valid?
	 *
	 * Multiple entries can have Parm ID VS_PARM_UNUSED.  For the
	 * other valid Parm IDs, only the first entry that uses a
	 * given Parm ID is valid.  Subsequent entries that reuse that
	 * Parm ID are invalid.
	 */
	DUP(1),                   /* -- s1 s2 s3 e p p */
	ROLL(5),                  /* -- s1 p s2 s3 e p */
	DUP(1),                   /* -- s1 p s2 s3 e p p */
	ROLL(4),                  /* -- s1 p s2 p s3 e p */
	DUP(1),                   /* -- s1 p s2 p s3 e p p */
	ROLL(3),                  /* -- s1 p s2 p s3 p e p */
	ROLL(8),                  /* -- p s1 p s2 p s3 p e */
	ROLL(8),                  /* -- e p s1 p s2 p s3 p */

	EQ(2),                    /* -- e p s1 p s2 p s3? */
	ROLL(5),                  /* -- e p s3? s1 p s2 p */
	EQ(2),                    /* -- e p s3? s1 p s2? */
	ROLL(3),                  /* -- e p s3? s2? s1 p */
	EQ(2),                    /* -- e p s3? s2? s1? */
	OR(3),                    /* -- e p not-valid? */
	JMPIF(4),                 /* -- e p */

	/* valid (no redef) */
	POP(2),                   /* -- */
	PUSHB(true),              /* -- true */
	RETURN,

	/* not valid */
	ROLL(2),                     /* -- p e */
	PUSHS(13),                   /* -- p e msg */
	ROLL(3),                     /* -- msg p e */
	PUSHN(VS_TBL_REDEF_ERR_EID), /* -- msg p e eid */
	ROLL(4),                     /* -- eid msg p e */
	CALL(EMIT_ERROR),            /* -- */
	PUSHB(false),                /* -- valid? */
	RETURN,
	
	
	/* HANDLE_PARMERR:
	 * entry -- 
	 */
	INPUT(1),                 /* -- entry pad0 */
	POP(1),                   /* -- entry */
	INPUT(2),                 /* -- entry pad12 */
	POP(1),                   /* -- entry */
	INPUT(4),                 /* -- entry lbnd */
	POP(1),                   /* -- entry */
	INPUT(4),                 /* -- entry hbnd */
	POP(1),                   /* -- entry */
	CALL(EMIT_ERROR_PARMERR), /* -- */
	RETURN,
	
	/* INC_UNUSED:
	 * old-unused valid parmid -- new-unused valid parmid
	 */
	ROLL(3),                 /* -- parmid old-unused valid */
	ROLL(3),                 /* -- valid parmid old-unused */
	PUSHN(1),                /* -- valid parmid old-unused 1 */
	ADD,                     /* -- valid parmid new-unused */
	ROLL(3),                 /* -- new-unused valid parmid */
	RETURN,

	
	/* INC_VALID:
	 * unused old-valid parmid -- unused new-valid parmid
	 */
	ROLL(2),                 /* -- unused parmid new-valid   */
	PUSHN(1),                /* -- unused parmid old-valid 1 */
	ADD,                     /* -- unused parmid new-valid   */
	ROLL(2),                 /* -- unused new-valid parmid   */
	RETURN,

	
	/* COMPUTE_INVALID:
	 * unused valid -- unsused invalid valid
	 */
	DUP(2),     /* -- unused valid unused valid */
	ADD,        /* -- unused valid not-invalid */
	PUSHN(4),   /* -- unused valid not-invalid total-entries */
	ROLL(2),    /* -- unused valid total-entries not-invalid */
	SUB,        /* -- unused valid invalid */
	ROLL(2),    /* -- unused invalid valid */
	RETURN,


	/* COMPUTE_RESULT:
	 * u i v -- valid? u i v
	 *
	 * Compute the final valid? result the validation function
	 * should return based on the invalid entry count.
	 */
	ROLL(2),    /* -- u v i */
	DUP(1),     /* -- u v i i */
	PUSHN(0),   /* -- u v i i 0 */
	EQ(2),      /* -- u v i valid? */
	ROLL(4),    /* -- valid? u v i */
	ROLL(2),    /* -- valid? u i v */
	RETURN,
	
	
	/* EMIT_INFO:
	 * unused invalid valid --
	 */
	PUSHS(0),
	OUTPUT,    /* "Table image entries: " */
	OUTPUT,    /* "Table image entries: v" */
	PUSHS(1),
	OUTPUT,    /* "Table image entries: v valid, " */
	OUTPUT,    /* "Table image entries: v valid, i" */
	PUSHS(2),
	OUTPUT,    /* "Table image entries: v valid, i unused, " */
	OUTPUT,    /* "Table image entries: v valid, i unused, u" */
	PUSHS(3),
	OUTPUT,    /* "Table image entries: v valid, i unused, u unused" */
	PUSHN(VS_VALIDATION_INF_EID),          /* -- eid */
	PUSHN(CFE_EVS_EventType_INFORMATION),  /* -- eid etype */
	FLUSH,                                 /* -- */
	RETURN,

	/* EMIT_ERROR_PARMERR:
	 * entry --
	 */
	PUSHS(4), /* -- entry str ; "" */
	OUTPUT,   /* -- entry     ; "Table entry " */
	OUTPUT,   /* --           ; "Table entry e" */
	PUSHS(7), /* -- str       ; "Table entry e" */
	OUTPUT,   /* --           ; "Table entry e invalid Parm ID" */
	PUSHN(VS_TBL_PARM_ERR_EID),      /* -- eid */
	PUSHN(CFE_EVS_EventType_ERROR),  /* -- eid etype */
	FLUSH,                           /* -- */
	RETURN,
	
	/* EMIT_ERROR:
	 * eid msg parm entry --
	 */
	PUSHS(4), /* -- eid msg parm entry str ; "" */
	OUTPUT,   /* -- eid msg parm entry     ; "Table entry " */
	OUTPUT,   /* -- eid msg parm           ; "Table entry e" */
	PUSHS(5), /* -- eid msg parm str       ; "Table entry e" */
	OUTPUT,   /* -- eid msg parm           ; "Table entry e parm " */
	CALL(PARM_TO_STR),  /* -- eid msg ps   ; "Table entry e parm " */
	OUTPUT,   /* -- eid msg                ; "Table entry e parm ps" */
	OUTPUT,   /* -- eid                    ; "Table entry e parm ps m" */
	PUSHN(CFE_EVS_EventType_ERROR),  /* -- eid etype */
	FLUSH,                           /* -- */
	RETURN,

	/* PARM_TO_STR:
	 * parmid -- parmstring
	 */
	DUP(1),                 /* -- parmid parmid */
	PUSHN(VS_PARM_UNUSED),  /* -- parmid parmid code */
	EQ(2),                  /* -- parmid equal? */
	NOT,                    /* -- parmid not-equal? */
	JMPIF(4),               /* -- parmid */
	POP(1),                 /* -- */
	PUSHS(14),              /* -- string */
	RETURN,

	DUP(1),                 /* -- parmid parmid */
	PUSHN(VS_PARM_APE),     /* -- parmid parmid code */
	EQ(2),                  /* -- parmid equal? */
	NOT,                    /* -- parmid not-equal? */
	JMPIF(4),               /* -- parmid */
	POP(1),                 /* -- */
	PUSHS(15),              /* -- string */
	RETURN,
	
	DUP(1),                 /* -- parmid parmid */
	PUSHN(VS_PARM_BAT),     /* -- parmid parmid code */
	EQ(2),                  /* -- parmid equal? */
	NOT,                    /* -- parmid not-equal? */
	JMPIF(4),               /* -- parmid */
	POP(1),                 /* -- */
	PUSHS(16),              /* -- string */
	RETURN,

	DUP(1),                 /* -- parmid parmid */
	PUSHN(VS_PARM_CAT),     /* -- parmid parmid code */
	EQ(2),                  /* -- parmid equal? */
	NOT,                    /* -- parmid not-equal? */
	JMPIF(4),               /* -- parmid */
	POP(1),                 /* -- */
	PUSHS(17),              /* -- string */
	RETURN,

	DUP(1),                 /* -- parmid parmid */
	PUSHN(VS_PARM_DOG),     /* -- parmid parmid code */
	EQ(2),                  /* -- parmid equal? */
	NOT,                    /* -- parmid not-equal? */
	JMPIF(4),               /* -- parmid */
	POP(1),                 /* -- */
	PUSHS(18),              /* -- string */
	RETURN,

	DUP(1),                 /* -- parmid parmid */
	PUSHN(VS_PARM_NORTH),   /* -- parmid parmid code */
	EQ(2),                  /* -- parmid equal? */
	NOT,                    /* -- parmid not-equal? */
	JMPIF(4),               /* -- parmid */
	POP(1),                 /* -- */
	PUSHS(19),              /* -- string */
	RETURN,

	DUP(1),                 /* -- parmid parmid */
	PUSHN(VS_PARM_SOUTH),   /* -- parmid parmid code */
	EQ(2),                  /* -- parmid equal? */
	NOT,                    /* -- parmid not-equal? */
	JMPIF(4),               /* -- parmid */
	POP(1),                 /* -- */
	PUSHS(20),              /* -- string */
	RETURN,

	DUP(1),                 /* -- parmid parmid */
	PUSHN(VS_PARM_EAST),    /* -- parmid parmid code */
	EQ(2),                  /* -- parmid equal? */
	NOT,                    /* -- parmid not-equal? */
	JMPIF(4),               /* -- parmid */
	POP(1),                 /* -- */
	PUSHS(21),              /* -- string */
	RETURN,

	DUP(1),                 /* -- parmid parmid */
	PUSHN(VS_PARM_WEST),    /* -- parmid parmid code */
	EQ(2),                  /* -- parmid equal? */
	NOT,                    /* -- parmid not-equal? */
	JMPIF(4),               /* -- parmid */
	POP(1),                 /* -- */
	PUSHS(22),              /* -- string */
	RETURN,

	POP(1),                 /* -- */
	PUSHS(23),              /* "unknown" */
	RETURN,
};


#endif
