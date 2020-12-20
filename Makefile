CFLAGS := -D_REENTRANT -Wall -pedantic -Isrc
CFLAGS += -fPIC
LDFLAGS = -lpthread

LIBNAME = libthreadpool
SHARED_SUFFIX = .so
STATIC_SUFFIX = .a

ifdef DEBUG
CFLAGS += -g
LDFLAGS += -g
endif

SHARED = $(LIBNAME)$(SHARED_SUFFIX)
STATIC = $(LIBNAME)$(STATIC_SUFFIX)

TARGETS = $(SHARED) $(STATIC)

all: $(TARGETS)

OBJS := \
	src/threadpool.o

deps := $(OBJS:%.o=%.o.d)
src/%.o: src/%.c
	$(CC) $(CFLAGS) -o $@ -MMD -MF $@.d -c $<
tests/%: tests/%.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

TESTS =
TESTS += tests/shutdown
TESTS += tests/thrdtest
TESTS += tests/heavy

$(LIBNAME)$(SHARED_SUFFIX): $(OBJS)
	$(CC) -shared -o $@ $< ${LDLIBS}

$(LIBNAME)$(STATIC_SUFFIX): $(OBJS)
	$(AR) rcs $@ $^

clean:
	rm -f $(TARGETS) *~ */*~ $(OBJS) $(deps)

test: check
check: $(TESTS)
	@for test in $^ ; \
	do \
		echo "Execute $$test..." ; $$test && echo "OK!\n" ; \
	done

-include $(deps)
