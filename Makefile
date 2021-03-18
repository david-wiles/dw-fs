CC := clang

# Sources and flags needed to compile static library
VENDOR := vendor/cbitset/bitset.o
VENDOR_INCLUDE := vendor/cbitset/include
SRC := lib/dir.c lib/mem.c lib/dwfs.c
OBJ := $(SRC:%=build/%.o)

# Sources and flags to compile for unit testing
TESTSRC := lib/test_dwfs.c
TESTFLAGS := -lcheck

CFLAGS := -c


.PHONY: all
all: clean bin/dwfs.o test

.PHONY: test
test:
	$(CC) $(TESTFLAGS) -o $@ $(TESTSRC) bin/dwfs.o -I$(VENDOR_INCLUDE)
	./$@
	rm $@

.PHONY: clean
clean:
	rm -rf bin
	rm -rf build
	rm -rf debug

bin/dwfs.o: $(OBJ)
	mkdir -p $(dir $@)
	ld -r $(OBJ) $(VENDOR) -o $@

build/%.c.o: %.c $(SRC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@ -I$(VENDOR_INCLUDE)

# Compile debugging target
.PHONY: debug/dwfs
debug/dwfs:
	mkdir -p $(dir $@)
	$(CC) -g -o $@ lib/main.c $(SRC) $(VENDOR) -Ilib -I$(VENDOR_INCLUDE)
