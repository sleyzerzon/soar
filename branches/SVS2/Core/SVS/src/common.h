#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <string>
#include <vector>
#include <ostream>

void split(const std::string &s, const std::string &delim, std::vector<std::string> &fields);
std::string getnamespace();

class floatvec {
public:
	floatvec() : mem(NULL), end(NULL), sz(0) {}
	
	floatvec(const floatvec &v) : sz(v.sz) { 
		mem = (float *) malloc(sz * sizeof(float));
		end = mem + sz;
		memcpy(mem, v.mem, sizeof(float) * sz);
	}
	
	floatvec(int sz) : sz(sz) {
		mem = (float *) malloc(sz * sizeof(float));
		end = mem + sz;
	}
	
	floatvec(const std::vector<float> &v) {
		float *p1;
		std::vector<float>::const_iterator p2;
		
		sz = v.size();
		mem = (float *) malloc(sz * sizeof(float));
		end = mem + sz;
		for (p1 = mem, p2 = v.begin(); p1 != end; ++p1, ++p2) {
			*p1 = *p2;
		}
	}
	
	~floatvec() {
		free(mem);
	}
	
	float distsq(const floatvec &v) const {
		assert(sz == v.sz);
		
		float *p1, *p2;
		double s, d;
		for(p1 = mem, p2 = v.mem; p1 != end; ++p1, ++p2) {
			d = *p1 - *p2;
			s += d * d;
		}
		return s;
	}
	
	int size() const { return sz; }

	void zero() {
		memset(mem, 0, sz * sizeof(float));
	}
	
	void replace(float v1, float v2) {
		for (float *p = mem; p != end; ++p) {
			if (*p == v1) *p = v2;
		}
	}
	
	void resize(int size) {
		sz = size;
		mem = (float*) realloc(mem, sz * sizeof(float));
		end = mem + sz;
	}
	
	float sum() const {
		float s = 0.0, *p;
		for (p = mem; p != end; ++p) {
			s += *p;
		}
		return s;
	}
	
	float &operator[](int i) {
		assert(i >= 0 && i < sz);
		return mem[i];
	}
	
	float operator[](int i) const {
		assert(i >= 0 && i < sz);
		return mem[i];
	}
	
	void operator=(const floatvec &v) {
		if (sz != v.sz) {
			sz = v.sz;
			free(mem);
			mem = (float*) malloc(sizeof(float) * sz);
			end = mem + sz;
		}
		memcpy(mem, v.mem, sz * sizeof(float));
	}
	
	void combine(const floatvec &v1, const floatvec &v2) {
		assert(sz == v1.sz + v2.sz);
		float *p1 = mem, *p2;
		for(p2 = v1.mem; p2 != v1.end; ++p1, ++p2) {
			*p1 = *p2;
		}
		for(p2 = v2.mem; p2 != v2.end; ++p1, ++p2) {
			*p1 = *p2;
		}
	}
	
	void operator+=(const floatvec &v) {
		assert(sz == v.sz);
		float *p1, *p2;
		
		for(p1 = mem, p2 = v.mem; p1 != end; ++p1, ++p2) {
			*p1 += *p2;
		}
	}
	
	void operator-=(const floatvec &v) {
		assert(sz == v.sz);
		float *p1, *p2;
		
		for(p1 = mem, p2 = v.mem; p1 != end; ++p1, ++p2) {
			*p1 -= *p2;
		}
	}
	
	void operator*=(const floatvec &v) {
		assert(sz == v.sz);
		float *p1, *p2;
		
		for(p1 = mem, p2 = v.mem; p1 != end; ++p1, ++p2) {
			*p1 *= *p2;
		}
	}
	
	void operator*=(float v) {
		for(float *p = mem; p != end; ++p) {
			*p *= v;
		}
	}
	
	void operator/=(const floatvec &v) {
		assert(sz == v.sz);
		float *p1, *p2;
		
		for(p1 = mem, p2 = v.mem; p1 != end; ++p1, ++p2) {
			*p1 /= *p2;
		}
	}
	
	void operator/=(float v) {
		for(float *p = mem; p != end; ++p) {
			*p /= v;
		}
	}
	
private:
	float *mem, *end;
	int sz;
};

std::ostream &operator<<(std::ostream &os, const floatvec &v);

#endif
