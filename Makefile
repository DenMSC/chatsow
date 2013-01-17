CC = gcc
DEBUG = -g
CFLAGS = -O2 -Wall -c $(DEBUG)
LFLAGS = -O2 -Wall -lm $(DEBUG)

PROGRAM = wrlc

SOURCE = source/
BUILD = build/

THIS = Makefile
CFILES = $(wildcard $(SOURCE)*.c)
MODULES = $(patsubst $(SOURCE)%.c,%,$(CFILES))
OBJS = $(addprefix $(BUILD),$(addsuffix .o,$(MODULES)))
$(shell mkdir -p $(BUILD))

default: $(BUILD)$(PROGRAM)

clean:
	rm -rf $(BUILD)

test: $(BUILD)$(PROGRAM)
	./$(BUILD)$(PROGRAM) 127.0.0.1

$(BUILD)$(PROGRAM): $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $@

define module_depender
$(shell mkdir -p $(BUILD))
$(shell touch $(BUILD)$(1).d)
$(shell makedepend -f $(BUILD)$(1).d -- $(CFLAGS) -- $(SOURCE)$(1).c
	2>/dev/null)
$(shell sed -i 's~^$(SOURCE)~$(BUILD)~g' $(BUILD)$(1).d)
-include $(BUILD)$(1).d
endef

define module_compiler
$(BUILD)$(1).o: $(SOURCE)$(1).c $(THIS)
	$(CC) $(CFLAGS) $$< -o $$@
endef

$(foreach module, $(MODULES), $(eval $(call module_depender,$(module))))
$(foreach module, $(MODULES), $(eval $(call module_compiler,$(module))))

.PHONY: default clean test
