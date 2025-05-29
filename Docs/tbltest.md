# Tbltest Test Suite

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


The `tbltest` program implements a functional test suite that runs a
fixed deterministic series of automated tests to explore the NASA Core
Flight System (cFS) table validation workflow. It uses a fixed series
of valid and invalid table images to explore the control flow
alternatives in the TBL load-validate-activate workflow and each of
the specific validity property checks in the V-SPELLS challenge app
table validation functions. It also uses the cFS performance
monitoring feature to time the execution of the validation functions,
producing data useful for performance comparisons.


## How to Run

Once you have built the integrated cFS/use case software (see
[build.md](build.md)), you will need two terminals to run the tests:

Terminal 1: Make your current working directory `build/exe/cpu1` and
run `./core-cpu1`.  Wait until you see `./core-cpu1` indicate that
it is up an running by emitting a line that ends with
`CFE_ES_Main: CFE_ES_Main entering OPERATIONAL state`
before proceeding.


Terminal 2: Make your current working directory `build/exe/host`.  Run
`./tbltest` or `./tbltest --vsa` to test the V-SPELLS Alpha (VSA)
app. Run `./tbltest --vsb` or `./tbltest --vsc` to test the V-SPELLS
Bravo (VSB) or Charlie (VSC) apps.

The current working directories are important as both `core-cpu1` and
`tbltest` will look for the simulated spacecraft filesystem
`build/exe/cpu1/cf` using paths relative to those locations.

You may also need to increase your kernel's maximum POSIX message
queue depth as described at the end of this file.


## Tbltest output

To help you track the status of its tests, `Tbltest` will emit output
to its terminal like this:

```
FILE: create table image with bounds out of order.
      Parm:   South Pad: 0x000000 Low: 0x00010000  High: 0x00010000
      Parm:     Ape Pad: 0x000000 Low: 0x00001000  High: 0x00000010
      Parm:  Unused Pad: 0x000000 Low: 0x00000000  High: 0x00000000
      Parm:  Unused Pad: 0x000000 Low: 0x00000000  High: 0x00000000
PERF: start storing performance events.
SENT:      CFE_ES  CMD  PSTRT
TEST: load file into inactive image.
SENT:      CFE_TBL CMD  LOAD  /cf/tbltest_Prm.tbl
WANT:      CFE_TBL INFO LOAD  Successful load of '/cf/tbltest_Prm.tbl' into 'VS
SEEN:                              V-SPELLS App Alpha (VSA) housekeeping
SEEN:                              V-SPELLS App Bravo (VSB) housekeeping
SEEN:                         Telemetry Output (TO, TO_LAB) housekeeping
SEEN:      CFE_TBL INFO LOAD  Successful load of '/cf/tbltest_Prm.tbl' into 'VS
PASS.
TEST: validate inactive image.
SENT:      CFE_TBL CMD  VALID VSA_APP.Prm
WANT:      VSA_APP EROR ORDER Table entry 1 parm Ape invalid bound order
SEEN:                               Executive Services (ES) housekeeping
SEEN:                                  Event Services (EVS) housekeeping
SEEN:                                  Time Services (TIME) housekeeping
SEEN:      VSA_APP EROR ORDER Table entry 1 parm Ape invalid bound order
PASS.
WANT:      VSA_APP INFO VINFO Table image entries: 1 valid, 1 invalid, 2 unused
SEEN:      VSA_APP INFO VINFO Table image entries: 1 valid, 1 invalid, 2 unused
PASS.
WANT:      CFE_TBL EROR VALER VSA_APP validation failed for Inactive 'VSA_APP.P
SEEN:      CFE_TBL EROR VALER VSA_APP validation failed for Inactive 'VSA_APP.P
PASS.
PERF: stop storing performance events.
SENT:      CFE_ES  CMD  PSTOP
PERF: Verification function execution duration in ticks: 27940
```

This output contains several kinds of lines each with its own
four-character prefix:

FILE: Tests begin by creating an inactive table image with values
carefully chosen to cover a particular execution path through the
table validation workflow.  These lines describe the inactive table
image for the subsequent tests.

PERF: These lines show the test suite's simulated ground station
commanding the simulated spacecraft to monitor the performance of the
validation function under test.  For tests that pass, they report
validation function execution time measured in simulated spacecraft
clock ticks.

SENT: These lines show the table load, validate, and activate commands
sent from the test suite's simulated ground station to the simulated
spacecraft.

WANT: These lines indicate the response from the spacecraft
the ground station needs to see for the test to pass.

SEEN: These lines show the messages the spacecraft sends to the ground
station. The spacecraft is quite chatty; many services will send
"housekeeping" telemetry messages that are irrelevant and ignored by
the test.  `Tbltest` nonetheless notes them in its output to reassure
the user that it is making progress and to help with debugging.

PASS: These lines indicate that `Tbltest` has seen the response it
wanted from the spacecraft and the most recent test has passed.

FAIL: These lines indicate that `Tbltest` got tired of waiting for the
response it wanted.  It has decided the most recent test failed and
moved on to the next test.


## POSIX Message Queue Depth

When built for simulation on a desktop, cFS uses POSIX message queues
to implement message-passing between components.  `Tbltest` causes the
simulated spacecraft to send many messages to the simulated ground
station.  On my particular GNU/Linux system, these messages overflowed
the queues and `core-cpu1` began reporting error events  like these:

```
EVS Port1 66/1/CFE_SB 25: Pipe Overflow,MsgId 0x808,pipe TO_LAB_TLM_PIPE,sender CFE_TIME.TIME_1HZ_TASK
EVS Port1 66/1/CFE_SB 25: Pipe Overflow,MsgId 0x808,pipe TO_LAB_TLM_PIPE,sender VSA_APP
```

These error events indicated that the cFS Event Service was dropping
some spacecraft responses it ought to have delivered to the ground
station.  When it dropped a response `tbltest` wanted to see, a test
would fail despite there being no bug.

To avoid this problem, increase your kernel's maximum queue depth
setting.  On my GNU/Linux system, I added this line to my
`/etc/sysctl.conf` file:

```
fs.mqueue.msg_max = 50
```

As a sanity check, after reboot `cat /proc/sys/fs/mqueue/msg_max`
should report 50.
