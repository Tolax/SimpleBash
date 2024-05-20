#include "s21_grep.h"
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void grep(int argc, char *argv[]);
void output_line(char* line, int n);
void process_file(struct Options *options, char *path, regex_t* reg);
void add_reg_from_file(struct Options *options, char* filepath);
void output(struct Options *options, int argc, char** argv);

void pattern_add(struct Options *options, char* pattern){
    int n = strlen(pattern);

    if(options->pattern == 0){
        options->pattern = malloc(1024 * sizeof(char));
        options->mem_pattern = 1024;
    }
    if(options->mem_pattern < options->len_pattern + n){
        options->pattern = realloc(options->pattern, options->mem_pattern * 2 * sizeof(char));
        options->mem_pattern *= 2;
    }
    if(options->len_pattern != 0){
        strcat(options->pattern + options->len_pattern, "|");
        options->len_pattern++;
    }
    options->len_pattern += sprintf(options->pattern + options->len_pattern, "(%s)", pattern);

}


int main(int argc, char *argv[]) {
    struct Options Options = {0};
    flag_parser(argc, argv, &Options);
    output(&Options, argc, argv);
    free(Options.pattern);
    return 0;
}

void add_reg_from_file(struct Options *options, char* filepath){
    FILE *f = fopen(filepath, "r");
    if(f == NULL){
        if(!options->s){
            perror(filepath);
            exit(1);
        }
        return;
    }
    char* line = NULL;
    size_t memlen = 0;
    int read = getline(&line, &memlen, f);

    while(read != -1){
        if(line[read-1] == '\n'){
            line[read-1] = '\0';
        }
        pattern_add(options, line);
        read = getline(&line, &memlen, f);
    }
    free(line);
    fclose(f);
}


void flag_parser(int argc, char *argv[], struct Options *Options) {
    int opt;
    while ((opt = getopt(argc, argv, "e:ivclnhsf:o")) != -1) {
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
                Options->c = 1;
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
            default:
        }
    }
    if(Options->len_pattern == 0){
        pattern_add(Options, argv[optind]);
        optind++;
    }
    if(argc - optind == 1){
        Options->h = 1;
    }
}

void output_line(char* line, int n){
    for(int i=0; i<n; i++){
        putchar(line[i]);
    }
    if(line[n-1] != '\n') putchar('\n');
}

void output(struct Options *options, int argc, char** argv){
    regex_t re;
    int error = regcomp(&re, options->pattern, REG_EXTENDED | options->i);

    if(error){
        printf("ERROR");
    }
    for(int i = optind; i < argc; i++){
        process_file(options, argv[i], &re);
    }
}

void print_match(regex_t* reg, char* line, char *path, struct Options *options){
    regmatch_t match;
    int offset = 0;
    int result;
    while((result = regexec(reg, line + offset, 1, &match, 0)) == 0){
        if(options->o){
                options->h = 1;
                printf("%s:", path);
            }
        for(int i = match.rm_so; i < match.rm_eo; i++){
            
            putchar(line[offset + i]);
        }
        putchar('\n');
        offset += match.rm_eo;
    }
    // Если regexec вернул неудачу, и это не из-за отсутствия совпадений,
    // а из-за ошибки или завершения строки, выводим сообщение об ошибке
    if(result != REG_NOMATCH && result != 0){
        char error_buffer[100];
        regerror(result, reg, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "Regex match failed: %s\n", error_buffer);
    }
}


void process_file(struct Options *options, char *path, regex_t* reg) {
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        if (!options->s) {
            perror(path);
            exit(1);
        }
        return;
    }

    char* line = NULL;
    size_t memlen = 0;
    int read = 0;
    int line_count = 1;
    int count_same = 0;
    read = getline(&line, &memlen, f);

    // Check if the path is from a file list
    int is_from_file_list = options->f && strcmp(path, options->pattern) == 0;

    while (read != -1) {
        int result = regexec(reg, line, 0, NULL, 0);
        if ((result == 0 && !options->v) || (result != 0 && options->v)) {
            if (!options->c && !options->l) {
                if(options->o){
                    if(options->n) printf("%d:", line_count);
                    if(options->o){
                    print_match(reg, line,path, options);
                    }
                }else{
                    if (!options->h) printf("%s:", path); // Print filename if not from file list
                    if (options->n) printf("%d:", line_count);
                    output_line(line, read);
                }
                
            }
            count_same++;
        }
        read = getline(&line, &memlen, f);
        line_count++;
    }

    if (options->c && !options->l) {
        if (!options->h && !is_from_file_list) printf("%s:", path); // Print filename if not from file list
        printf("%d\n", count_same);
    }
    if (options->l && count_same > 0) printf("%s\n", path);

    free(line);
    fclose(f);
}