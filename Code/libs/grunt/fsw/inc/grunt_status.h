#ifndef _GRUNT_STATUS_H_
#define _GRUNT_STATUS_H_

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

/* A well-formed Grunt program puts either true or false on the arg
 * stack and then HALTs.  When the Grunt interpreter executes a HALT
 * instruction and finds a Boolean value on the stack, it returns one
 * of these two values to the caller.
 */
#define GRUNT_HALT_TRUE  0x01  /* Program halted with true on the arg stack */
#define GRUNT_HALT_FALSE 0x02  /* Program halted with false on the arg stack */

/* The Grunt interpreter uses this error code when it encounters a bug
 * in its own code, as opposed to a bug in the Grunt program it is
 * interpreting.  These might have been assert()s describing states
 * that the interpreter ought not to reach, except that we eventually
 * want to include this interpreter in an application that must keep
 * running as best it can rather than halting upon reaching an
 * unexpected bug state.
 */
#define GRUNT_ERROR_INTERPRETERBUG    0x11  /* Grunt interpreter bug */

/* The Grunt interpreter uses these remaining error codes to report
 * bugs in the Grunt program it is interpreting.
 */
#define GRUNT_ERROR_INVALIDARGUMENT   0x12  /* bad arg from stack */
#define GRUNT_ERROR_INVALIDLITERAL    0x13  /* bad literal in program */
#define GRUNT_ERROR_INVALIDOPCODE     0x14  /* bad opcode in program */
#define GRUNT_ERROR_NOLOOPS           0x15  /* no backwards jumps or calls */
#define GRUNT_ERROR_NOPROGRAM         0x16  /* PC beyond end of program */
#define GRUNT_ERROR_OUTOFBOUNDS       0x17  /* stack, buffer over/underflow */

#endif
