CC=gcc
DBG=valgrind

CFLAGS=-I$(IDIR) -Wall -Wshadow -Wvla -Wimplicit-function-declaration -pedantic -Werror -g3
DBGFLAGS=--leak-check=yes --leak-check=full --show-leak-kinds=all --track-origins=yes -s -q

IDIR=include
SDIR=src
BDIR=build
LDIR=lib
ODIR=$(BDIR)/obj
TDIR=$(SDIR)/tests

#_LIBS=libcjson.so.1.0 libhash.so.1.0
#LIBS=$(patsubst %,$(LDIR)/%,$(_DEPS))

_DEPS=types.h sha256.h base.h list.h cjson.h config.h extra.h
DEPS = $(patsubst %,$(IDIR)/cjson/%,$(_DEPS))

_SRC=$(notdir $(shell find $(SDIR) -maxdepth 1 -name '*.c'))
OBJS := $(addprefix $(ODIR)/,$(_SRC:%.c=%.o))
OBJ = $(ODIR)/cjson.o

_TEST=$(notdir $(shell find $(TDIR) -name '*.c'))
TEST := $(addprefix $(TDIR)/,$(_TEST))

BINS := $(addprefix $(BDIR)/,$(_TEST:%.c=%))

all: $(OBJ)

.PHONY: test

test: $(BINS) $(TEST) $(OBJ)
	$(foreach BIN,$(BINS), \
		time $(BIN) ; printf "\n" ; \
		)

.PHONY: debug

debug: $(BINS) $(TEST) $(OBJ)
	$(foreach BIN,$(BINS), \
		$(DBG) $(DBGFLAGS) $(BIN) ; printf "\n" ; \
		)

.PHONY: info

info:
	$(info $$DEPS is [${DEPS}])
	$(info $$OBJS is [${OBJS}])
	$(info $$TEST is [${TEST}])
	$(info $$BINS is [${BINS}])

$(OBJ): $(OBJS)
	ld -r $^ -o $(OBJ)

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BDIR)/%: $(TDIR)/%.c $(OBJ)
	$(CC) -o $@ $< $(OBJ) $(CFLAGS) $(LIBS)

$(OBJS): | $(ODIR)

$(ODIR): | $(BDIR)
	mkdir -p $@

$(BDIR):
	mkdir -p $@

.PHONY: clean

clean: 
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ && find $(BDIR) -type f -delete

# libjson.so:
# 	mkdir -p libs libs/cjson libs/hash
# 	cd src/cjson && gcc -O0 -g -Wall -fPIC -c *.c && gcc -shared -Wl,-soname,libcjson.so.1 -o ../../libs/cjson/libcjson.so.1.0 *.o; cd -
# 	ln -sf libcjson.so.1.0 libs/cjson/libcjson.so.1
# 	ln -sf libcjson.so.1.0 libs/cjson/libcjson.so
# 	cd src/hash && gcc -O0 -Wall -fPIC -c *.c && gcc -shared -Wl,-soname,libhash.so.1 -o ../../libs/hash/libhash.so.1.0 *.o; cd -
# 	ln -sf libhash.so.1.0 libs/hash/libhash.so.1
# 	ln -sf libhash.so.1.0 libs/hash/libhash.so
