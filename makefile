CXX=g++
CXXFLAGS=-Wall -Wextra -pedantic -Werror -std=c++17 -O3 -g -I /usr/local/boost_1_72_0/ -pthread
LDFLAGS=$(CXXFLAGS)
OBJ=$(SRC:.cc=.o)
BUILDDIR=out/
VPATH=cache/:out/

all: server.bin perf_test.bin data_pt1.bin

$(BUILDDIR)%.o: %.cc
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -c $< -o $@

clean:
	rm out/*.o
	rm *.bin

server.bin: $(BUILDDIR)cache_lib.o $(BUILDDIR)fifo_evictor.o $(BUILDDIR)tcp_listener.o
	$(CXX) $(LDFLAGS) -o $@ cache/cache_server.cc  $^ /usr/local/boost_1_72_0/ehpop/Documents/lib/libboost_program_options.a

perf_test.bin:  $(BUILDDIR)fifo_evictor.o $(BUILDDIR)gen.o $(BUILDDIR)cache_client.o
	$(CXX) $(LDFLAGS) -o $@ thread_test.cc  $^

data_pt1.bin:  $(BUILDDIR)fifo_evictor.o $(BUILDDIR)gen.o $(BUILDDIR)cache_client.o
	$(CXX) $(LDFLAGS) -o $@ data_output.cc  $^
