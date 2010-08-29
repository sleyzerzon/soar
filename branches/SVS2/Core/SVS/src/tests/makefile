headers=$(addprefix ../,sg_node.h nsg_node.h sgel_interp.h linalg.h scene.h wm_sgo.h soar_interface.h svs.h)
srcs=$(addprefix ../,nsg_node.cpp sgel_interp.cpp linalg.cpp scene.cpp wm_sgo.cpp svs.cpp soar_interface.cpp)
objs=$($(notdir srcs):.cpp=.o)

testprogs=bbox buildsg buildfilter

CXXFLAGS=-g

.SUFFIXES: # delete default suffix rules
.PHONY: clean tests

all: $(testprogs)

bbox: bbox.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

buildsg: buildsg.cpp test_interface.cpp nsg_node.o sgel_interp.o linalg.o scene.o
	$(CXX) $(CXXFLAGS) -o $@ $^

buildfilter: buildfilter.cpp filter.o bbox_filter.o linalg.o
	$(CXX) $(CXXFLAGS) -o $@ $^

test_svs: test_svs.cpp test_interface.cpp test_interface.h $(objs)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: ../%.cpp $(headers)
	$(CXX) $(CXXFLAGS) -c $(CXXFLAGS) -o $@ $<

clean:
	$(RM) *.o $(testprogs)

tests:
	cd tests && ./test_sgel
	@echo All tests passed
