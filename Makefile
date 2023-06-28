IDIR=include
CC=gcc
CFLAGS=-I$(IDIR) -Wall -Wshadow -Wvla -Wimplicit-function-declaration -pedantic -lm -g3 #-Werror

SDIR=src
ODIR=$(SDIR)/obj
LDIR =lib

#_LIBS=libcjson.so.1.0 libhash.so.1.0
#LIBS=$(patsubst %,$(LDIR)/%,$(_DEPS))

_DEPS=types.h sha256.h base.h list.h cjson.h config.h extra.h
DEPS = $(patsubst %,$(IDIR)/cjson/%,$(_DEPS))

_OBJ=base.o sha256.o types.o test.o config.o extra.o list.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS) | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

test: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(OBJ): | $(ODIR)

$(ODIR):
	mkdir -p $@

run: test
	# export LD_LIBRARY_PATH=$(LDIR):$(LD_LIBRARY_PATH)
	./test

.PHONY: clean

clean: 
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

# libjson.so:
# 	mkdir -p libs libs/cjson libs/hash
# 	cd src/cjson && gcc -O0 -g -Wall -fPIC -c *.c && gcc -shared -Wl,-soname,libcjson.so.1 -o ../../libs/cjson/libcjson.so.1.0 *.o; cd -
# 	ln -sf libcjson.so.1.0 libs/cjson/libcjson.so.1
# 	ln -sf libcjson.so.1.0 libs/cjson/libcjson.so
# 	cd src/hash && gcc -O0 -Wall -fPIC -c *.c && gcc -shared -Wl,-soname,libhash.so.1 -o ../../libs/hash/libhash.so.1.0 *.o; cd -
# 	ln -sf libhash.so.1.0 libs/hash/libhash.so.1
# 	ln -sf libhash.so.1.0 libs/hash/libhash.so
