LIBRARY := libregex.a
OBJECTS := src/regex_allocators.o \
           src/regex_compile.o \
           src/regex_parse.o \
           src/regex_program.o \
           src/regex_vm_pike.o
DEPENDS := $(OBJECTS:%.o=%.d)
SUBDIRS := test examples/regex examples/regex2dot

CFLAGS  += -std=c11 -Wall -pedantic

PREFIX ?= /usr/local

.PHONY: all
all: CPPFLAGS += -DNDEBUG
all: CFLAGS   += -O2
all: $(LIBRARY) $(SUBDIRS)

.PHONY: debug
debug: CPPFLAGS += -DDEBUG
debug: CFLAGS   += -g
debug: LDFLAGS  += -g
debug: $(LIBRARY) $(SUBDIRS)

.PHONY: clean
clean:
	for subdir in $(SUBDIRS); do       \
		$(MAKE) -C $$subdir clean; \
	done
	$(RM) $(LIBRARY) $(OBJECTS) $(DEPENDS)

-include $(DEPENDS)

$(LIBRARY): $(OBJECTS)
	$(AR) rcs $@ $^

$(SUBDIRS): $(LIBRARY)
	$(MAKE) -C $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -c -o $@ $<

.PHONY: install
install: all
	mkdir -p $(PREFIX)/lib
	install $(LIBRARY) $(PREFIX)/lib
