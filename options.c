/*
 * ts-options.c - a very simple option handler.
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

// shared state for option handling - these are 
// all exported via get_* functions

static char* progname; // program name argv[0] 

static int noptions; // number of options (-opt val)
static char** options; // list of (-opt val)

static int nfiles; // number of filenames
static char** files; // list of filenames

// initialise options based on argc/argv
void init_options(int argc, char* argv[]) { 
  int i;

  // save progname away for error messages
  assert(argc >= 1);
  progname = argv[0]; 

  // process -opt val pairs first
  options = &argv[1];
  for(i = 1; i < argc; i++) {
    if(argv[i][0] == '-') { // -option
      i++;
      if(i < argc) { // value ok
	noptions++;
      } else {
	fprintf(stderr, "%s: fatal no argument for %s\n",
		progname, argv[i]);
	exit(101);
      }
    } else {
      break;
    }
  }

  // the rest must be filenames that we can read
  FILE *fp;
  files = &argv[i];
  for(nfiles = 0; i < argc; i++) {
    errno = 0;
    if((fp = fopen(argv[i], "r")) == NULL) {
      fprintf(stderr, "%s: fatal error cannot open file \"%s\": %s\n", 
	      get_progname(), 
	      argv[i],
	      strerror(errno));
      exit(102);
    } else {
      fclose(fp);
      nfiles++;
    }
  }
}

// returns progname, i.e. argv[0]
char* get_progname() {
  return progname;
}

// return option name for slot i, -opt
static char* get_opt(int i) {
  assert(i < noptions);
  return options[i*2];
}

// return the value for slot i, arg-a
static char* get_val(int i) {
  assert(i < noptions);
  return options[(i * 2) + 1];
}

// return the filename for slot i or
// NULL if no more files
char* get_filename(int i) { 
  if(0 <= i && i < nfiles) {
    return files[i];
  } else {
    return NULL;
  }
}

// show all options on stdout
void show_options() {
  int i;
  for(i = 0; i < noptions; i++) {
    printf("# option[%d] = %s %s\n", i, get_opt(i), get_val(i));
  }
  for(i = 0; i < nfiles; i++) {
    printf("# file[%d] = %s\n", i, files[i]);
  }
}

// get value for option opt defaulting to dflt if
// its not given.
char* option(char* opt, char* dflt) {
  int i;
  for(i = 0; i < noptions; i++) {
    if(strcmp(opt, get_opt(i)) == 0) { // match
      return get_val(i);
    }
  }
  return dflt;
}

// return bool option
bool option_bool(char* opt, char* dflt) {
  char* s = option(opt,dflt);
  // we could add in plain text options for booleans
  return atoi(s) != 0;
}

// return double option
double option_double(char* opt, char* dflt) {
  return atof(option(opt,dflt));
}

// return long option
long option_long(char* opt, char* dflt) {
  return atol(option(opt,dflt));
}

bool option_t(char* opt, bool* tdelta, tms* tsize) {
  if(parse_t_header(option(opt, ""), tdelta, tsize)) {
    return true;
  } else { 
    return false;
  }
}

tms option_time(char* opt, char* dflt) {
  return parse_t(option(opt,dflt));
}


#ifdef TEST

int main(int argc, char* argv[]) {
  int i;
  for(i = 0; i < argc; i++) {
    printf("argv[%d] %s\n", i, argv[i]);
  }
  init_options(argc, argv);
  show_options();
  return 0;
}
#endif
