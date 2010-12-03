headers=../*.h ../filters/*.h
srcs=../*.cpp ../filters/*.cpp
objs=$($(notdir srcs):.cpp=.o)

testprogs=bbox buildsg buildfilter

CXXFLAGS=-g -I../ -I../filters

.SUFFIXES: # delete default suffix rules
.PHONY: clean tests

all: $(testprogs)

bbox: bbox.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

buildsg: buildsg.cpp test_interface.cpp nsg_node.o sgel_interp.o linalg.o scene.o ipcsocket.o
	$(CXX) $(CXXFLAGS) -o $@ $^

buildfilter: buildfilter.cpp linalg.o nsg_node.o filter.o bbox_filter.o gen_filter.o filter_factory.o
	$(CXX) $(CXXFLAGS) -o $@ $^

test_svs: test_svs.cpp test_interface.cpp test_interface.h $(objs)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: ../%.cpp $(headers)
	$(CXX) $(CXXFLAGS) -c $(CXXFLAGS) -o $@ $<

%.o: ../filters/%.cpp $(headers)
	$(CXX) $(CXXFLAGS) -c $(CXXFLAGS) -o $@ $<

clean:
	$(RM) *.o *.out $(testprogs)

tests: $(testprogs)
	@./test_bbox || echo bbox fail
	@./test_filter || echo filter fail
	@./test_nsg || echo nsg fail
	