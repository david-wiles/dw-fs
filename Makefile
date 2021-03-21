CC := clang

# Sources and flags needed to compile static library
VENDOR_SRC := vendor/lemire/cbitset/bitset.o vendor/mikhail-j/libc_hsearch_r/search_hsearch_r.c
VENDOR_FLAGS := -Ivendor/lemire/cbitset/include -Ivendor/mikhail-j/libc_hsearch_r/include
SRC := lib/mem.c lib/hash_table.c lib/dir.c lib/file_table.c lib/dwfs.c
OBJ := $(SRC:%=build/%.o)

# Sources and flags to compile for unit testing
TESTSRC := lib/test_dwfs.c
TESTFLAGS := -lcheck -lpthread -fPIC  -std=c99 -ggdb -Wall -Wextra -Wshadow -fsanitize=undefined  -fno-omit-frame-pointer -fsanitize=address

CFLAGS := -lpthread -c -fPIC -std=c99 -O3  -Wall -Wextra -Wshadow


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


debug/test: bin/dwfs.o $(TESTSRC)
	mkdir -p $(dir $@)
	$(CC) $(TESTFLAGS) -o $@ $(TESTSRC) $(SRC) $(VENDOR_SRC) -Ilib $(VENDOR_FLAGS)

bin/dwfs.o: $(OBJ)
	mkdir -p $(dir $@)
	ld -r $(OBJ) $(VENDOR_SRC) -o $@

build/%.c.o: %.c $(SRC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@ $(VENDOR_FLAGS)
