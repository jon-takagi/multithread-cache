CXX=clang++
CXXFLAGS=-Wall -Wextra -pedantic -Werror -std=c++17 -O3 -g -I /usr/local/boost_1_72_0/ -pthread
LDFLAGS=$(CXXFLAGS)
OBJ=$(SRC:.cc=.o)
BUILDDIR=out/
VPATH=cache/:out/

all: server.bin test_driver.bin test_gen.bin

$(BUILDDIR)%.o: %.cc
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -c $< -o $@

clean:
	rm out/*.o
	rm *.o
	rm *.bin

server.bin: $(BUILDDIR)cache_lib.o $(BUILDDIR)fifo_evictor.o $(BUILDDIR)tcp_listener.o
	$(CXX) $(LDFLAGS) -o $@ cache/cache_server.cc  $^ /vagrant/systems/boost/lib/libboost_program_options.a

test_driver.bin: $(BUILDDIR)fifo_evictor.o $(BUILDDIR)driver.o $(BUILDDIR)gen.o $(BUILDDIR)cache_client.o
	$(CXX) $(LDFLAGS) -o $@ driver_test.cc  $^
test_gen.bin: $(BUILDDIR)gen.o
	$(CXX) $(LDFLAGS) -o $@ gen_test.cc  $^
