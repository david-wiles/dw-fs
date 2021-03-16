CC := clang

# Sources and flags needed to compile static library
SRC := lib/conf.h lib/err.h lib/mem.h lib/bitmap.h lib/dwfs.h lib/dir.h lib/mem.c lib/dir.c lib/bitmap.c lib/dwfs.c
CFLAGS := -c

# Sources and flags to compile for unit testing
TESTSRC := lib/test_dwfs.c
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
