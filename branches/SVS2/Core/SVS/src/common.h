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
#include <map>

#include "linalg.h"

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

template <typename A, typename B>
inline bool map_get(const std::map<A, B> &m, const A &key, B &val) {
	typename std::map<A, B>::const_iterator i = m.find(key);
	if (i == m.end()) {
		return false;
	}
	val = i->second;
	return true;
}

template <typename A, typename B>
inline bool map_pop(std::map<A, B> &m, const A &key, B &val) {
	typename std::map<A, B>::iterator i = m.find(key);
	if (i == m.end()) {
		return false;
	}
	val = i->second;
	m.erase(i);
	return true;
}

class floatvec {
public:
	floatvec() : sz(0), mem(NULL) {}
	
	floatvec(const floatvec &v) : sz(v.sz) { 
		mem = (float *) malloc(sz * sizeof(float));
		memcpy(mem, v.mem, sizeof(float) * sz);
	}
	
	floatvec(int sz) : sz(sz) {
		mem = (float *) malloc(sz * sizeof(float));
		zero();
	}
	
	floatvec(int sz, float *data) : sz(sz) {
		mem = (float *) malloc(sz * sizeof(float));
		memcpy(mem, data, sizeof(float) * sz);
	}
	
	floatvec(const std::vector<float> &v) {
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

	void set_indices(const std::vector<int> &inds, const floatvec &v) {
		assert(v.size() == inds.size());
		for (int i = 0; i < inds.size(); ++i) {
			assert(0 <= inds[i] && inds[i] < sz);
			mem[inds[i]] = v[i];
		}
	}
	
	void resize(int size) {
		sz = size;
		mem = (float*) realloc(mem, sz * sizeof(float));
	}
	
	void extend(const floatvec &v) {
		int oldsz = sz;
		resize(sz + v.sz);
		memcpy(mem + oldsz, v.mem, v.sz * sizeof(float));
	}
	
	void combine(const floatvec &v1, const floatvec &v2) {
		assert(sz == v1.sz + v2.sz);
		memcpy(mem, v1.mem, sizeof(float) * v1.sz);
		memcpy(mem + v1.sz, v2.mem, sizeof(float) * v2.sz);
	}
	
	void randomize(const floatvec &min, const floatvec &max) {
		assert(sz == min.sz && sz == max.sz);
		for (int i = 0; i < sz; ++i) {
			mem[i] = min[i] + (((float) rand()) / RAND_MAX) * (max[i] - min[i]);
		}
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
		assert(0 <= i && i <= j && j <= sz);
		floatvec s(j - i);
		memcpy(s.mem, &mem[i], (j - i) * sizeof(float));
		return s;
	}
	
	floatvec slice(const std::vector<int> &inds) const {
		floatvec s(inds.size());
		for (int i = 0; i < inds.size(); ++i) {
			assert(0 <= inds[i] && inds[i] < sz);
			s[i] = mem[inds[i]];
		}
		return s;
	}

	void operator=(const floatvec &v) {
		if (sz != v.sz) {
			sz = v.sz;
			mem = (float*) realloc(mem, sizeof(float) * sz);
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
		assert(sz == v.sz);
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
	
	bool operator==(const floatvec &v) const {
		if (sz != v.sz) {
			return false;
		}
		for (int i = 0; i < sz; ++i) {
			if (mem[i] != v.mem[i]) {
				return false;
			}
		}
		return true;
	}
	
	bool operator<(const floatvec &v) const {
		for (int i = 0; i < sz; ++i) {
			if (i >= v.sz) {
				// all common values equal, v is shorter
				return false;
			}
			if (mem[i] < v.mem[i]) {
				return true;
			} else if (mem[i] > v.mem[i]) {
				return false;
			}
		}
		if (v.sz > sz) {
			// all common values equal, v is longer
			return true;
		}
		// vectors are identical
		return false;
	}
	
	bool operator<=(const floatvec &v) const {
		return operator==(v) || operator<(v);
	}
	
	bool operator>(const floatvec &v) const {
		return !operator<=(v);
	}
	
	bool operator>=(const floatvec &v) const {
		return !operator<(v);
	}
	
	friend std::ostream &operator<<(std::ostream &os, const floatvec &v);
	
private:
	float *mem;
	int sz;
};

class namedvec {
public:
	namedvec() {}
	
	namedvec(const namedvec &v) 
	: vals(v.vals), name2ind(v.name2ind), ind2name(v.ind2name)
	{}
	
	namedvec(std::vector<std::string> &names) 
	: vals(names.size())
	{
		for (int i = 0; i < names.size(); ++i) {
			name2ind[names[i]] = i;
			ind2name[i] = names[i];
		}
	}
	
	void set_name(int index, const std::string &name) {
		name2ind[name] = index;
		ind2name[index] = name;
	}
	
	void set_names(const std::vector<std::string> &names) {
		for (int i = 0; i < names.size(); ++i) {
			name2ind[names[i]] = i;
			ind2name[i] = names[i];
		}
	}
	
	void add_name(const std::string &name, float v) {
		name2ind[name] = vals.size();
		ind2name[vals.size()] = name;
		vals.resize(vals.size() + 1);
		vals[vals.size() - 1] = v;
	}
	
	bool get_name(int index, std::string &name) const {
		return map_get(ind2name, index, name);
	}
	
	bool get_by_name(const std::string &name, float &val) const {
		int index;
		if (!map_get(name2ind, name, index)) {
			return false;
		}
		val = vals[index];
		return true;
	}
	
	bool set_by_name(const std::string &name, float val) {
		int index;
		if (!map_get(name2ind, name, index)) {
			return false;
		}
		vals[index] = val;
		return true;
	}
	
	bool congruent(const namedvec &v) {
		return name2ind == v.name2ind;
	}
	
	void operator=(const namedvec &v) {
		vals = v.vals;
		name2ind = v.name2ind;
		ind2name = v.ind2name;
	}
	
	int size() const {
		return vals.size();
	}
	
	void get_names(std::vector<std::string> &names) const {
		std::map<int, std::string>::const_iterator i;
		
		names.clear();
		names.reserve(ind2name.size());
		for (i = ind2name.begin(); i != ind2name.end(); ++i) {
			names.push_back(i->second);
		}
	}
	
	void clear() {
		vals.resize(0);
		name2ind.clear();
		ind2name.clear();
	}
	
	floatvec vals;
	
private:
	std::map<std::string, int> name2ind;
	std::map<int, std::string> ind2name;
};

std::ostream &operator<<(std::ostream &os, const namedvec &v);

vec3 calc_centroid(const ptlist &pts);

/*
 Calculate the maximum difference between points in two point clouds in
 the direction of u.
 
  a         b
  .<-- d -->.        returns a positive d
 --------------> u
 
  b         a
  .<-- d -->.        returns a negative d
 --------------> u
*/
float dir_separation(const ptlist &a, const ptlist &b, const vec3 &u);

#endif
