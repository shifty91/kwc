#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wchar.h>
#include <wctype.h>
#include <getopt.h>

static struct option long_options[] = {
    { "lines",     no_argument, NULL, 'l' },
    { "words",     no_argument, NULL, 'w' },
    { "chars",     no_argument, NULL, 'c' },
    { "parseable", no_argument, NULL, 'p' },
    { "help",      no_argument, NULL, 'h' },
    { "version",   no_argument, NULL, 'v' },
    { NULL,        0,           NULL,  0  }
};

struct file_result {
    const char *file_name;
    size_t nlines;
    size_t nwords;
    size_t nchars;
};

struct options {
    int lines;
    int words;
    int chars;
    int parseable;
};

static struct file_result global_count;

static void
print_version_and_die(void)
{
    fprintf(stderr, "kwc version %s -- wordcount utility\n", VERSION);
    fprintf(stderr, "Copyright (C) 2016 Kurt Kanzenbach <kurt@kmk-computers.de>\n");
    exit(0);
}

/**
 * Print usage and die.
 *
 * @param ret return value for exit
 */
static void
print_usage_and_die(int ret)
{
    fprintf(stderr, "kwc [options] [files]\n");
    fprintf(stderr, "options:\n");
    fprintf(stderr, "  --lines, -l:     count lines\n");
    fprintf(stderr, "  --words, -w:     count words\n");
    fprintf(stderr, "  --chars, -c:     count character\n");
    fprintf(stderr, "  --parseable, -p: parseable output for use in scripts\n");
    fprintf(stderr, "  --help, -h:      print this help text\n");
    fprintf(stderr, "  --version, -v:   print version imformation\n");
    fprintf(stderr, "By default all options are enabled. If no file is specified, stdin is used.\n");
    fprintf(stderr, "(C) Kurt Kanzenbach <kurt@kmk-computers.de>\n");
    exit(ret);
}

/**
 * Count: Reads the file until EOF and counts lines/words/characters.
 *
 * @param file   file to use (might be stdin)
 * @param result counting results will be stored in result
 * @param opt    options
 */
static void
count(FILE *file, struct file_result *result, const struct options *const opt)
{
    wint_t prev, c;
    int no_words;

    /* args */
    if (!file || !result || !opt) {
        fprintf(stderr, "NULL pointer(s) passed to '%s' function. Fix the code.\n",
                __func__);
        exit(EXIT_FAILURE);
    }

    /* options */
    no_words = !opt->words;

    /* count */
    prev = WEOF;
    while ((c = fgetwc(file)) != WEOF) {
        result->nchars++;

        if (c == L'\n')
            result->nlines++;

        /* if no words -> skip counting (compiler should do unswitching here) */
        if (no_words)
            continue;

        if (iswspace(c) && !iswspace(prev))
            result->nwords++;

        prev = c;
    }
    if (ferror(file)) {
        perror("fgetwc() failed");
        exit(EXIT_FAILURE);
    }
    /* last line */
    if (!no_words && !iswspace(prev))
        result->nwords++;

    /* add to global counter */
    global_count.nlines += result->nlines;
    global_count.nwords += result->nwords;
    global_count.nchars += result->nchars;
}

/**
 * Print the result depending on the options used.
 *
 * @param result result
 * @param opt    options
 */
static void
print_stats(const struct file_result * const result, const struct options *const opt)
{
    if (!result || !opt) {
        fprintf(stderr, "NULL pointer(s) passed to '%s' function. Fix the code.\n",
                __func__);
        exit(EXIT_FAILURE);
    }

    if (opt->parseable) {
        printf("%s;%zu;%zu;%zu", result->file_name, result->nlines,
               result->nwords, result->nchars);
        goto out;
    }

    printf("file: %s: ", result->file_name);
    if (opt->lines)
        printf("lines: %zu ", result->nlines);
    if (opt->words)
        printf("words: %zu ", result->nwords);
    if (opt->chars)
        printf("characters: %zu", result->nchars);

out:
    printf("\n");
    fflush(stdout);
}

/**
 * Counts words/lines/characters in the current file.
 *
 * @param f    file to count
 * @param path path of file to count
 * @param opt  options
 */
static void
do_file(FILE *f, const char * const path, const struct options *const opt)
{
    struct file_result result = { 0 };

    if (!f || !path || !opt) {
        fprintf(stderr, "NULL pointer(s) passed to '%s' function. Fix the code.\n",
                __func__);
        exit(EXIT_FAILURE);
    }

    result.file_name = path;

    /* count */
    count(f, &result, opt);

    print_stats(&result, opt);
}

/**
 * This function captures the logic.
 *
 * @param argc files
 * @param argv files
 * @param opt  options to be used
 */
static void
dispatcher(int argc, char **argv, const struct options *const opt)
{
    if (!argv || !opt) {
        fprintf(stderr, "NULL pointer(s) passed to '%s' function. Fix the code.\n",
                __func__);
        exit(EXIT_FAILURE);
    }
    /* stdin */
    if (argc == 0) {
        do_file(stdin, "stdin", opt);
        return;
    }

    /* files */
    for (int i = 0; i < argc; ++i) {
        FILE *f = fopen(argv[i], "r");
        if (!f) {
            perror("fopen failed");
            continue;
        }

        do_file(f, argv[i], opt);

        fclose(f);
    }

    /* global count */
    if (argc > 1) {
        global_count.file_name = "global";
        print_stats(&global_count, opt);
    }
}

int main(int argc, char *argv[])
{
    struct options opt = { 0 };
    int c;

    /* args */
    while ((c = getopt_long(argc, argv, "lwcphv", long_options, NULL)) != -1) {
        switch (c) {
        case 'l':
            opt.lines = 1;
            break;
        case 'w':
            opt.words = 1;
            break;
        case 'c':
            opt.chars = 1;
            break;
        case 'p':
            opt.parseable = 1;
            break;
        case 'h':
            print_usage_and_die(0);
        case 'v':
            print_version_and_die();
        case '?':
        default:
            print_usage_and_die(1);
        }
    }
    if (!opt.lines && !opt.words && !opt.chars) {
        opt.lines = 1;
        opt.words = 1;
        opt.chars = 1;
    }

    argc -= optind;
    argv += optind;

    if (!setlocale(LC_ALL, "")) {
        fprintf(stderr, "setlocale() failed\n");
        return EXIT_FAILURE;
    }

    /* do it */
    dispatcher(argc, argv, &opt);

    return EXIT_SUCCESS;
}
