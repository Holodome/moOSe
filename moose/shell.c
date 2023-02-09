#include <shell.h>

enum shell_command {
    CMD_LS,
    CMD_PWD,
    CMD_ECHO,
    CMD_TIME,
    CMD_CAT,
    CMD_TOUCH,
    CMD_HELP,
};

struct shell {
    struct pfatfs fs;
    char cwd[256];
    size_t cwd_length;
};

void shell_execute_command(struct shell *shell, const char *cmd);
