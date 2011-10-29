#include "drivers/tty/tty.h"
#include "drivers/tty/status.h"
#include "shell/shell.h"
#include "type.h"
#include "system/process/table.h"
#include "system/scheduler.h"
#include "system/fs/fs.h"

#include "library/stdio.h"
#include "system/accessControlList/users.h"

static int activeTerminal = 0;

static struct Terminal terminals[NUM_TERMINALS];

static size_t op_write(struct FileDescriptor* fd, const void* buffer, size_t len);

static size_t op_read(struct FileDescriptor* fd, void* buffer, size_t len);

static int op_ioctl(struct FileDescriptor* fd, int cmd, void* argp);

void tty_run(char* unused) {

    tty_screen_init();
    tty_keyboard_init();

    activeTerminal = 0;
    terminals[activeTerminal].active = 1;

    tty_write("\033[1;1H\033[2J", 10);

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
   
    int ias = unlink("/users"); 
    printf("%d\n",ias);
    FILE* fp = fopen("/users", "w");
    fprintf(fp, "root:x:0:4:root:root\n");
    fprintf(fp, "root1:x:1:5:root:root\n");
    fprintf(fp, "root2:x:2:6:root:root\n");
    fprintf(fp, "root3:x:3:7:root:root\n");
    fprintf(fp, "root4:x:4:7:root:root\n");

    fclose(fp);
    fp = fopen("/groups", "w");
    fprintf(fp, "root:x:0:root\n");
    fprintf(fp, "users:x:1:\n");
    fclose(fp);

    get_users_num();

    // Spawn the shells (this is a kernel process, so we can do this)
    for (int i = 0; i < NUM_TERMINALS; i++) {
        terminals[i].termios.canon = 1;
        terminals[i].termios.echo = 1;
        process_table_new(shell, NULL, scheduler_current(), 0, i, 1);
    }

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

