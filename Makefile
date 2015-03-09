CFLAGS = -D_REENTRANT -Wall -pedantic -Isrc
LDLIBS = -lpthread

ifdef DEBUG
CFLAGS += -g
LDFLAGS += -g
endif

TARGETS = tests/thrdtest tests/heavy tests/shutdown

all: $(TARGETS)

tests/shutdown: tests/shutdown.o src/threadpool.o
tests/thrdtest: tests/thrdtest.o src/threadpool.o
tests/heavy: tests/heavy.o src/threadpool.o
src/threadpool.o: src/threadpool.c src/threadpool.h
tests/thrdtest.o: tests/thrdtest.c src/threadpool.h
tests/heavy.o: tests/heavy.c src/threadpool.h

shared_lib:
	cc \
		-shared \
		-fPIC \
		${CFLAGS} \
		-o libthreadpool.so \
		src/threadpool.c \
		${LDLIBS}

static_lib:
	cc \
		-c \
		-fPIC \
		${CFLAGS} \
		-o libthreadpool.o \
		src/threadpool.c \
		${LDLIBS}
	ar rcs \
		libthreadpool.a \
		libthreadpool.o

clean:
	rm -f $(TARGETS) *~ */*~ */*.o *.so
