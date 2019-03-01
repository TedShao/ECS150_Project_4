# Target library
lib := libfs.a
objs := disk.o fs.objs

CC := gcc
CFLAGS := -Wall -Werror

all: $(lib)

deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)
DEPFLAGS = -MMD -MF $(@:.o=.d)

$(lib): $(objs)
	ar rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(DEPFLAGS)	

clean:
	rm $(lib) $(del_objs) $(deps)
