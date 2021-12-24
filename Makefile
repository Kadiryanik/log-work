CC=gcc
OBJDIR=obj
EXEC_NAME=log-work

# open all warning
CFLAGS = -Wall

# include directories
CFLAGS += -I. -Iinclude

# library directories
LIB_DIRS = . src/

# set source files and object files under LIB_DIRS
SOURCE_FILES = ${foreach d, $(LIB_DIRS), ${subst ${d}/,,${wildcard $(d)/*.c}}}
OBJ_FILES := $(SOURCE_FILES:%.c=$(OBJDIR)/%.o)

#TODO: detect header file changes

# add src directories to path for building object files
vpath %.c $(LIB_DIRS)

# for tracing compilation
TRACE_CC = @echo "  CC       " $<
Q=@

# rules
build: $(OBJDIR) $(OBJ_FILES)
	$(Q)echo " LINK"
	$(CC) -o $(EXEC_NAME) $(OBJ_FILES)
$(OBJDIR)/%.o: %.c
	$(TRACE_CC)
	$(Q)$(CC) $(CFLAGS) -c $< -o $@
$(OBJDIR):
	$(Q)mkdir -p $(OBJDIR)
clean:
	rm -rf $(OBJDIR)
	rm -f $(EXEC_NAME)
	rm -f *.bin
