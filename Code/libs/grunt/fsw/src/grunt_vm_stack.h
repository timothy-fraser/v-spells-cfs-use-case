#ifndef _GRUNT_VM_STACK_H_
#define _GRUNT_VM_STACK_H_

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

int grunt_vm_dup(grunt_rep_t);
int grunt_vm_pop(grunt_value_t *, grunt_rep_t);
int grunt_vm_pushb(const grunt_value_t *);
int grunt_vm_pushn(const grunt_value_t *);
int grunt_vm_pushs(const grunt_value_t *);
int grunt_vm_roll(grunt_rep_t);

#endif
