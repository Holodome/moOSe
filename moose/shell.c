#include <arch/jiffies.h>
#include <drivers/disk.h>
#include <fs/fat.h>
#include <kstdio.h>
#include <shell.h>
#include <string.h>

enum shell_command {
    CMD_LS,
    CMD_TIME,
    CMD_CAT,
    CMD_TOUCH,
    CMD_HELP,
    CMD_RM,
    CMD_MD,
    CMD_REN,
    CMD_TREE,
    CMD_WRITE
};

struct cmd {
    enum shell_command cmd;
    char a[32];
    char b[32];
};

static struct {
    struct fatfs fs;
} shell = {};

static int parse_command_name(const char *name, size_t name_length,
                              enum shell_command *cmd) {
    static const char *names[] = {"LS", "TIME", "CAT", "TOUCH", "HELP",
                                  "RM", "MD",   "REN", "TREE",  "WRITE"};
    for (size_t i = 0; i < ARRAY_SIZE(names); ++i) {
        const char *opt = names[i];
        size_t opt_len = strlen(opt);
        if (opt_len == name_length && memcmp(name, opt, opt_len) == 0) {
            *cmd = i;
            return 0;
        }
    }

    return -1;
}

static int parse_cmd(const char *str, struct cmd *cmd) {
    static const char *spaces = " \t\r\n";
    size_t first_word_length = strcspn(str, spaces);
    if (!first_word_length) {
        kprintf("invalid command format\n");
        return -1;
    }

    if (parse_command_name(str, first_word_length, &cmd->cmd)) {
        kprintf("undefined command %.*s\n", (int)first_word_length, str);
        return -1;
    }

    str += first_word_length;
    str += strspn(str, spaces);

    switch (cmd->cmd) {
    case CMD_TIME:
    case CMD_HELP:
        break;
    case CMD_LS:
    case CMD_TREE:
    case CMD_CAT:
    case CMD_TOUCH:
    case CMD_RM:
    case CMD_MD: {
        size_t length = strcspn(str, spaces);
        if (!length) {
            kprintf("1 argument expected\n");
            return -1;
        }

        strlcpy(cmd->a, str, sizeof(cmd->a));
    } break;
    case CMD_WRITE:
    case CMD_REN: {
        size_t length = strcspn(str, spaces);
        if (!length) {
            kprintf("2 arguments expected\n");
            return -1;
        }

        if (length >= sizeof(cmd->a)) {
            kprintf("too long string\n");
        }
        memcpy(cmd->a, str, length);
        cmd->a[length] = 0;
        str += length;
        str += strspn(str, spaces);
        length = strcspn(str, spaces);
        if (!length) {
            kprintf("2 arguments expected\n");
            return -1;
        }

        if (length >= sizeof(cmd->b)) {
            kprintf("too long string\n");
        }
        memcpy(cmd->b, str, length);
        cmd->b[length] = 0;
    } break;
    }

    return 0;
}

static void do_cat(const char *filename) {
    struct fatfs_file file = {0};
    int result = fatfs_open(&shell.fs, filename, &file);
    if (result != 0) {
        kprintf("Open err\n");
        return;
    }

    char buffer[512];
    for (size_t i = 0; i < (file.size + sizeof(buffer) - 1) / sizeof(buffer);
         ++i) {
        ssize_t result = fatfs_read(&shell.fs, &file, buffer, sizeof(buffer));
        if (result < 0) {
            kprintf("Read err\n");
            return;
        }

        kprintf("%.*s", (int)result, buffer);
    }
    kprintf("\n");
}

static void do_ls(const char *filename) {
    struct fatfs_file file = {0};
    int result = fatfs_open(&shell.fs, filename, &file);
    if (result != 0) {
        kprintf("Open err\n");
        return;
    }

    if (file.type != FATFS_FILE_DIR) {
        kprintf("Opened file is not a dir\n");
        return;
    }

    size_t i = 0;
    for (;; ++i) {
        struct fatfs_file new_file = {0};
        result = fatfs_readdir(&shell.fs, &file, &new_file);
        if (result == -ENOENT) {
            break;
        }
        if (result < 0) {
            kprintf("Read err\n");
            return;
        }
        kprintf("%zu: %11s %c %ub\n", i, (char *)new_file.name,
                new_file.type == FATFS_FILE_DIR ? 'D' : 'F', new_file.size);
    }
}

