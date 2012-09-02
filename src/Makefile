ifndef SCHEDULER
SCHEDULER=priorityScheduler
endif

SRCDIR=../src
OBJDIR=../bin
TARGET=$(OBJDIR)/kernel.bin
FOLDERS=. $(CHILD_FOLDERS)
CHILD_FOLDERS=$(shell find . -mindepth 1 -type d)
ALL_OBJS=$(addprefix $(OBJDIR)/,$(foreach dir,$(FOLDERS),$(patsubst %.c, %.o, $(wildcard $(dir)/*.c)) $(patsubst %.asm, %-asm.o, $(wildcard $(dir)/*.asm))))
INCLUDES=$(addprefix $(SRCDIR)/,$(foreach dir,$(FOLDERS), $(wildcard $(dir)/*.h)))

OBJS=$(filter-out $(OBJDIR)/./system/scheduler/%.o, $(ALL_OBJS)) $(OBJDIR)/./system/scheduler/$(SCHEDULER).o


include common.mk
CFLAGS=-fno-builtin -I$(SRCDIR) -pedantic -std=c99 -fstrict-aliasing -Wall -Wextra -Wshadow -Wcast-qual \
	-Wwrite-strings -Wpointer-arith -Wcast-align -Wmissing-prototypes \
	-Wmissing-declarations -Wredundant-decls -Wnested-externs -Winline \
	-Wstrict-prototypes -Wunreachable-code -fno-builtin -nostdlib \
	-nostartfiles -nodefaultlibs -m32 -include $(SRCDIR)/unused.h
LDFLAGS=-T $(SRCDIR)/link.ld
NASMFLAGS=

.SUFFIXES:
.SUFFIXES: .c .o .asm .h

.PHONY: debug release clean all prepare

.DEFAULT: $(TARGET)

# Rule for ASM
$(OBJDIR)/%-asm.o: $(SRCDIR)/%.asm
	nasm $(NASMFLAGS) -f aout $< -o $@
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@
%.c:

$(TARGET): prepare $(OBJS) $(INCLUDES)
# The wire, and filter-out is a hack to make sure the loader is at the start of the endfile, so it can load!
	$(LD) $(LDFLAGS) -o $@ $(OBJDIR)/./system/loader-asm.o $(filter-out $(OBJDIR)/./system/loader-asm.o, $(OBJS))
	mcopy -o $@ b:boot/

all: release

# Debug flags
debug: override CFLAGS += -O0 -g
debug: override NASMFLAGS += -O0 -g
debug: $(TARGET)

# Release flags
release: override CFLAGS += -O3
release: $(TARGET)

clean: 
	-rm $(OBJS)
	-cd $(OBJDIR) && rm -rf $(CHILD_FOLDERS)
	-rm $(TARGET)

prepare:
	cd $(OBJDIR) && mkdir -p $(CHILD_FOLDERS)
