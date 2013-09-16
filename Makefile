CFLAGS_DBG=   $(CFLAGS) -O0 -g -Wall
LDFLAGS_DBG=  $(LDFLAGS) -rdynamic -lpthread
CPPFLAGS_DBG= $(CPPFLAGS)

DIR_DBG=debug
O_DBG=$(DIR_DBG)/annot



CFLAGS+=  -O2 -Wall
LDFLAGS+= -rdynamic -lpthread
CPPFLAGS+=

DIR=build
O=$(DIR)/annot



sh_files=find -type f -print0 | grep -viP '/[\._]' -zZ | grep -iP '\.c$$' -zZ | xargs -t0

compile:
	mkdir -p "$(DIR)"
	$(sh_files) $(CC) $(CFLAGS) $(LDFLAGS) $(CPPFLAGS) -o "$(O)"

debug:
	mkdir -p "$(DIR_DBG)"
	$(sh_files) $(CC) $(CFLAGS_DBG) $(LDFLAGS_DBG) $(CPPFLAGS_DBG) -o "$(O_DBG)"

gdb: debug
	gdb $(O_DBG)


clean-compile:
	rm -Rfv $(DIR)

clean-debug:
	rm -Rfv $(DIR_DBG)

clean: clean-compile clean-debug
