SHARED=0

SRCDIR=./src/
INCDIR=./htpl/
OBJS = parse.o strutil.o token.o sections.o varlist.o format.o
CFLAGS = -ggdb 

all: clean libhtpl test

test:
	gcc -o htpltest -I$(INCDIR) test.c -L. -lhtpl 

libhtpl: $(OBJS)
ifeq ($(SHARED), 0)
	ar -r libhtpl.a $(OBJS)
else
	gcc -shared -o libhtpl.so $(LFLAGS) $(OBJS)
endif
        
%.o: $(SRCDIR)%.c
	gcc $(CFLAGS) -I$(INCDIR) -c $<

        
install: libhtpl
	install test /usr/local/sbin

clean:
	rm -f *~ *.o *.so.* *.so *.a htpltest
      
        
