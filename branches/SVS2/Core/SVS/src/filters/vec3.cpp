#include <string>
#include <sstream>
#include "filter.h"

using namespace std;

class vec3_filter : public filter {
public:
	vec3_filter(filter *f[])
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
		vec3 r;
		for (int i = 0; i < 3; ++i) {
			if (cf[i] && !get_double_filter_result_value(this, cf[i], r[i])) {
				stringstream err;
				err << "parameter " << i << " is not a double";
				set_error(err.str());
				return NULL;
			}
		}
		return new vec3_filter_result(r);
	}

private:
	filter *cf[3];
	filter_container c;
};

filter *_make_vec3_filter_(scene *scn, const filter_params &p) {
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
	return new vec3_filter(fs);
}
