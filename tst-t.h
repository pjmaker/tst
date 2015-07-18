/*
 * tst-t.h - a time stamp parser
 *
 * Copyright (c) 2015, Phil Maker <philip.maker@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _TST_T_H_
#define _TST_T_H_ 1

#include <time.h>
#include <limits.h>
#include <stdbool.h>

// times are represented internally as milliseconds
// since the epoch
typedef long tms; // time in milliseconds since epoch
#define NOTIME LONG_MIN // invalid time value
#define ISTIME(v) ((v) != NOTIME) // 

tms parse_t(char* s);
tms parse_tf(char* s, char* fmt[]);

char* fmt_t(tms);

bool parse_t_header(char* s, bool* delta, tms* size);
char* unparse_t_header(bool delta, tms size);

void print_tm(struct tm* tmp);
void print_iso8601(struct tm* tmp);
void print_t(tms t);


#endif /* _TST_T_H_ */
