# Simple test build for kernel functions
CXX = g++
CC  = gcc
CXXFLAGS = -std=c++17 -I../.. -I../../deps/ggml/include -I../../deps/ggml/src \
           -I../../deps/uv/include \
           -Wall -Wextra
CFLAGS   = -std=c99  -I../.. -I../../deps/ggml/include -I../../deps/ggml/src \
           -I../../deps/uv/include \
           -Wall -Wextra
LDFLAGS = -lpthread

# GGML objects (stub implementation – replaced by real ggml.c when submodule present)
GGML_SRCS = ../../deps/ggml/src/ggml.c

KERN_SRCS = ../../src/kern_boot.cc \
            ../../src/kern_hypergraph_fs.cc \
            ../../src/kern_scheduler.cc

TEST_TARGET = test_kern_boot

all: $(TEST_TARGET)

$(TEST_TARGET): test_kern_boot.cc $(KERN_SRCS) $(GGML_SRCS) uv_stub.c
	$(CXX) $(CXXFLAGS) -o $@ test_kern_boot.cc $(KERN_SRCS) \
	  -x c $(GGML_SRCS) uv_stub.c -x none \
	  $(LDFLAGS)

clean:
	rm -f $(TEST_TARGET) *.o

test: $(TEST_TARGET)
	./$(TEST_TARGET)

.PHONY: all clean test
