CC := clang

# Sources and flags needed to compile static library
VENDOR := vendor/cbitset/bitset.o
VENDOR_INCLUDE := vendor/cbitset/include
SRC := lib/dir.c lib/mem.c lib/dwfs.c
OBJ := $(SRC:%=build/%.o)

# Sources and flags to compile for unit testing
TESTSRC := lib/test_dwfs.c
TESTFLAGS := -lcheck -fPIC  -std=c99 -ggdb -Wall -Wextra -Wshadow -fsanitize=undefined  -fno-omit-frame-pointer -fsanitize=address

CFLAGS := -c -fPIC -std=c99 -O3  -Wall -Wextra -Wshadow


.PHONY: all
all: clean bin/dwfs.o test
	# Copy object file to current directory if all tests succeeded
	cp bin/dwfs.o dwfs.o

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
	$(CC) $(TESTFLAGS) -o $@ $(TESTSRC) $(SRC) $(VENDOR) -Ilib -I$(VENDOR_INCLUDE)

bin/dwfs.o: $(OBJ)
	mkdir -p $(dir $@)
	ld -r $(OBJ) $(VENDOR) -o $@

build/%.c.o: %.c $(SRC)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@ -I$(VENDOR_INCLUDE)
