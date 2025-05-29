# GRUNT PROGRAMMING MANUAL

```
Copyright (c) 2024 Timothy Jon Fraser Consulting LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied.  See the License for the specific language governing
permissions and limitations under the License.
```


Grunt is a Domain-Specific Language (DSL) for programming validation
functions for the simplified NASA Core Flight System (cFS) tables
described in [tbl-structure.md](tbl-structure.md). It is an example
solution for the second V-SPELLS cFS challenge problem that (aims to)
implement the table validation function specification in
[dsl-vf-spec.md](dsl-vf-spec.md).

Grunt's design is motivated by this (possibly misguided) chain of
reasoning:

1) Programmers are primarily concerned with verifying the memory
   safety, control flow safety, and termination properties of their
   table validation functions,

2) programs written in less powerful sub-Turing languages are easier
   to verify than ones written in general-purpose languages, and so

3) the weakest language that is still capable of implementing table
   validation functions is the best solution.

Ease of programming is notably absent from the above list of concerns.

No verification of Grunt safety and liveness properties has yet been
attempted.  However, the Grunt design aims to support an argument that
all Grunt programs:

- are control-flow safe: that is, Grunt programs can execute the
  control flow paths described in the source and no others.

- are memory safe: that is, Grunt programs cannot read or write memory
  outside the bounds of the input, output, and stack disciplines
  described later in this document.

- always terminate.


## Key Grunt design features

The following (lack of) features aim to make reasoning about
control-flow and termination easier:

- Grunt programs are a finite array of instructions. All instructions
  have equal length. The Grunt interpreter begins running a program
  at instruction 0 and proceeds from instruction `n` to instruction
  `n + 1` until it reaches the end of the array, unless diverted by
  the JMPIF, CALL, RETURN, or HALT instructions described below.

- Grunt supports conditional jumps with a JMPIF instruction and
  procedure calls with a CALL instruction. JMPIF and CALL both take
  the index of a target instruction as constant literal argument in
  the source.  Critically, the target index of a JMPIF or CALL
  instruction must be greater than the index of that JMPIF or CALL
  instruction itself - only "forward" jumps and calls are allowed.
  There are no function pointers in Grunt. All jump and call targets
  are constant and specified as literals in the source; Grunt programs
  cannot alter these targets at runtime.

- The CALL instruction pushes the index of the instruction following
  the CALL on a dedicated control stack that is separate and distinct
  from the argument stack Grunt programs use for computations. The
  RETURN instruction pops an index off of the control stack and causes
  the interpreter to execute the instruction at that index next.
  Grunt programs cannot manipulate the control stack outside of this
  CALL and RETURN discipline.  They cannot duplicate, reorder, or
  modify instruction indicies stored on the control stack.

- The HALT instruction terminates the program and returns a numeric
  result to the enclosing C program that invoked the embedded Grunt
  interpreter.

The following (lack of) features aim to make reasoning about
memory-safety easier:

- Grunt is a stack-based language. Grunt instructions pop their
  arguments from an argument stack, do some computation, and then push
  their results back on the argument stack. Although there is a DUP
  instruction that pushes an additional copy of arguments already on
  the stack and there is a ROLL instruction that reorders arguments on
  the stack, Grunt has no instruction that modifies the values of
  arguments on the stack without a pop-push transaction.

- Grunt has no heap, program-accesible registers, auxilliary
  dictionary, or other additional means of storing data for later
  retrieval. The argument stack is the only place Grunt programs can
  store computed values.

- Grunt programs use an INPUT instruction to read bytes from an input
  buffer containing the table data to validate.  The Grunt interpreter
  manages a cursor that indicates the index of the next byte to read
  and ensures that Grunt programs cannot read beyond the bounds of the
  input buffer.  Grunt programs cannot write their input buffers.

- Grunt programs use an OUTPUT instruction to write bytes to an output
  buffer that is just large enough to hold a maximum-length cFS event
  message string. The Grunt interpreter manages a cursor that
  indicates the index of the next byte to write and ensures that Grunt
  programs cannot write beyond the bounds of the output buffer.  Grunt
  programs cannot read their output buffers.


## Instruction semantics

The following subsections define the basic semantics of the Grunt
arithmetic, control, input/output, logic, and stack instructions.  The
final "Additional error conditions" subsection completes these
definitions by adding some additional error condition behavior that is
common to all instructions.

Grunt has three data types:

  - Boolean: either True or False.

  - Number: 32-bit unsigned integers.

  - String: Grunt programs specify a table of immutable string
    literals, generally the substrings table validation functions need
    to put in their telemetry messages. Each string in the table is a
    C string composed of one-byte ASCII characters with a NUL
    terminator. Each string is no longer than cFS
    `CFE_MISSION_EVS_MAX_MESSAGE_LENGTH`, including both characters
    and the NUL terminator. The Grunt interpreter
    implements its String type as an index into this immutable string
    literal table.

The definitions use the following conventions:

```
A, B, C are Boolean values
X, Y, Z are numbers
M is a string
S, T are instruction addresses
Q is a Boolean, number, or string

The -- symbol describes stack activity, like so:

  contents of the stack before the instruction -- contents after

  For example, POP 1;  A B C -- A B

```

### Arithmetic instructions

