/* A point list with a single point */

#include <string>
#include "filter.h"

using namespace std;

class singleton_filter : public filter {
public:
	singleton_filter(filter *f[])
	: c(this)
	{
		for (int i = 0; i < 3; ++i) {
			cf[i] = f[i];
			if (cf[i] != NULL) {
				c.add(cf[i]);
			}
		}
	}
	
	filter_result *calc_result() {
		ptlist l;
		vec3 r;
		for (int i = 0; i < 3; ++i) {
			if (cf[i] && !get_double_filter_result_value(this, cf[i], r[i])) {
				return NULL;
			}
		}
		l.push_back(r);
		return new ptlist_filter_result(l);
	}

private:
	filter *cf[3];
	filter_container c;
};

filter *_make_singleton_filter_(scene *scn, const filter_params &p) {
	int i;
	filter_params::const_iterator j;
	filter *fs[] = {NULL, NULL, NULL};
	const char *pnames[] = {"x", "y", "z"};
	
	for (i = 0; i < 3; ++i) {
		j = p.find(string(pnames[i]));
		if (j != p.end()) {
			fs[i] = j->second;
		}
	}
	return new singleton_filter(fs);
}
