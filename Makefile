CC=gcc
CFLAGS=-I$(IDIR) -Wall -Wshadow -Wvla -Wimplicit-function-declaration -pedantic -lm -g3 -Werror

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
OBJ := $(addprefix $(ODIR)/,$(_SRC:%.c=%.o))

_TEST=$(notdir $(shell find $(TDIR) -name '*.c'))
TEST := $(addprefix $(TDIR)/,$(_TEST))

BINS := $(addprefix $(BDIR)/,$(_TEST:%.c=%))

all: $(OBJ)

.PHONY: test

test: $(BINS) $(TEST) $(OBJ)
	$<

.PHONY: info

info:
	$(info $$DEPS is [${DEPS}])
	$(info $$OBJ is [${OBJ}])
	$(info $$TEST is [${TEST}])
	$(info $$BINS is [${BINS}])

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BINS): $(TEST) $(OBJ)
	$(CC) -o $@ $< $(filter-out $<,$^) $(CFLAGS) $(LIBS)

$(OBJ): | $(ODIR)

$(ODIR): | $(BDIR)
	mkdir -p $@

$(BDIR):
	mkdir -p $@

.PHONY: clean

clean: 
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
	find $(BDIR) -type f -delete

# libjson.so:
# 	mkdir -p libs libs/cjson libs/hash
# 	cd src/cjson && gcc -O0 -g -Wall -fPIC -c *.c && gcc -shared -Wl,-soname,libcjson.so.1 -o ../../libs/cjson/libcjson.so.1.0 *.o; cd -
# 	ln -sf libcjson.so.1.0 libs/cjson/libcjson.so.1
# 	ln -sf libcjson.so.1.0 libs/cjson/libcjson.so
# 	cd src/hash && gcc -O0 -Wall -fPIC -c *.c && gcc -shared -Wl,-soname,libhash.so.1 -o ../../libs/hash/libhash.so.1.0 *.o; cd -
# 	ln -sf libhash.so.1.0 libs/hash/libhash.so.1
# 	ln -sf libhash.so.1.0 libs/hash/libhash.so
