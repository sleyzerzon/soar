#CXXFLAGS+=-g
LIBS=-lCGAL -lboost_thread-mt

srcs=nsg_node.cpp sgel_interp.cpp
objs=$(srcs:.cpp=.o)
headers=sg_node.h nsg_node.h cgalsupport.h sgel_interp.h
pcheaders=sg_node.h.gch nsg_node.h.gch cgalsupport.h.gch sgel_interp.h.gch
#pcheaders=$(headers:.h=.h.gch)

.SUFFIXES:  # delete default suffix rules
.PRECIOUS: $(pcheaders)

all: buildsg

buildsg: buildsg.cpp $(objs)
	$(CXX) -o $@ $^ $(LIBS)

%.o: %.cpp $(pcheaders)
	$(CXX) -c $(CFLAGS) -o $@ $<

%.h.gch: %.h
	$(CXX) $<

.PHONY: clean tests

clean:
	$(RM) *.o *.gch $(tests)

tests:
	cd tests && ./test_sgel
	@echo All tests passed
