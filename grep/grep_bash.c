#include "grep_bash.h"

int main(int argc, char *argv[]) {
  struct Options Options = {0};
  if (argc > 2) {
    flag_parser(argc, argv, &Options);
    output(&Options, argc, argv);
    free(Options.pattern);
  } else {
    return 1;
  }

  return 0;
}

void pattern_add(struct Options *options, char *pattern) {
  int n = strlen(pattern);

  if (options->pattern == 0) {
    options->pattern = malloc(1024 * sizeof(char));
    options->mem_pattern = 1024;
  }
  if (options->mem_pattern < options->len_pattern + n) {
    options->pattern =
        realloc(options->pattern, options->mem_pattern * 2 * sizeof(char));
    options->mem_pattern *= 2;
  }
  if (options->len_pattern != 0) {
    strcat(options->pattern + options->len_pattern, "|");
    options->len_pattern++;
  }
  options->len_pattern +=
      sprintf(options->pattern + options->len_pattern, "(%s)", pattern);
}

void add_reg_from_file(struct Options *options, char *filepath) {
  FILE *f = fopen(filepath, "r");
  if (f == NULL) {
    if (!options->s) {
      perror(filepath);
      printf("%s\n", filepath);
    }
    return;
  }
  char *line = NULL;
  size_t memlen = 0;
  int read = getline(&line, &memlen, f);

  while (read != -1) {
    if (line[read - 1] == '\n') {
      line[read - 1] = '\0';
    }
    pattern_add(options, line);
    read = getline(&line, &memlen, f);
  }
  free(line);
  fclose(f);
}

void flag_parser(int argc, char *argv[], struct Options *Options) {
  int opt;
  struct option long_options[] = {
      {"number-nonblank", 0, 0, 'b'},
      {"number", 0, 0, 'n'},
      {"squeeze-blank", 0, 0, 's'},
      {0, 0, 0, 0},
  };

  while ((opt = getopt_long(argc, argv, "e:ivclnhsf:o", long_options, NULL)) !=
         -1) {
    switch (opt) {
      case 'e':
        Options->e = 1;
        pattern_add(Options, optarg);
        break;
      case 'i':
        Options->i = REG_ICASE;
        break;
      case 'v':
        Options->v = 1;
        break;
      case 'c':
        Options->c = 1;
        break;
      case 'l':
        Options->l = 1;
        break;
      case 'n':
        Options->n = 1;
        break;
      case 'h':
        Options->h = 1;
        break;
      case 's':
        Options->s = 1;
        break;
      case 'f':
        Options->f = 1;
        add_reg_from_file(Options, optarg);
        break;
      case 'o':
        Options->o = 1;
        break;
    }
  }
  if (Options->len_pattern == 0) {
    pattern_add(Options, argv[optind]);
    optind++;
  }
  if (argc - optind == 1) {
    Options->h = 1;
  }
}

void output_line(char *line, int n) {
  for (int i = 0; i < n; i++) {
    putchar(line[i]);
  }
  if (line[n - 1] != '\n') putchar('\n');
}

void output(struct Options *options, int argc, char **argv) {
  regex_t re;
  int error = regcomp(&re, options->pattern, REG_EXTENDED | options->i);

  if (error) {
    printf("ERROR");
    return;
  }

  for (int i = optind; i < argc; i++) {
    process_file(options, argv[i], &re, argc);
  }

  regfree(&re);
}

void print_match(regex_t *reg, char *line, char *path, struct Options *options,
                 int line_count, int file_count) {
  regmatch_t match;
  int offset = 0;
  int result;
  int match_found = 0;

  while ((result = regexec(reg, line + offset, 1, &match, 0)) == 0) {
    match_found = 1;
    if (options->o && !options->h) {
      printf("%s:", path);
    }
    if (options->o && options->n && file_count > 1) {
      printf("%d:", line_count);
    } else if (options->n) {
      printf("%d:", line_count);
    }

    int start_pos = offset + match.rm_so;
    int end_pos = offset + match.rm_eo;

    for (int i = start_pos; i < end_pos; i++) {
      putchar(line[i]);
    }

    putchar('\n');
    offset += match.rm_eo;
  }

  if (!match_found && options->v) {
    if (!options->h) {
      printf("%s:", path);
    }
    if (options->n) {
      printf("%d:", line_count);
    }
    printf("%s\n", line + offset);
  }

  if (result != REG_NOMATCH && result != 0) {
    char error_buffer[100];
    regerror(result, reg, error_buffer, sizeof(error_buffer));
    fprintf(stderr, "Regex match failed: %s\n", error_buffer);
  }
}

void process_file(struct Options *options, char *path, regex_t *reg, int argc) {
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    if (!options->s) {
      perror(path);
    }
    return;
  }

  int file_count = argc - optind;
  char *line = NULL;
  size_t memlen = 0;
  int read = 0;
  int line_count = 1;
  int count_same = 0;
  read = getline(&line, &memlen, f);

  while (read != -1) {
    if (line[read - 1] == '\n') {
      line[read - 1] = '\0';
      read--;
    }

    int result = regexec(reg, line, 0, NULL, 0);
    if ((result == 0 && !options->v) || (result != 0 && options->v)) {
      if (!options->c && !options->l) {
        if (options->o) {
          print_match(reg, line, path, options, line_count, file_count);
        } else {
          if (!options->h) printf("%s:", path);
          if (options->n) printf("%d:", line_count);
          output_line(line, read);
        }
      }
      if (options->c || options->l) {
        count_same++;
      }
    }
    read = getline(&line, &memlen, f);
    line_count++;
  }

  if (options->c) {
    if (!options->h) {
      printf("%s:", path);
    }
    if (options->l && count_same > 0) count_same = 1;
    printf("%d\n", count_same);
  }
  if (options->l && count_same > 0) printf("%s\n", path);

  free(line);
  fclose(f);
}
