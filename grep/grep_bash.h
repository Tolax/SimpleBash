#ifndef S21_GREP
#define S21_GREP
#define _GNU_SOURCE

#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Options {
  int e;
  int i;
  int v;
  int c;
  int l;
  int n;
  int h;
  int s;
  int f;
  int o;
  char *pattern;
  int len_pattern;
  int mem_pattern;
};

void grep(int argc, char *argv[]);
void output_line(char *line, int n);
void process_file(struct Options *options, char *path, regex_t *reg, int argc);
void add_reg_from_file(struct Options *options, char *filepath);
void output(struct Options *options, int argc, char **argv);
void pattern_add(struct Options *options, char *pattern);
void flag_parser(int argc, char *argv[], struct Options *Options);
void pattern_add(struct Options *options, char *pattern);

#endif