```
ADD
Argument stack: X Y -- Z
         where: Z = X + Y

ERROR CONDITION                                HALT AND RETURN
X + Y is > the maximum Number                  GRUNT_ERROR_OUTOFBOUNDS     0x17


SUB
Argument stack: X Y -- Z
         where: Z = X - Y

ERROR CONDITION                                HALT AND RETURN
X - Y is < 0                                   GRUNT_ERROR_OUTOFBOUNDS     0x17
```

### Control instructions

```
CALL T
Instruction pointer: T
      Control stack: -- S
      where: S is the instruction that follows the CALL.

ERROR CONDITION                                HALT AND RETURN
T is not greater than the CALL's address.      GRUNT_ERROR_NOLOOPS         0x15


HALT
Argument stack: B --

CONDITION                                      HALT AND RETURN
B is True                                      GRUNT_HALT_TRUE             0x01
B is False                                     GRUNT_HALT_FALSE            0x02


JMPIF T
Argument stack: B --

If B is True and T is greater than the JMPIF instruction's address,
set the instruction pointer to T.

If B is True and T is not greater than the JMPIF instruction's
address, halt and return GRUNT_ERROR_NOLOOPS.

Otherwise, do nothing and proceed to the next instruction as usual.


RETURN
Control stack: S --

Set the instruction pointer to S.
```

### Input/Output instructions

```
FLUSH
Argument stack: A B --

Emits a cFS event of Type A with Event ID B and the contents of the
output queue as its message, then sets the output queue to empty.


INPUT N
Argument stack: -- A
         where: A = the unsigned int value read, zero-extended to 32 bits.

If N is 1, reads a 1-byte number from the input queue at the current
cursor location and advances the cursor by 1.

If N is 2, reads a 2-byte number from the input queue at the current
cursor location and advances the cursor by 2.

If N is 4, reads a 4-byte number from the input queue at the current
cursor location and advances the cursor by 4.

ERROR CONDITION                                HALT AND RETURN
N is not 1, 2, or 4                            GRUNT_ERROR_INVALIDLITERAL  0x13
Attempt to read with cursor beyond input end.  GRUNT_ERROR_OUTOFBOUNDS     0x17


OUTPUT
Argument stack Q --

If Q is a Boolean, appends "True" or "False" to the output queue.

If Q is a number, appends a decimal representation of Q to the output queue.

If Q is a string, appends Q to the output queue.

ERROR CONDITION                                HALT AND RETURN
Output queue length exceeds
`CFE_MISSION_EVS_MAX_MESSAGE_LENGTH`           GRUNT_ERROR_OUTOFBOUNDS     0x17


REWIND N

If N is 0, set the input cursor to the beginning of the input queue.

If N > 0, subtract N bytes from the input cursor.

ERROR CONDITION                               HALT AND RETURN
N would reduce the input cursor below 0.      GRUNT_ERROR_OUTOFBOUNDS      0x17
```

### Logic instructions

```
AND N
Argument stack:  A1 .. AN -- B
         where: B is the Boolean AND of A1 through AN.

ERROR CONDITION                                HALT AND RETURN
N is 0 or 1.                                   GRUNT_ERROR_INVALIDLITERAL  0x13


GT
Argument stack: X Y -- B
         where: B is True if X > Y, else False.
	 

EQ N
Argument stack:  X1 .. XN -- B
         where: B is True if X1 .. XN are all equal, else False. 

ERROR CONDITION                                HALT AND RETURN
N is 0 or 1.                                   GRUNT_ERROR_INVALIDLITERAL  0x13


LT
Argument stack: X Y -- B
         where: B is True if X < Y, else False.
	 

NOT
Argument stack: A -- B
         where: B is the logical NOT of A.
	 

OR N
Argument stack:  A1 .. AN -- B
         where: B is the Boolean OR of A1 through AN.

ERROR CONDITION                                HALT AND RETURN
N is 0 or 1.                                   GRUNT_ERROR_INVALIDLITERAL  0x13
```

### Stack instructions

```
DUP N
Argument stack: Q1 .. QN -- Q1 .. QN Q1 .. QN

ERROR CONDITION                                HALT AND RETURN
N is 0.                                        GRUNT_ERROR_INVALIDLITERAL  0x13


POP N
Argument stack: Q1 .. QN -- 

ERROR CONDITION                                HALT AND RETURN
N is 0.                                        GRUNT_ERROR_INVALIDLITERAL  0x13


PUSHB B
Argument stack: -- B


PUSHN X
Argument stack: -- X


PUSHS M
Argument stack: -- M


ROLL N
Argument stack: Q1 .. QN-1 QN -- QN Q1 .. QN-1
```


### Additional error conditions

```
ERROR CONDITION                                HALT AND RETURN
Argument on stack is not of the expected type. GRUNT_ERROR_INVALIDARGUMENT 0x12
Literal in source is not of the expected type. GRUNT_ERROR_INVALIDLITERAL  0x13
Instruction has an invalid opcode.             GRUNT_ERROR_INVALIDOPCODE   0x14
Instruction fetch off the end of the program.  GRUNT_ERROR_NOPROGRAM       0x16
Fewer than expected arguments on stack.        GRUNT_ERROR_OUTOFBOUNDS     0x17
```



