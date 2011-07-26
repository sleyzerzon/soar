#include "filter.h"
#include "bbox.h"
#include "scene.h"
#include "linalg.h"

class ontop_filter : public map_filter<bool> {
public:
	ontop_filter(filter_input *input) : map_filter<bool>(input) {}

	bool compute(filter_param_set *params, bool &result, bool adding) {
		filter_val *tfv, *bfv;
		sgnode *tn, *bn;
		ptlist tp, bp;
		vec3 tmin, tmax, bmin, bmax;
		
		if (!get_filter_param(this, params, "top", tn) || 
		    !get_filter_param(this, params, "bottom", tn))
		{
			return false;
		}
		
		tn->get_world_points(tp);
		bn->get_world_points(bp);
		
		bbox tbox(tp), bbox(bp);
		bbox.get_vals(bmin, bmax);
		tbox.get_vals(tmin, tmax);
		return new filter_val_c<bool>(bbox.intersects(tbox) && tmin[2] == bmax[2]);
	}
};

filter* _make_ontop_filter_(scene *scn, filter_input *input) {
	return new ontop_filter(input);
}
