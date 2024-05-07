#include "cat_bash.h"

int main(int argc, char *argv[]) {
    struct Options Options = {0};
    flag_parser(argc, argv, &Options);
    if(optind != -1){
        for(int i = optind; i<argc; i++) print_file(argv[i], &Options);

    }
             
    return 0;
}

void flag_parser(int argc, char *argv[], struct Options *Options) {
    int opt;
    struct option long_options[] = {
        {"number-nonblank", 0, 0, 'b'},
        {"number", 0, 0, 'n'},
        {"squeeze-blank", 0, 0, 's'},
    };

    while ((opt = getopt_long(argc, argv, "+benstvTE", long_options, NULL)) != -1) {
        switch (opt) {
            case 'b':
                Options->b = 1;
                break;
            case 'e':
                Options->e = 1;
                Options->v = 1;
                break;
            case 'E':
                Options->e = 1;
                break;
            case 'n':
                Options->n = 1;
                break;
            case 's':
                Options->s = 1;
                break;
            case 't':
                Options->t = 1;
                Options->v = 1;
                break;
            case 'T':
                Options->t = 1;
                break;
            case 'v':
                Options->v = 1;
                break;
            default:
        }
    }
    if(Options->b) Options->n = 0;
}

int print_file(char *filename, struct Options *options) {
    FILE *f = fopen(filename, "r");
    int ch;
    int ch_prev = 0;
    int line_number = 0;
    int empty_line_printed = 0;
    if (f == NULL) {
        perror("Error opening file");
        return 0;
    }

    while ((ch = fgetc(f)) != EOF) {
        if (!(options->s && ch_prev == '\n' && ch == '\n' && empty_line_printed)) {
            if (ch_prev == '\n' && ch == '\n') {
                empty_line_printed = 1;
            }else {
                empty_line_printed = 0;
            }

            if (((options->b && ch_prev == '\n') || (options->b && line_number == 0)) && ch != '\n') {
                    printf("%6d\t", ++line_number);
            }

            if(options->e && options->b){
                    if((ch == '\n' && ch_prev == '\n') || (ch =='\n' && line_number==0)){
                    printf("      \t");
                }
            }

            if(options->n) {
                if (ch_prev == 0 || ch_prev == '\n') {
                    printf("%6d\t", ++line_number);
                }
                ch_prev = ch;
            }

            if(options->e && ch == '\n') printf("$"); 

            if (options->t && ch == '\t') {
                printf("^");
                ch  += 64;
            }

            if (options->v) {
                if(ch <= 31 && ch != '\n' && ch !='\t'){
                    printf("^");
                    ch = ch + 64;
                }
                if(ch == 127){
                    printf("^");
                    ch = '?';
                }
                if(ch >= 128 && ch <= 160 && ch != '\n' && ch !='\t'){
                    printf("M-");
                    ch -= 64;
                }
            }

            putc(ch, stdout);
    }
        ch_prev = ch;
    }
    fclose(f);
    return 1;
}