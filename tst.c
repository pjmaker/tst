/*
 * tst.c - the time series transmogrifier
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
#include "tst-t.h"

// global options which are settable via
// command line
bool   help;
bool   meta_add = false;
bool   meta_strip;
double dv;
double zdb;
tms st;
tms et;
char* vfmt;
char* sep;
char* recsep;
bool show_parsed_t;
bool show_parsed_v;
bool show_input; 
tms every;
char* topt;

static void process(char* filename); // process an input file

int main(int argc, char** argv) {
  init_options(argc, argv);
  // show_options();

  // grab all the options
  help = option_bool("-help", "1", "What is it?");
  meta_add = option_bool("-meta_add", "0", "What is it?");
  meta_strip = option_bool("-meta_strip", "1", "What is it?"); 
  dv = option_double("-dv", "0", "What is it?");
  zdb = option_double("-zdb", "0", "What is it?");
  st = option_time("-st", "1970-1-1", "What is it?");
  et = option_time("-et", "3000-1-1", "What is it?");
  vfmt = option("-vfmt", "%g", "What is it?");  
  sep = option("-sep", ",", "What is it?");
  recsep = option("-recsep", "\n", "What is it?");

  show_input = option_bool("-show_input", "0", "What is it?");
  show_parsed_t = option_bool("-show_parsed_t", "0", "What is it?");
  show_parsed_v = option_bool("-show_parsed_v", "0", "What is it?");

  every = option_period("-every", "0", "What is it?");

  topt = option("-t", "iso", 
	     "iso|10m|%Y/%M/...");

  if(help) { // we've printed the help message so exit
    exit(0);
  }
    
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

// read line from infp stripping out 
//  meta_data if -meta_strip
//  empty lines
//  finally trim the trailing \n 
static char line[1024];

static char* readline() {
  for(;;) {
    if(fgets(line, sizeof(line), infp) == NULL) {
      return NULL;
    } else {
      if(show_input) {
	printf("# line %s", line);
      }
      if(meta_strip && line[0] == '#') {
	// strip out meta_data from input
      } else if(line[0] == '\n') { 
	// strip out empty lines
      } else { // got it, just remove \n
	line[strlen(line)-1] = '\0';
	return line;
      }
    }
  }
}

// read the header line

static char* tlabel;
static char* vlabel;
static bool  read_delta;
static tms   read_tsize;

void read_header() { 
  if(readline()) { 
    if(split_csv(line) != 2) { 
      fprintf(stderr, "oops must have two fields in header\n");
      exit(99);
    }
    tlabel = strdup(field(0));
    if(!parse_t_header(tlabel, &read_delta, &read_tsize)) {
      fprintf(stderr, "failed to parse tlabel %s\n", tlabel);
      exit(100);
    }
    vlabel = strdup(field(1));
  } else {
    fprintf(stderr, "oops: no header\n");
    exit(11);
  }
}

void write_output(tms t, double v);
void write_header();

bool show_parsed_t;
bool show_parsed_v;
 
void read_input() { 
  read_header();
  write_header();
  while(readline()) { 
    if(split_csv(line) != 2) {
      fprintf(stderr, "wrong number of fields\n");
      exit(90);
    }

    tms t = parse_t(field(0)) * read_tsize;
    if(read_delta) {
      static tms old_t = 0;
      t = t + old_t;
    }
    double v = strtod(field(1), NULL);
    
    if(show_parsed_t) {
      printf("* t = %ld = ", t);
      print_t(t);
      printf("\n");
    }
    if(show_parsed_v) { 
      printf("* v = %g\n", v);
    }
    write_output(t, v);
  }
}

tms st; // start time for output
tms et; // end time for output
char* vfmt; // format for printing a variable
char* sep; // separator between fields
char* recsep; // record separator
tms write_tsize; // step size for t in ms
bool write_delta; // delta encoded time

void write_header() {
  printf("%s,%s\n", 
	 unparse_t_header(write_delta, write_tsize), 
	 vlabel);
}

tms every; // every t ms show a sample if not 0

void write_output1(tms t, double v);
void write_every(tms t, double v);

static tms ot = 0;
static double ov = 0;

// write_output t v - 
void write_output(tms t, double v) {
  if(every == 0) { // not resampling the data
    write_output1(t, v); // so send it straight off
  } else {
    write_every(t, v);
  }
  ot = t;
  ov = v;
}

tms next_every(tms t) {
  return ((t / every) + 1) * every;
}

static bool first = true;

// write_every every ms t,v so
// collect samples until the last one before 
// a value that rounds 
void write_every(tms t, double v) { 
  if(first) {
    if((t % every) == 0) {
      write_output1(t,v);
    }
    first = false;
  } else {
    while((ot = next_every(ot)) < t) {
      write_output1(ot, ov);
    }
    if(ot == t) {
      write_output1(t, v);
    }
  }
}


double zdb = 0;
double dv = 0;

bool v_changed(double v) {
  static double ov; 
  static bool first = true; // first 
  if(first) {
    first = false;
    ov = v;
    return true;
  }

  if(-zdb < v && v < zdb) { // treat it 0 since its in zdb
    v = 0;
  }
  double d = v - ov; // the change in value 
  if(-dv < d && d < dv) { // less than dv so ignore it
    return false;
  } else {
    ov = v;
    return true;
  }
}

void write_sample(tms t, double v) {
  tms tv;
  static tms tb;

  if(write_delta) { 
    tv = (t - tb) / write_tsize;
  } else {
    tv =  t / write_tsize;
  }
  tb = t;
  if(strcmp(topt, "iso") == 0) { // ttt speed
    printf("%s", fmt_t(t));
  } else if(topt[0] == '%') {
    printf("%s", fmt_tg(t, topt));
  } else {
    printf("%ld", tv);
  }
  printf("%s", sep);
  printf(vfmt, v);
  printf("%s", recsep);
}

void write_output1(tms t, double v) {
  if(st <= t && t <= et) {
    if(v_changed(v)) { 
      write_sample(t, v);
    }
  } else { 
    // outside the -st..-et range
  }
}

static void process(char* filename) {
  if(meta_add) {
    printf("# process %s\n", filename);
  }
  open_filename(filename);
  setlinebuf(stdout);
  read_input();
}


