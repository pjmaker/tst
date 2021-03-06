/*
 * tst-split - split a line into CSV separated fields
 *
 * Copyright (c) 2015, Phil Maker
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

#include <stdio.h>
#include <string.h>

#include "tst-split.h"
// split a string
static char *fields[100];
static int nfields;

int split_csv(char *s) {
  nfields = 0;
  fields[0] = s;
  for(;;) {
    // printf("split_csv processing %s\n", s);
    if(*s == '\0') { // end of string
      return ++nfields;
    } else if(*s == ',') { // new field
      *s = '\0'; // terminate old one
      fields[++nfields] = ++s; 
    } else { // keep moving
      s++;
    }
  }
}

char* field(int n) { 
  return fields[n];
}

void print_fields() {
  int i;
  for(i = 0; i != nfields; i++) {
    printf("#%d = %s\n", i, fields[i]);
  }
}

#ifdef TEST
int main() {
  char* tests[] = {
    "hello",
    "field1,field2",
    ",f",
    "",
    ",,,,",
    NULL
  };

  int i;
  for(i = 0; tests[i] != NULL; i++) { 
    printf("split('%s') -> ", tests[i]);
    // you need the strdup since we modify the string in place
    int nf = split_csv(strdup(tests[i]));
    printf("%d fields\n", nf);
    print_fields();
  }
}
#endif
