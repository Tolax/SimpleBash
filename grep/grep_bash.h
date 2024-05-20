#ifndef S21_GREP
#define S21_GREP
#include <stdio.h>
#include <getopt.h>

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
    char* pattern; 
    int len_pattern;
    int mem_pattern;
};

int print_file(/* char *filename, struct Options *Options */);
void flag_parser(int argc, char *argv[], struct Options *Options);

#endif