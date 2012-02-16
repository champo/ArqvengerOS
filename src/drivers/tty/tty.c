#include "drivers/tty/tty.h"
#include "drivers/tty/status.h"
#include "shell/shell.h"
#include "type.h"
#include "system/process/table.h"
#include "system/scheduler.h"
#include "system/fs/fs.h"
#include "shell/login.h"
#include "library/stdio.h"
#include "system/accessControlList/users.h"

static int activeTerminal = 0;

static struct Terminal terminals[NUM_TERMINALS];

static size_t op_write(struct FileDescriptor* fd, const void* buffer, size_t len);

static size_t op_read(struct FileDescriptor* fd, void* buffer, size_t len);

static int op_ioctl(struct FileDescriptor* fd, int cmd, void* argp);

void tty_early_init(void) {
    tty_screen_init();

    terminals[activeTerminal].active = 1;
    tty_write("\033[1;1H\033[2J", 10);
}

void tty_run(char* unused) {

    tty_keyboard_init();

    fs_register_ops(INODE_CHARDEV, (struct FileDescriptorOps) {
            .open = NULL,
            .write = op_write,
            .read = op_read,
            .ioctl = op_ioctl,
            .readdir = NULL,
            .close = NULL
    });

    struct fs_Inode* root = fs_root();
    if (root == NULL) {
        panic();
    }

    int res = fs_mknod(root, "tty", INODE_CHARDEV);
    if (res != 0 && res != EEXIST) {
        panic();
    }

    open("/tty", O_RDONLY);
    open("/tty", O_WRONLY);
    open("/tty", O_WRONLY);

    int fd;
    FILE* fp;
    if ((fd = open("/users", O_RDONLY)) == -1) {

        fp = fopen("/users", "w");
        fprintf(fp, "root:x:0:0:root:root\n");
        fprintf(fp, "acrespo:x:5:1:alv:users\n");

        fclose(fp);
    } else {
        close(fd);
    }


    if ((fd = open("/groups", O_RDONLY)) == -1) {
        fp = fopen("/groups", "w");
        fprintf(fp, "root:x:0:root\n");
        fprintf(fp, "users:x:1:acrespo\n");

        fclose(fp);
    } else {
        close(fd);
    }

    // Spawn the shells (this is a kernel process, so we can do this)
    for (int i = 0; i < NUM_TERMINALS - 1; i++) {
        terminals[i].termios.canon = 1;
        terminals[i].termios.echo = 1;
        process_table_new(login, "login", scheduler_current(), 0, i, 1);
    }

    terminals[NUM_TERMINALS - 1].termios.canon = 1;
    terminals[NUM_TERMINALS - 1].termios.echo = 0;

    while (1) {
        process_scancode();
    }
}

void tty_change(int active) {

    terminals[activeTerminal].active = 0;
    activeTerminal = active;
    terminals[activeTerminal].active = 1;

    tty_screen_change();
}

struct Terminal* tty_current(void) {

    struct Process* caller = scheduler_current();

    if (caller == NULL || caller->terminal == NO_TERMINAL) {
        return tty_terminal(0);
    } else {
        return tty_terminal(caller->terminal);
    }
}

struct Terminal* tty_active(void) {
    return &terminals[activeTerminal];
}

struct Terminal* tty_terminal(int number) {
    if (number == NO_TERMINAL) {
        return tty_active();
    }

    return &terminals[number];
}

size_t op_write(struct FileDescriptor* fd, const void* buffer, size_t len) {
    return tty_write(buffer, len);
}

size_t op_read(struct FileDescriptor* fd, void* buffer, size_t len) {
    return tty_read(buffer, len);
}

int op_ioctl(struct FileDescriptor* fd, int cmd, void* argp) {
    return ioctlKeyboard(cmd, argp);
}

