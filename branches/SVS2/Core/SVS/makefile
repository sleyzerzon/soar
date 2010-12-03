srcs=src/filters/*.h src/filters/*.cpp
lib=${SOAR_HOME}/build/SVS/libsvs.a

all: src/filter_factory.cpp svs.cxx

src/filter_factory.cpp: ${srcs}
	sh gen_filter_factory

svs.cxx: ${srcs} svs.static.cxx
	sh gen_filter_factory
