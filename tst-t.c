/*
 * tst-t.c - a time stamp parser including ISO8601 and other
 *   formats including subsecond timestamps, e.g. millisecond
 *   timestamps.
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
#define __USE_XOPEN 1
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define _ISOC99_SOURCE
#include <math.h>
#include <errno.h>

#include "tst-t.h"

static int verbose = 0;

// currently cached fmt to use (-1 is invalid, so we do a lookup
// on startup
int cfmt = -1; 

// the actual timestamp formats
char* fmts[100][8] = {
  {"#", NULL },
  {"%Y-%m-%dT%H:%M:%S", NULL},
  {"%Y-%m-%dT%H:%M:%S", ".", NULL},
  {"%Y-%m-%dT%H:%M:%S", ".", "%z", NULL},
  {"%Y-%m-%d", NULL },
  {"%Y", "%z", NULL },

  // timestamps from OSIsoft PI
  {"%d/%m/%Y %H:%M:%S %p", NULL },
  { NULL }
};

// parse timestamp s against all the formats in fmts[]
// using parse_tf
tms parse_t(char* s) {
  if(verbose) {
    printf("* parse_t '%s'\n", s);
  }
  
  if(cfmt != -1) { // try the cached fmt first
    tms t = parse_tf(s, fmts[cfmt]);
    if(ISTIME(t)) {
      return t;
    } 
  }
    
  int i;
  for(i = 0; fmts[i][0] != NULL; i++) {
    if(verbose) {
      printf("** parse_t trying fmts[%d]\n", i);
    }
    tms t = parse_tf(s, fmts[i]);
    if(ISTIME(t)) {
      cfmt = i;
      return t;
    }
  }  
  if(verbose) { 
    printf("** parse_t all formats failed\n");
  }
  cfmt = -1;
  return NOTIME;
}

// parse timestamp s against fmt[0..] and return either
// the time as a double or NOTIME
tms parse_tf(char* s, char* fmt[]) {
  char* skip_ws(char*);
  char* parse_subsec(char*);
  tms parse_numeric_ts(char*);

  void init_subsec();
  tms get_subsec();

  char *ss = s;
  struct tm tmb;

  memset(&tmb, 0, sizeof(tmb));
  init_subsec();

  ss = skip_ws(ss);

  int i;
  for(i = 0; fmt[i] != NULL; i++) { 
    if(fmt[i][0] == '#') { // numeric timestamp
      return parse_numeric_ts(s);
    } else if(fmt[i][0] == '.') { // subsecond timestamp
      ss = parse_subsec(ss);
      if(ss == NULL) {
	if(verbose) {
	  printf("*** parse_tf '.' failed to match\n");
	}
	return NOTIME;
      } else {
	if(verbose) {
	  printf("*** parse_tf '.' matched leaving %s\n", ss);
	}
      }
    } else { // strptime format
      char* r = strptime(ss, fmt[i], &tmb);
      if(r == NULL) { 
	return NOTIME;
      } else { // match ok
	ss = r;
      }
    }
  }

  ss = skip_ws(ss);
  if(*ss != '\0') { 
    return NOTIME;
  } else {
    tms tb = (1000 * 
	      ((long)mktime(&tmb))) + get_subsec();
    return tb;
  }
}

tms parse_numeric_ts(char* s) {
  errno = 0;
  char *endp;
  tms tn = strtol(s, &endp, 10);
  if(errno == 0) {
    while(isspace(*endp)) {
      endp++;
    }
    if(*endp == '\0') {
      return tn;
    } 
  } 
  return NOTIME;
}

tms subsec = 0;

void init_subsec() {
  subsec = 0;
}

tms get_subsec() {
  return subsec;
}

char* parse_subsec(char* in) {
  float sd;
  int nc;
  if(sscanf(in, "%f%n", &sd, &nc) >= 1) { 
    subsec = (tms) (1000*(sd));
    return in+nc;
  } else {
    return NULL;
  }
}

char* skip_ws(char* s) { 
  while(isspace(*s)) {
    s++;
  }
  return s;
}

// parse the time header which looks like:
// [d]t[NNNN|][s|ms|m|h|d]
bool parse_t_header(char* s, bool* delta, tms* tsize) {
  char *p = s;
  p = skip_ws(p);

  *delta = false;
  *tsize = 1000;

  if(*p == 'd') {
    *delta = true;
    p++;
  }
  if(*p == 't') { 
    p++;
  } else {
    return false;
  }
  if(*p == '\0') {
    return true;
  }
  errno = 0;
  char *old_p = p;
  tms n = strtol(p, &p, 10);
  if(old_p == p) { // no digits 
    n = 1;
  } else if(errno != 0) { // default to 1s
    n = 1; 
  } else {
    // we got it into n so keep going
  }

  p = skip_ws(p);
  printf("@ %c %ld\n", *p, n);
  if(*p == '\0') { // default to seconds
    n *= 1000; 
  } else if(strncmp("ms", p, 2) == 0) {
    // leave n as is
  } else if(strncmp("s", p, 1) == 0) {
    n *= 1000;
  } else if(strncmp("m", p, 1) == 0) {
    n *= 60 * 1000;
  } else if(strncmp("h", p, 1) == 0) {
    n *= 60 * 60 * 1000;
  } else {
    return false;
  }
  *tsize = n;
  return true;
}

char* unparse_t_header(bool delta, tms t) {
  static char buf[80];
  char* p = buf;
  if(delta) { 
    *p++ = 'd';
  } 
  tms tv;
  char* u;
  if(t >= (3600 * 1000) && (t % (3600 * 1000)) == 0) { // h
    tv = t / (3600 * 1000);
    u = "h";
  } else if(t >= 60 * 1000 && (t % (60 * 1000)) == 0) { // m
    tv = t / (60* 1000);
    u = "m";
  } else if(t >= 1000 && (t % 1000) == 0) { // s
    tv = t / 1000;
    u = "s";
  } else { // ms     
    tv = t;
    u = "ms";
  }
  if(tv == 1) {
    snprintf(buf, sizeof(buf)-5, "t%s", u);
  } else {
    snprintf(buf, sizeof(buf)-5, "t%ld%s", tv, u);
  }
  return buf;
}

// fmt_* - format a string in a format, note these are not reentrant
//   but thats not a big problem.

char* fmt_t(tms t) { 
  if(!ISTIME(t)) {
    return "*";
  } else {
    time_t tsecs = (time_t) (t/1000);
    long subsecs = t % 1000;
    
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", gmtime(&tsecs));

    char ssbuf[80];
    if(subsecs != 0) {
      snprintf(ssbuf, sizeof(ssbuf), ".%03ld", subsecs);
    } else {
      ssbuf[0] = '\0';
    }
    
    static char r[80];
    snprintf(r, sizeof(r), "%s%sZ", buf, ssbuf);
    return r;
  }
}

void print_t(tms t) { 
  printf("%s", fmt_t(t));
}

void print_tm(struct tm* tmp) {
  printf("%d-%d-%dT%d:%d:%d %ld daylight saving %d offset %ld\n",
	 tmp->tm_year,
	 tmp->tm_mon,
	 tmp->tm_mday,
	 tmp->tm_hour,
	 tmp->tm_min,
	 tmp->tm_sec,
	 subsec, // hack data in but ....
	 tmp->tm_isdst,
	 tmp->tm_gmtoff
	 );
}

char* fmt_iso8601(struct tm* tmp) {
  char buf[80];
  strftime(buf, sizeof(buf), "%FT%H:%M:%S%z", tmp);
  static char r[80];
  snprintf(r, sizeof(r), "%s\n",buf);
  return r;
}

void print_iso8601(struct tm* tmp) {
  printf("%s", fmt_iso8601(tmp));
}





