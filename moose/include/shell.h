#pragma once

#include <fs/fat.h>
#include <types.h>

struct shell {
    struct pfatfs fs;
    struct device *dev;
    char cwd[256];
    size_t cwd_length;
};

void shell_execute_command(struct shell *shell, const char *cmd);
