CC := gcc

# Sources
SRC := lib/mem.c lib/dir.c lib/file_table.c lib/dwfs.c
INC := lib

VENDOR_SRC := vendor/lemire/cbitset/src/bitset.c vendor/david-wiles/htable/htable.c
VENDOR_INC := -Ivendor/lemire/cbitset/include -Ivendor/david-wiles/htable

# Sources and flags to compile for unit testing
TESTSRC := lib/test_dwfs.c
TESTFLAGS := -pthread -fPIC -std=gnu99 -ggdb -Wall -Wextra -Wshadow -fsanitize=undefined  -fno-omit-frame-pointer -fsanitize=address

CFLAGS := -pthread -fPIC -std=gnu99 -O3  -Wall -Wextra -Wshadow


# Copy object file to current directory if all tests succeeded
.PHONY: all
all: clean bin/dwfs.o test
	cp bin/dwfs.o dwfs.o

vendor:
	mkdir -p "vendor/lemire/cbitset"
	mkdir -p "vendor/david-wiles/htable"
	git clone https://github.com/lemire/cbitset.git vendor/lemire/cbitset
	git clone https://github.com/david-wiles/htable.git vendor/david-wiles/htable

.PHONY: test
test: clean debug/test
	./debug/test
	rm ./debug/test

.PHONY: clean
clean:
	rm -rf bin
	rm -rf build
	rm -rf debug

debug/test:
	mkdir -p $(dir $@)
	$(CC) $(TESTFLAGS) $(TESTSRC) $(SRC) $(VENDOR_SRC) -o $@ -I$(INC) $(VENDOR_INC)

bin/dwfs.o:
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $(SRC) $(VENDOR_SRC) -o $@ -I$(INC) $(VENDOR_INC)