static void do_mkdir(const char *filename) {
    int result = fatfs_mkdir(&shell.fs, filename);
    if (result != 0) {
        kprintf("Mkdir err\n");
    }
}

static void do_rm(const char *filename) {
    int result = fatfs_remove(&shell.fs, filename);
    if (result != 0) {
        kprintf("Rm err\n");
    }
}

static void do_create(const char *filename) {
    struct fatfs_file file;
    int result = fatfs_create(&shell.fs, filename, NULL, &file);
    if (result < 0) {
        kprintf("Create err\n");
    }
}

static void do_rename(const char *a, const char *b) {
    int result = fatfs_rename(&shell.fs, a, b);
    if (result < 0) {
        kprintf("Rename err\n");
    }
}

static void do_write(const char *str, const char *filename) {
    struct fatfs_file file = {0};
    int result = fatfs_open(&shell.fs, filename, &file);
    if (result != 0) {
        kprintf("Open err\n");
        return;
    }

    if (file.type == FATFS_FILE_DIR) {
        kprintf("Opened file is a dir\n");
        return;
    }

    if (fatfs_seek(&shell.fs, &file, 0, SEEK_END) != 0) {
        kprintf("seek failed\n");
        return;
    }

    ssize_t write_result = fatfs_write(&shell.fs, &file, str, strlen(str));
    if (write_result < 0) {
        kprintf("write failed\n");
    }
}

static void do_tree(const char *path) {
    struct fatfs_file stack[16];
    size_t stack_size = 1;
    int result = fatfs_open(&shell.fs, path, stack);
    if (result != 0) {
        kprintf("Open err\n");
        return;
    }
    kprintf("/\n");

    while (stack_size) {
        struct fatfs_file *file = stack + stack_size - 1;
        result = fatfs_readdir(&shell.fs, file, stack + stack_size);
        if (result == -ENOENT) {
            --stack_size;
        } else if (result < 0) {
            kprintf("Readdir err\n");
            break;
        } else {
            for (size_t i = 0; i < stack_size * 2; ++i)
                kputc(' ');
            kprintf("%11s\n", file[1].name);

            if (file[1].type == FATFS_FILE_DIR)
                ++stack_size;
        }
    }
}

static void execute(const struct cmd *cmd) {
    switch (cmd->cmd) {
    case CMD_TIME:
        kprintf("time: %lus\n",
                (unsigned long)jiffies64_to_msecs(get_jiffies64()));
        break;
    case CMD_TREE:
        do_tree(cmd->a);
        break;
    case CMD_WRITE:
        do_write(cmd->a, cmd->b);
        break;
    case CMD_HELP:
        kprintf("HELP         - display this message\n"
                "LS <PATH>    - list files\n"
                "TIME         - print time\n"
                "CAT <PATH>   - print file\n");
        kprintf("TOUCH <PATH> - create file\n"
                "RM <PATH>    - delete file or directory\n"
                "MD <PATH>    - create directory\n"
                "REN <A> <B>  - rename\n"
                "TREE <PATH>  - recursive ls\n");
        break;
    case CMD_CAT:
        do_cat(cmd->a);
        break;
    case CMD_LS:
        do_ls(cmd->a);
        break;
    case CMD_MD:
        do_mkdir(cmd->a);
        break;
    case CMD_RM:
        do_rm(cmd->a);
        break;
    case CMD_TOUCH:
        do_create(cmd->a);
        break;
    case CMD_REN:
        do_rename(cmd->a, cmd->b);
        break;
    }
}

int init_shell(void) {
    shell.fs.dev = disk_part_dev;
    int result = fatfs_mount(&shell.fs);
    if (result)
        return -1;

    return result;
}

void shell_execute_command(const char *cmd) {
    struct cmd command;
    if (parse_cmd(cmd, &command))
        return;

    execute(&command);
}
