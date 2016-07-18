# build helloworld executable when user executes "make" 
ifdef STAGING_DIR
LDFLAGS+=-rpath-link $(shell find $(STAGING_DIR) -name ld-uClibc.so.0)
endif

WakeON_conf: config.o
	$(CC) $(LDFLAGS) config.o -o WakeON_conf

config.o: config.c 
	$(CC) $(CFLAGS) -c config.c 

# remove object files and executable when user executes "make clean"
clean:
	rm *.o WakeON_conf