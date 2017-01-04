PROGRAM := regex2dot
OBJECTS := regex2dot.o
DEPENDS := $(OBJECTS:%.o=%.d)

CFLAGS  += -std=c11 -Wall -pedantic -I../../src

.PHONY: all
all: CPPFLAGS += -DNDEBUG
all: CFLAGS   += -O2
all: $(PROGRAM)

.PHONY: debug
debug: CPPFLAGS += -DDEBUG
debug: CFLAGS   += -g
debug: LDFLAGS  += -g
debug: $(PROGRAM)

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJECTS) $(DEPENDS)

-include $(DEPENDS)

$(PROGRAM): $(OBJECTS) ../../libregex.a
	$(CC) $(LDFLAGS) -o $@ $^ ../../libregex.a

../../libregex.a:
	$(MAKE) -C ../.. libregex.a

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -c -o $@ $<
