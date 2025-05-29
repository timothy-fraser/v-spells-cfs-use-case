# Building the Use Case

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


This file describes how to retrieve the needed NASA cFS components,
integrate the new use case code, and build the integrated system
for execution on a single Linux host.


## Retrieve the cFS components

First, unzip, untar, or clone the repository for this use case to
produce the following directory structure:

```
v-spells-cfs-use-case/
  CFS/    ;; initially empty, will hold NASA cFS distro tarballs
  Code/   ;; contains the new code for this use case
  Diffs/  ;; applying these diffs will integrate the new code with cFS
  Docs/   ;; holds the use case documentation files
  setup   ;; a script that automates the cFS/use-case integration
```

Then retrieve the following tar archive files from the NASA cFS
repository on GitHub and place them in the `CFS` subdirectory.

```
cFS-draco-rc5.tar.gz
cFE-draco-rc5.tar.gz
osal-draco-rc5.tar.gz
PSP-draco-rc5.tar.gz
ci_lab-draco-rc5.tar.gz
sch_lab-draco-rc5.tar.gz
to_lab-draco-rc5.tar.gz
sample_app-draco-rc5.tar.gz
sample_lib-draco-rc5.tar.gz
cFS-GroundSystem-draco-rc5.tar.gz
elf2cfetbl-draco-rc5.tar.gz
tblCRCTool-draco-rc5.tar.gz
```


## Integrate cFS and the new use case code

Change to the `v-spells-cfs-use-case` directory and run `./setup`.
This script will first confirm you have all the cFS tar archives it
needs.  It will then create a `cFS-draco-rc5-vs` subdirectory that
contains the cFS code and new use case code integrated together,
configured to build executables for use on a single Linux host.

Note that the `setup` script will refuse to run a second time to avoid
clobbering any changes you may have made to the source under
`cFS-draco-rc5-vs`.  If you want to start fresh, remove that
subdirectory and run `setup` again.


## Build and install

You can build the integrated system and install it for testing using
these commands:

```
cd cFS-draco-rc5-vs
make
make install
```

Researchers familiar with cFS may wonder what happened to the initial
file copying and `make prep` steps that begin the NASA build procedure
in the cFS README.  The `setup` script described above has already
handled these steps.

The `make ; make install` procedure places all of its output under a
`v-spells-cfs-use-case/cFS-draco-rc5-vs/build` subdirectory.  It will
not attempt to install files elsewhere on your host; you can run this
procedure without any kind of special administrative privileges.

Once you have built and installed, see [tbltest.md](tbltest.md) for
instructions on how to run the system.


## Adding new components

If you decide to add your own new components to the system rather than
simply modifying old ones to solve the challenges, you will need to
take some extra steps to integrate them.  NASA's own cFS documentation
is the best source for information on writing your own apps and
libraries, but here is a checklist of some important things you will
need to do:

  - Add new apps and libraries to `sample_defs/targets.cmake` to cause
    the build system to build them.
    
  - Add new apps and libraries to
    `sample_defs/cpu1_cfe_es_startup.scr` to cause the cFE Executive
    service to initialize and/or run them.

  - Add a line for each new app to the sch_lab scheduler service's
    configuration table in `sch_lab_table.c` so the scheduler will
    prompt them to do housekeeping periodically.

  - Add a line for each new app to the to_lab telemetry output
    service's configuration table in `to_lab_sub.c` so that it will
    pass their telemetry messages to the ground station.

When you add new apps or libraries, you will need to 

```
cd cFS-draco-rc5-vs
make SIMULATION=native prep
```

to make the build system aware of them before executing `make ; make
install`.
