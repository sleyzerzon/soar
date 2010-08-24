#CXXFLAGS+=-g
LIBS=

srcs=nsg_node.cpp sgel_interp.cpp linalg.cpp
objs=$(srcs:.cpp=.o)
headers=sg_node.h nsg_node.h sgel_interp.h linalg.h

.SUFFIXES: # delete default suffix rules

all: buildsg

buildsg: buildsg.cpp $(objs)
	$(CXX) -o $@ $^ $(LIBS)
	
%.o: %.cpp $(headers)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

.PHONY: clean tests

clean:
	$(RM) *.o *.gch $(tests) buildsg

tests:
	cd tests && ./test_sgel
	@echo All tests passed
