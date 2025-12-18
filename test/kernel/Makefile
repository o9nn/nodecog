# Simple test build for kernel functions
CXX = g++
CXXFLAGS = -std=c++17 -I../.. -I../../deps/ggml/include -I../../deps/ggml/src -Wall -Wextra
LDFLAGS = -L../../out/Release/obj.target -lpthread

# GGML objects (we'll compile them)
GGML_SRCS = ../../deps/ggml/src/ggml.c \
            ../../deps/ggml/src/ggml-alloc.c \
            ../../deps/ggml/src/ggml-backend.cpp \
            ../../deps/ggml/src/ggml-quants.c

KERN_SRCS = ../../src/kern_boot.cc

TEST_TARGET = test_kern_boot

all: $(TEST_TARGET)

$(TEST_TARGET): test_kern_boot.cc
	$(CXX) $(CXXFLAGS) -o $@ test_kern_boot.cc $(KERN_SRCS) $(GGML_SRCS) $(LDFLAGS)

clean:
	rm -f $(TEST_TARGET) *.o

test: $(TEST_TARGET)
	./$(TEST_TARGET)

.PHONY: all clean test
