#include <arch/cpu.h>
#include <sys/syscalls.h>

static u64 handle_syscall(const struct sysclass_parameters *params __unused) {
    return 0;
}

void syscall_handler(struct registers_state *state) {
    struct sysclass_parameters params;
    parse_sysclass_parameters(state, &params);
    u64 result = handle_syscall(&params);
    set_syscall_result(result, state);
}
