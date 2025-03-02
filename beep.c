#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

#define KEYBOARD_DEV "/dev/input/event3"

#define CONSOLE_DEV "/dev/console"

struct NoteMapping {
    unsigned int key_code;
    int frequency;
};

struct NoteMapping mappings[] = {
    { KEY_Z, 261 },      // C4
    { KEY_S, 277 },      // C#4
    { KEY_X, 293 },      // D4
    { KEY_D, 311 },      // D#4
    { KEY_C, 329 },      // E4
    { KEY_V, 349 },      // F4
    { KEY_G, 370 },      // F#4
    { KEY_B, 392 },      // G4
    { KEY_H, 415 },      // G#4
    { KEY_N, 440 },      // A4
    { KEY_J, 466 },      // A#4
    { KEY_M, 493 },      // B4
    { KEY_COMMA, 523 },  // C5
    { KEY_L, 554 },      // C#5
    { KEY_DOT, 587 },    // D5
    { KEY_SEMICOLON, 622 }, // D#5
    { KEY_SLASH, 659 },  // E5
    { KEY_Q, 523 },
    { KEY_2, 554 },      // C#5
    { KEY_W, 587 },      // D5
    { KEY_3, 622 },      // D#5
    { KEY_E, 659 },      // E5
    { KEY_R, 698 },      // F5
    { KEY_5, 740 },      // F#5
    { KEY_T, 784 },      // G5
    { KEY_6, 830 },      // G#5
    { KEY_Y, 880 },      // A5
    { KEY_7, 932 },      // A#5
    { KEY_U, 988 },      // B5
    { KEY_I, 1047 },     // C6
    { KEY_9, 1109 },     // C#6
    { KEY_O, 1175 },     // D6
    { KEY_0, 1245 },     // D#6
    { KEY_P, 1319 }      // E6
};

#define NUM_MAPPINGS (sizeof(mappings) / sizeof(mappings[0]))

int current_active = -1;
int console_fd = -1;

void beep_start(int frequency) {
    int duration = 0xffff;
    int div = 1193180 / frequency;
    int arg = div | (duration << 16);
    if (ioctl(console_fd, KDMKTONE, arg) < 0) {
        perror("ioctl(KDMKTONE)失敗");
    }
}
void beep_stop() {
    if (ioctl(console_fd, KDMKTONE, 0) < 0) {
        perror("ioctl(KDMKTONE, 0)失敗");
    }
}

int main(int argc, char *argv[]) {
    const char *kbd_dev = KEYBOARD_DEV;
    if (argc > 1) {
        kbd_dev = argv[1];
    }
    
    int kbd_fd = open(kbd_dev, O_RDONLY);
    if (kbd_fd < 0) {
        perror("キーボードデバイスオープン失敗");
        return 1;
    }
    
    console_fd = open(CONSOLE_DEV, O_WRONLY);
    if (console_fd < 0) {
        perror("コンソールデバイスオープン失敗");
        close(kbd_fd);
        return 1;
    }
    
    printf("=== BeepKeyboard ===\n");
    
    struct input_event ev;
    while (1) {
        ssize_t n = read(kbd_fd, &ev, sizeof(ev));
        if (n != sizeof(ev)) {
            perror("read失敗");
            continue;
        }
        
        if (ev.type == EV_KEY) {
            int freq = 0;
            for (int i = 0; i < NUM_MAPPINGS; i++) {
                if (ev.code == mappings[i].key_code) {
                    freq = mappings[i].frequency;
                    break;
                }
            }
            
            if (freq) {
                if (ev.value == 1) {
                    if (current_active != -1 && current_active != ev.code) {
                        beep_stop();
                    }
                    beep_start(freq);
                    current_active = ev.code;
                } else if (ev.value == 0) {
                    if (current_active == ev.code) {
                        beep_stop();
                        current_active = -1;
                    }
                }
            }
        }
    }
    
    close(kbd_fd);
    close(console_fd);
    return 0;
}
