CFLAGS = -D_REENTRANT -Wall -pedantic -Isrc
LDLIBS = -lpthread

ifdef DEBUG
CFLAGS += -g
LDFLAGS += -g
endif

TARGETS = tests/thirdtest tests/heavy tests/shutdown \
	libthreadpool.so libthreadpool.a

all: $(TARGETS)

tests/shutdown: tests/shutdown.o src/threadpool.o
tests/thirdtest: tests/thirdtest.o src/threadpool.o
tests/heavy: tests/heavy.o src/threadpool.o
src/threadpool.o: src/threadpool.c src/threadpool.h
tests/thirdtest.o: tests/thirdtest.c src/threadpool.h
tests/heavy.o: tests/heavy.c src/threadpool.h

# Short-hand aliases
shared: libthreadpool.so
static: libthreadpool.a

libthreadpool.so: src/threadpool.c src/threadpool.h
	$(CC) -shared -fPIC ${CFLAGS} -o $@ $< ${LDLIBS}

src/libthreadpool.o: src/threadpool.c src/threadpool.h
	$(CC) -c -fPIC ${CFLAGS} -o $@ $<

libthreadpool.a: src/libthreadpool.o
	ar rcs $@ $^

clean:
	rm -f $(TARGETS) *~ */*~ */*.o

test: $(TARGETS)
	./tests/shutdown
	./tests/thirdtest
	./tests/heavy

