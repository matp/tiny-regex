PROGRAM := test
OBJECTS := test.o

CFLAGS  += -std=c11 -Wall -pedantic -I../src

.PHONY: all
all: CPPFLAGS += -DNDEBUG
all: CFLAGS   += -O2
all: $(PROGRAM) $(SUBDIRS)

.PHONY: debug
debug: CPPFLAGS += -DDEBUG
debug: CFLAGS   += -g
debug: LDFLAGS  += -g
debug: $(PROGRAM) $(SUBDIRS)

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJECTS)

$(PROGRAM): $(OBJECTS) ../libregex.a
	$(CC) $(LDFLAGS) -o $@ $^ ../libregex.a

../libregex.a:
	$(MAKE) -C .. libregex.a

test.c: make-test.rb testdata/basic.dat testdata/nullsubexpr.dat testdata/repetition.dat
	./make-test.rb testdata/basic.dat testdata/nullsubexpr.dat testdata/repetition.dat > $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
