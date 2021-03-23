CC := clang

# Sources and flags needed to compile static library
VENDOR_SRC := vendor/lemire/cbitset/src/bitset.c vendor/mikhail-j/libc_hsearch_r/search_hsearch_r.c
VENDOR_FLAGS := -Ivendor/lemire/cbitset/include -Ivendor/mikhail-j/libc_hsearch_r/include
SRC := lib/mem.c lib/hash_table.c lib/dir.c lib/file_table.c lib/dwfs.c

OBJ := $(SRC:%=build/%.o)
VENDOR_OBJ := $(VENDOR_SRC:%=build/%.o)

# Sources and flags to compile for unit testing
TESTSRC := lib/test_dwfs.c
TESTFLAGS := -lcheck -lpthread -fPIC  -std=c99 -ggdb -Wall -Wextra -Wshadow -fsanitize=undefined  -fno-omit-frame-pointer -fsanitize=address

CFLAGS := -lpthread -fPIC -std=c99 -O3  -Wall -Wextra -Wshadow


# Copy object file to current directory if all tests succeeded
.PHONY: all
all: clean bin/dwfs.o test
	cp bin/dwfs.o dwfs.o

vendor:
	mkdir -p "vendor/lemire/cbitset"
	mkdir -p "vendor/mikhail-j/libc_hsearch_r"
	git clone https://github.com/lemire/cbitset.git vendor/lemire/cbitset
	git clone https://github.com/mikhail-j/libc_hsearch_r.git vendor/mikhail-j/libc_hsearch_r

.PHONY: test
test: debug/test
	./$^
	rm $^

.PHONY: clean
clean:
	rm -rf bin
	rm -rf build
	rm -rf debug

debug/test:
	mkdir -p $(dir $@)
	$(CC) $(TESTFLAGS) $(TESTSRC) $(SRC) $(VENDOR_SRC) -o $@ -Ilib $(VENDOR_FLAGS)

bin/dwfs.o: $(OBJ) $(VENDOR_OBJ)
	mkdir -p $(dir $@)
	ld -r $(OBJ) $(VENDOR_OBJ) -o $@

build/%.c.o: %.c $(SRC) $(VENDOR_SRC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@ -Ilib $(VENDOR_FLAGS)
