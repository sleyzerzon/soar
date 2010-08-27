#CXXFLAGS+=-g
LIBS=

srcs=nsg_node.cpp sgel_interp.cpp linalg.cpp scene.cpp wm_sgo.cpp svs.cpp
objs=$(srcs:.cpp=.o)
headers=sg_node.h nsg_node.h sgel_interp.h linalg.h scene.h wm_sgo.h soar_interface.h svs.h
testprogs=tests/test_svs tests/buildsg

.SUFFIXES: # delete default suffix rules

all: $(testprogs)

tests/test_svs: tests/test_svs.cpp tests/test_interface.cpp tests/test_interface.h $(objs)
	$(CXX) -o $@ $^ $(LIBS)

tests/buildsg: tests/buildsg.cpp $(objs)
	$(CXX) -o $@ $^ $(LIBS)
	
%.o: %.cpp $(headers)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

.PHONY: clean tests

clean:
	$(RM) *.o *.gch $(testprogs)

tests:
	cd tests && ./test_sgel
	@echo All tests passed

soar:
	cd ../.. && scons

