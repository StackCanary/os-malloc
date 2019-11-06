CC= gcc
CFLAGS= -g -Wall -Wextra
LIBOBJS = myalloc.o
HEADERS = prototypes.h debug.h chunk.h arena.h mpage.h
LIB=myalloc
LIBFILE=lib$(LIB).a
TESTS = test
all: $(TESTS) $(LIB) 

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< 

test : test.o $(LIB) $(HEADERS)
	$(CC) test.o $(CFLAGS) -o test -L. -l$(LIB) -lpthread

$(LIB) : $(LIBOBJS) $(HEADERS)
	ar -cvr $(LIBFILE) $(LIBOBJS)
	#ranlib $(LIBFILE) # may be needed on some systems
	ar -t $(LIBFILE)

clean:
	/bin/rm -f *.o $(TESTS) $(LIBFILE)
