#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 65536

static int cat(int in_cnt, int *in_fds) {
    char *buffer = malloc(BUF_SIZE);
    if (buffer == NULL) return -1;

    for (int i = 0; i < in_cnt; ++i) {
        int fd = in_fds[i];

        for (;;) {
            ssize_t res = read(fd, buffer, BUF_SIZE);
            if (res < 0) {
                perror("read");
                free(buffer);
                return -1;
            }
            if (res == 0) break;
            fwrite(buffer, 1, res, stdout);
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    int in_cnt;
    int *in_fds;
    int rc = EXIT_SUCCESS;

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "u")) != -1) {
        switch (c) {
        case 'u': /* ignored */
            break;
        case '?':
            fprintf(stderr, "Unknown option '-%c'\n", optopt);
            return EXIT_FAILURE;
        }
    }

    in_cnt = argc - optind;
    if (in_cnt) {
        in_fds = malloc(sizeof(int) * in_cnt);
        if (in_fds == NULL) {
            perror("malloc");
            return EXIT_FAILURE;
        }
        for (int idx = optind, i = 0; idx < argc; ++idx) {
            const char *str = argv[idx];
            int fd = strcmp(str, "-") == 0 ? STDIN_FILENO : open(str, O_RDONLY);
            if (fd == -1) {
                perror("open");
                rc = EXIT_FAILURE;
                goto free_files;
            }
            in_fds[i++] = fd;
        }
    } else {
        static int default_in[1] = {STDIN_FILENO};
        in_cnt = 1;
        in_fds = default_in;
    }

    if (cat(in_cnt, in_fds)) rc = EXIT_FAILURE;
free_files:
    for (int i = 0; i < in_cnt; ++i) {
        int fd = in_fds[i];
        if (fd != STDIN_FILENO) {
            if (close(fd)) {
                perror("close");
                rc = EXIT_FAILURE;
            }
        }
    }
    free(in_fds);

    return rc;
}
