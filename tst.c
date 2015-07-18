/*
 * tst.c
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
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "options.h"
#include "tst-split.h"

// global options which are settable via
// command line
bool   meta_add = false;
bool   meta_strip;
double dv;
double zdb;

static void process(char* filename); // process an input file

int main(int argc, char** argv) {
  init_options(argc, argv);
  // show_options();

  // grab all the options
  meta_add = option_bool("-meta_add", "0");
  meta_strip = option_bool("-meta_strip", "1"); 
  dv = option_double("-dv", "0");
  zdb = option_double("-zdb", "0");

  // add the command line
  if(meta_add) {
    printf("# %%");
    for(int i = 0; i < argc; i++) {
      printf(" %s", argv[i]);
    }
    printf("\n");
  }

  // process the files
  if(get_filename(0) == NULL) {
    process("-");
  } else {
    for(int i = 0; get_filename(i) != NULL; i++) {
      process(get_filename(i));
    }
  }

  return 0;
}

// open infp using filename or stdin if its "-"
static FILE* infp;

static void open_filename(char* filename) {
  if(strcmp(filename,"-") == 0) {
    infp = stdin;
  } else {
    errno = 0;
    if((infp = fopen(filename, "r")) == NULL) {
      fprintf(stderr, "%s: fatal error cannot open file \"%s\": %s\n", 
	      get_progname(), 
	      filename,
	      strerror(errno));
      exit(103);
    }
  }
}

static char line[1024];

static char* getline() {
  for(;;) {
    if(fgets(line, sizeof(line), infp) == NULL) {
      return NULL;
    } else {
      if(meta_strip && line[0] == '#') {
	// strip out meta_data from input
      } else {
	return line;
      }
    }
  }
}

static void process(char* filename) {
  if(meta_add) {
    printf("# process %s\n", filename);
  }
  open_filename(filename);
  while(getline() != NULL) { 
    printf("%s", line);
    int nf = split_csv(line);
    printf("nf = %d\n", nf);
    print_fields();
  }
}
