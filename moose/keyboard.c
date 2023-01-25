#include "keyboard.h"

#include "arch/ports.h"
#include "interrupt.h"
#include "kprint.h"

#define SC_MAX 57
#define BACKSPACE 0x0e
#define ENTER 0x1c

const char *sc_name[] = {
    "ERROR",     "Esc",     "1", "2", "3", "4",      "5",
    "6",         "7",       "8", "9", "0", "-",      "=",
    "Backspace", "Tab",     "Q", "W", "E", "R",      "T",
    "Y",         "U",       "I", "O", "P", "[",      "]",
    "Enter",     "Lctrl",   "A", "S", "D", "F",      "G",
    "H",         "J",       "K", "L", ";", "'",      "`",
    "LShift",    "\\",      "Z", "X", "C", "V",      "B",
    "N",         "M",       ",", ".", "/", "RShift", "Keypad *",
    "LAlt",      "Spacebar"};

static const char sc_ascii[] = {
    '?', '?', '1', '2', '3', '4', '5', '6', '7', '8', '9',  '0', '-', '=',  '?',
    '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',  '[', ']', '?',  '?',
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z',
    'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', '?', '?',  '?', ' '};

static void keyboard_callback(struct isr_regs *regs) {
    (void)regs;
    u8 scancode = port_u8_in(0x60);
    if (scancode > SC_MAX)
        return;
    if (scancode == BACKSPACE) {
        /* if (backspace(key_buffer)) { */
        /*     print_backspace(); */
        /* } */
    } else if (scancode == ENTER) {
        /* print_nl(); */
        /* execute_command(key_buffer); */
        /* key_buffer[0] = '\0'; */
    } else {
        int letter = sc_ascii[scancode];
        kprintf("pressed %c\n", letter);
    }
}

void init_keyboard(void) { register_irq_handler(32, keyboard_callback); }
