ifdef STAGING_DIR
LDFLAGS+=-rpath-link $(shell find $(STAGING_DIR) -name ld-uClibc.so.0)
endif

WakeON_server: server.o
	$(CC) $(LDFLAGS) server.o -o WakeON_server

server.o: server.c 
	$(CC) $(CFLAGS) -c server.c 

# remove object files and executable when user executes "make clean"
clean:
	rm *.o WakeON_server