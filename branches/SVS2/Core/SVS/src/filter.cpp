#include <sstream>
#include <iterator>
#include <utility>

#include "filter.h"

using namespace std;

/*
 Example input:
 
 (<ot> ^type on-top ^a <ota> ^b <otb>)
 (<ota> ^type node ^name box1)
 (<otb> ^type node ^name box2)
*/
filter *parse_filter_spec(soar_interface *si, Symbol *root, scene *scn) {
	wme_list children, params;
	wme_list::iterator i;
	Symbol* cval;
	string strval, pname, ftype, itype;
	long intval;
	float floatval;
	filter_input *input;
	bool fail;
	filter *f;
	
	fail = false;
	si->get_child_wmes(root, children);
	for (i = children.begin(); i != children.end(); ++i) {
		if (!si->get_val(si->get_wme_attr(*i), pname)) {
			continue;
		}
		cval = si->get_wme_val(*i);
		if (pname == "type") {
			if (!si->get_val(cval, ftype)) {
				return NULL;
			}
		} else if (pname == "input-type") {
			if (!si->get_val(cval, itype)) {
				return NULL;
			}
		} else if (pname != "status" && pname != "result") {
			params.push_back(*i);
		}
	}
	
	if (itype == "concat") {
		input = new concat_filter_input();
	} else if (params.size() == 0) {
		input = new null_filter_input();
	} else {
		input = new product_filter_input();
	}
	
	for (i = params.begin(); i != params.end(); ++i) {
		if (!si->get_val(si->get_wme_attr(*i), pname)) {
			continue;
		}
		cval = si->get_wme_val(*i);
		if (si->get_val(cval, strval)) {
			input->add_param(pname, new const_filter<string>(strval));
		} else if (si->get_val(cval, intval)) {
			input->add_param(pname, new const_filter<int>(intval));
		} else if (si->get_val(cval, floatval)) {
			input->add_param(pname, new const_filter<float>(floatval));
		} else {
			filter *cf;
			// must be identifier
			if ((cf = parse_filter_spec(si, cval, scn)) == NULL) {
				fail = true;
				break;
			}
			input->add_param(pname, cf);
		}
	}
	
	if (!fail) {
		f = make_filter(ftype, scn, input);
	}
	
	if (fail || ftype == "" || f == NULL) {
		delete input;
		return NULL;
	}
	return f;
}
