#ifndef S21_CAT
#define S21_CAT
#include <getopt.h>
#include <stdio.h>

struct Options {
  int b;
  int e;
  int n;
  int s;
  int t;
  int v;
};

int print_file(char *filename, struct Options *Options);
void flag_parser(int argc, char *argv[], struct Options *Options);

#endif