SRCS = *.cpp
HEAD = *.h
OBJS = connection.o networkmessage.o protocolgame81.o protocollogin.o \
	encryption.o	protocolgame80.o  protocolgame.o	rsa.o \
	protocolgame811.o protocolgame82.o protocolgame821.o \
	protocolgame83.o protocolgame831.o protocolgame84.o

all: __all.a

__all.a: $(OBJS)
	ar cru __all.a $(OBJS)
	ranlib __all.a

distdir: 
	cp $(SRCS) $(distdir)
	cp $(HEAD) $(distdir)
	cp Makefile $(distdir)
distcheck:
distclean:
install:
clean:
	rm $(OBJS) __all.a

