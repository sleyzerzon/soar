#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <string>
#include <vector>
#include <algorithm>
#include <ostream>

void split(const std::string &s, const std::string &delim, std::vector<std::string> &fields);
std::string getnamespace();

class timer {
public:
	timer() {}
	
	void start() {
		gettimeofday(&t1, NULL);
	}
	
	double stop() {
		timeval t2, t3;
		gettimeofday(&t2, NULL);
		timersub(&t2, &t1, &t3);
		return t3.tv_sec + t3.tv_usec / 1000000.0;
	}
	
	timeval t1;
};

class floatvec {
public:
	floatvec() : sz(0), mem(NULL) {}
	
	floatvec(const floatvec &v) : sz(v.sz) { 
		mem = (float *) malloc(sz * sizeof(float));
		memcpy(mem, v.mem, sizeof(float) * sz);
	}
	
	floatvec(int sz) : sz(sz) {
		mem = (float *) malloc(sz * sizeof(float));
	}
	
	floatvec(int sz, float *data) : sz(sz) {
		mem = (float *) malloc(sz * sizeof(float));
		memcpy(mem, data, sizeof(float) * sz);
	}
	
	floatvec(const std::vector<float> &v) {
		float *p1;
		std::vector<float>::const_iterator p2;
		
		sz = v.size();
		mem = (float *) malloc(sz * sizeof(float));
		std::copy(v.begin(), v.end(), mem);
	}
	
	~floatvec() {
		free(mem);
	}

	int size() const {
		return sz;
	}

	int argmax() const {
		int max = 0;
		for(int i = 1; i < sz; i++) {
			if (mem[i] > mem[max]) {
				max = i;
			}
		}
		return max;
	}
	
	int max() const {
		return mem[argmax()];
	}

	/* The working array is necessary to allow the operations to be
	 * vectorized. It must be at least as large as the floatvec.
	 * Auto-vectorization by g++ v4.2+ gives a 4x speed-up with 4 byte
	 * floats on intel processors with sse2+. Without auto-vectorization
	 * this at least shouldn't hurt performance. */
	float distsq(const floatvec &v, float * __restrict__ work) const {
		//assert(sz == v.sz);
		int i;
		float s = 0.;
		for(i = 0; i < sz; ++i) {
			work[i] = (mem[i] - v.mem[i]) * (mem[i] - v.mem[i]);
		}
		for(i = 0; i < sz; ++i) {
			s += work[i];
		}
		return s;
	}
	
	/* This version isn't vectorizable */
	float distsq(const floatvec &v) const {
		//assert(sz == v.sz);
		float s = 0.;
		for(int i = 0; i < sz; ++i) {
			s += (mem[i] - v.mem[i]) * (mem[i] - v.mem[i]);
		}
		return s;
	}
	
	float dist(const floatvec &v, float *work) const {
		return sqrt(distsq(v, work));
	}

	float dist(const floatvec &v) const {
		return sqrt(distsq(v));
	}
	
	void zero() {
		memset(mem, 0, sz * sizeof(float));
	}
	
	void replace(float v1, float v2) {
		for (int i = 0; i < sz; ++i) {
			if (mem[i] == v1) {
				mem[i] = v2;
			}
		}
	}
	
	void resize(int size) {
		sz = size;
		mem = (float*) realloc(mem, sz * sizeof(float));
	}
	
	void extend(const floatvec &v) {
		resize(sz + v.sz);
		memcpy(mem + sz, v.mem, v.sz * sizeof(float));
	}
	
	void combine(const floatvec &v1, const floatvec &v2) {
		assert(sz == v1.sz + v2.sz);
		memcpy(mem, v1.mem, sizeof(float) * v1.sz);
		memcpy(mem + v1.sz, v2.mem, sizeof(float) * v2.sz);
	}
	
	float sum() const {
		float s = 0.0;
		for (int i = 0; i < sz; ++i) {
			s += mem[i];
		}
		return s;
	}
	
	float magnitude() const {
		float m = 0.;
		for (int i = 0; i < sz; ++i) {
			m += mem[i] * mem[i];
		}
		return sqrt(m);
	}
	
	floatvec unit() const {
		floatvec c(sz);
		float m = magnitude();
		for (int i = 0; i < sz; ++i) {
			c[i] = mem[i] / m;
		}
		return c;
	}
	
	float &operator[](int i) {
		assert(i >= 0 && i < sz);
		return mem[i];
	}
	
	float operator[](int i) const {
		assert(i >= 0 && i < sz);
		return mem[i];
	}
	
	floatvec slice(int i, int j) const {
		assert(0 <= i && i <= j && j < sz);
		floatvec s(j - i);
		memcpy(s.mem, &mem[i], (j - i) * sizeof(float));
		return s;
	}
	
	void operator=(const floatvec &v) {
		if (sz != v.sz) {
			sz = v.sz;
			free(mem);
			mem = (float*) malloc(sizeof(float) * sz);
		}
		memcpy(mem, v.mem, sz * sizeof(float));
	}
	
	void operator=(const std::vector<float> &v) {
		if (sz != v.size()) {
			sz = v.size();
			free(mem);
			mem = (float*) malloc(sizeof(float) * sz);
		}
		std::copy(v.begin(), v.end(), mem);
	}
	
	floatvec operator+(const floatvec &v) const {
		floatvec c(sz);
		for(int i = 0; i < sz; ++i) {
			c.mem[i] = mem[i] + v.mem[i];
		}
		return c;
	}
	
	void operator+=(const floatvec &v) {
		//assert(sz == v.sz);
		for(int i = 0; i < sz; ++i) {
			mem[i] += v.mem[i];
		}
	}
	
	floatvec operator-(const floatvec &v) const {
		floatvec c(sz);
		for(int i = 0; i < sz; ++i) {
			c.mem[i] = mem[i] - v.mem[i];
		}
		return c;
	}
	
	void operator-=(const floatvec &v) {
		//assert(sz == v.sz);
		for(int i = 0; i < sz; ++i) {
			mem[i] -= v.mem[i];
		}
	}
	
	floatvec operator*(float v) const {
		floatvec c(sz);
		for(int i = 0; i < sz; ++i) {
			c.mem[i] = mem[i] * v;
		}
		return c;
	}
	
	floatvec operator*(const floatvec &v) const {
		floatvec c(sz);
		for(int i = 0; i < sz; ++i) {
			c.mem[i] = mem[i] * v.mem[i];
		}
		return c;
	}
	
	void operator*=(const floatvec &v) {
		//assert(sz == v.sz);
		for(int i = 0; i < sz; ++i) {
			mem[i] *= v.mem[i];
		}
	}
	
	
	void operator*=(float v) {
		for(int i = 0; i < sz; ++i) {
			mem[i] *= v;
		}
	}
	
	floatvec operator/(const floatvec &v) const {
		floatvec c(sz);
		for(int i = 0; i < sz; ++i) {
			c.mem[i] = mem[i] / v.mem[i];
		}
		return c;
	}
	
	floatvec operator/(float v) const {
		floatvec c(sz);
		for(int i = 0; i < sz; ++i) {
			c.mem[i] = mem[i] / v;
		}
		return c;
	}
	
	void operator/=(const floatvec &v) {
		//assert(sz == v.sz);
		for(int i = 0; i < sz; ++i) {
			mem[i] /= v.mem[i];
		}
	}
	
	void operator/=(float v) {
		for(int i = 0; i < sz; ++i) {
			mem[i] /= v;
		}
	}
	
private:
	float * __restrict__ mem;
	int sz;
};

std::ostream &operator<<(std::ostream &os, const floatvec &v);

#endif
