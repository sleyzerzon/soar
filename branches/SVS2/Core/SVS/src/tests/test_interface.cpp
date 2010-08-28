#include <iostream>
#include <sstream>
#include "test_interface.h"

using namespace std;

test_interface::test_interface() {
	idcounter = 1;
}

wme_hnd test_interface::make_str_wme(sym_hnd id, string attr, string val) {
	if (id >= idcounter) {
		cout << "ACCESSING NON-EXISTENT IDENTIFIER " << id << endl;
	}
	stringstream ss;
	ss << "id" << id;
	wme w = { ss.str(), attr, val };
	wmes.push_back(w);
	cout << "CREATING WME " << wmes.size() << " (" << ss.str() << " " << attr << " " << val << ")" << endl;
	return wmes.size() - 1;
}

sym_wme_pair test_interface::make_id_wme(sym_hnd id, string attr) {
	sym_wme_pair p;
	stringstream ss;
	string idstr, valstr;
	
	ss << "id" << id;
	idstr = ss.str();
	ss.clear();
	ss << "id" << idcounter;
	valstr = ss.str();
	
	p.first = idcounter;
	cout << "CREATING IDENTIFIER id" << idcounter << endl;
	idcounter++;
	
	wme w = { idstr, attr, valstr };
	wmes.push_back(w);
	cout << "CREATING WME " << wmes.size() << " (" << idstr << " " << attr << " " << valstr << ")" << endl;
	p.second = wmes.size() - 1;
	return p;
}

void test_interface::remove_wme(wme_hnd w) {
	if (w < 0 || w >= wmes.size()) {
		cout << "TRYING TO REMOVE NON-EXISTENT WME id" << w << endl;
	} else {
		cout << "REMOVING WME " << w << " (" << wmes[w].id << " " << wmes[w].attr << " " << wmes[w].val << ")" << endl;
		wmes[w].id   = "XXX";
		wmes[w].attr = "XXX";
		wmes[w].val  = "XXX";
	}
}

sym_hnd test_interface::get_scene_root() {
	cout << "GETTING SCENE ROOT id0" << endl;
	return 0;
}

void test_interface::add_line(string &line) {
	input_lines.push_back(line);
}

bool test_interface::get_env_line(string &line) {
	if (input_lines.size() == 0) {
		return false;
	}
	line = input_lines.front();
	input_lines.pop_front();
	return true;
}
