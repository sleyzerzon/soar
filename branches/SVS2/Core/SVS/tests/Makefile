OPTS=-I../src -I../src/models -L/home/jzxu/local/lib

all: lwrtest balltree_test

lwrtest:
	g++ -pg -o lwrtest $(OPTS) ../src/models/lwr.cpp lwrtest.cpp -larmadillo

balltree_test:
	g++ -pg -o balltree_test $(OPTS) ../src/models/balltree.cpp balltree_test.cpp -larmadillo

clean:
	rm -f lwrtest balltree_test
	