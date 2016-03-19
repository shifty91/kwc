#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

struct file_result {
    const char *file_name;
    size_t nlines;
    size_t nwords;
    size_t nbytes;
};

struct options {
    int lines;
    int words;
    int bytes;
    int parseable;
};

/**
 * Print usage and die.
 */
static void
print_usage_and_die(void)
{
    fprintf(stderr, "kwc [options] [files]\n");
    fprintf(stderr, "options:\n");
    fprintf(stderr, "  -l: count lines\n");
    fprintf(stderr, "  -w: count words\n");
    fprintf(stderr, "  -b: count bytes\n");
    fprintf(stderr, "  -p: parseable output for use in scripts\n");
    fprintf(stderr, "By default all options are enabled. If no file is specified, stdin is used.\n");
    fprintf(stderr, "(C) Kurt Kanzenbach <kurt@kmk-computers.de>\n");
    exit(EXIT_FAILURE);
}

/**
 * Shortcut: If just -b and a file is specified, we can simply use lstat(2).
 *
 * @param file file to check
 *
 * @return file size
 */
static size_t
get_file_size(const char * const file)
{
    struct stat stat;

    if (!file) {
        fprintf(stderr, "NULL pointer(s) passed to '%s' function. Fix the code.\n",
                __func__);
        exit(EXIT_FAILURE);
    }

    if (lstat(file, &stat)) {
        perror("lstat() failed");
        exit(EXIT_FAILURE);
    }

    return (size_t)stat.st_size;
}

/**
 * Count: Reads the file until EOF and counts lines/words/bytes.
 *
 * @param file   file to use (might be stdin)
 * @param result counting results will be stored in result
 * @param opt    options
 */
static void
count(FILE *file, struct file_result *result, const struct options * const opt)
{
    int prev, c, linesonly;

    /* args */
    if (!file || !result || !opt) {
        fprintf(stderr, "NULL pointer(s) passed to '%s' function. Fix the code.\n",
                __func__);
        exit(EXIT_FAILURE);
    }

    /* options */
    linesonly = !opt->words && opt->lines;

    /* count */
    prev = EOF;
    while ((c = fgetc(file)) != EOF) {
        if (c == '\n')
            result->nlines++;

        /* if lines only -> skip words (compiler should do unswitching here) */
        if (linesonly)
            continue;

        if (isspace(c) && !isspace(prev))
            result->nwords++;

        result->nbytes++;
        prev = c;
    }
}

/**
 * Print the result depending on the options used.
 *
 * @param result result
 * @param opt    options
 */
static void
print_stats(const struct file_result * const result, const struct options * const opt)
{
    if (!result || !opt) {
        fprintf(stderr, "NULL pointer(s) passed to '%s' function. Fix the code.\n",
                __func__);
        exit(EXIT_FAILURE);
    }

    if (opt->parseable) {
        printf("%s;%zu;%zu;%zu", result->file_name, result->nlines,
               result->nwords, result->nbytes);
        goto out;
    }

    printf("file: %s: ", result->file_name);
    if (opt->lines)
        printf("lines: %zu ", result->nlines);
    if (opt->words)
        printf("words: %zu ", result->nbytes);
    if (opt->bytes)
        printf("bytes: %zu", result->nbytes);

out:
    printf("\n");
    fflush(stdout);
}

/**
 * Counts words/lines/bytes in the current file.
 *
 * @param f    file to count
 * @param path path of file to count
 * @param opt  options
 */
static void
do_file(FILE *f, const char * const path, const struct options * const opt)
{
    struct file_result result;

    if (!f || !path || !opt) {
        fprintf(stderr, "NULL pointer(s) passed to '%s' function. Fix the code.\n",
                __func__);
        exit(EXIT_FAILURE);
    }

    memset(&result, '\0', sizeof(result));
    result.file_name = path;

    /**
     * Shortcut: If no words, we can simply count lines and
     *           get the file size via lstat(2);
     */
    if (!strcmp(path, "stdin") && !opt->words) {
        result.nbytes = get_file_size(path);

        /* just bytes -> quit */
        if (!opt->lines) {
            print_stats(&result, opt);
            return;
        }
    }

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
dispatcher(int argc, char **argv, const struct options * const opt)
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
            exit(EXIT_FAILURE);
        }

        do_file(f, argv[i], opt);

        fclose(f);
    }
}

int main(int argc, char *argv[])
{
    struct options opt;
    int c;

    /* args */
    memset(&opt, '\0', sizeof(opt));
    while ((c = getopt(argc, argv, "lwbp")) != -1) {
        switch (c) {
        case 'l':
            opt.lines = 1;
            break;
        case 'w':
            opt.words = 1;
            break;
        case 'b':
            opt.bytes = 1;
            break;
        case 'p':
            opt.parseable = 1;
            break;
        case '?':
        default:
            print_usage_and_die();
        }
    }
    if (!opt.lines && !opt.words && !opt.bytes) {
        opt.lines = 1;
        opt.words = 1;
        opt.bytes = 1;
    }

    argc -= optind;
    argv += optind;

    /* do it */
    dispatcher(argc, argv, &opt);

    return EXIT_SUCCESS;
}
