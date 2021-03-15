CC := clang

# Sources and flags needed to compile static library
SRC := lib/mem.h lib/dw_fs.h lib/dw_fs.c
CFLAGS := -c

# Sources and flags to compile for unit testing
TESTSRC := lib/dw_fs_test.c
TESTFLAGS := -lcheck

.PHONY: all
all: test clean-test dw-fs

.PHONY: clean
clean:
	rm bin/dw-fs
	rm -rf bin/debug

dw-fs: $(SRC)
	$(CC) $(CFLAGS) -o bin/$@ $(filter %.c,$^)

test: $(SRC) $(TESTSRC)
	$(CC) $(TESTFLAGS) -o $@ $(filter %.c,$^)
	./$@
	rm $@

# Compile debugging target
debug: $(SRC)
	$(CC) -g -o bin/debug/dw-fs $(filter %.c,$^) main.c
