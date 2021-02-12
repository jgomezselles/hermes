ROOTDIR=$(shell git rev-parse --show-toplevel)

-include $(BUILD_TYPE).mk

# Order IS important ...
#LIBS=server
CPPFLAGS += -I$(ROOTDIR)/include
LDFLAGS  += -L$(ROOTDIR)/lib

LDLIBS = $(addprefix -l,$(shell echo $(LIBS) | tac -s " ")) \
    -lnghttp2_asio -lnghttp2 \
    -lboost_thread -lboost_filesystem -lboost_system \
    -lboost_regex -lssl -lcrypto -ldl -lpthread

EXECS=hermes

all: $(EXECS)

$(EXECS): main.o
	$(CXX) -o $@ $^ $(LDFLAGS) $(LDLIBS)

install: configure libs $(EXECS)
	mkdir -p $(DESTDIR)/bin
	install -m 755 $(EXECS) $(DESTDIR)/bin

clean:
	rm -f *.o
	rm -f $(EXECS)
	rm -f *.gcno *.gcda

staging:
	make DESTDIR=$(DESTDIR) install
	if [ $(BUILD_TYPE) = .debug ]; then install `which cp` $(DESTDIR)/bin/; fi
	if [ $(BUILD_TYPE) = .debug ]; then  tar cf - `find . -name \*.gcno` | tar -C $(DESTDIR) -xf -; fi
	STAGING_AREA=$(DESTDIR) mkstaging $(DESTDIR)/bin/*
