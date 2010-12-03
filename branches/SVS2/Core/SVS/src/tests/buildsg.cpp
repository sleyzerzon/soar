/* Read SGEL from stdin and print world coordinates of all points to
 * stdout.
 */

#include <iostream>
#include <iomanip>
#include <iterator>
#include <list>
#include "../scene.h"
#include "../linalg.h"

using namespace std;

int parse_state_update(string s, scene *scn);

void print_structure(sg_node *root) {
	if (!root->is_group()) {
		cout << root->get_name();
	} else {
		cout << "(" << root->get_name() << " ";
		for (int i = 0; i < root->num_children(); ++i) {
			print_structure(root->get_child(i));
			cout << " ";
		}
		cout << ")";
	}
}

void print_points(sg_node *root) {
	sg_node *c;
	ptlist pts;

	//cout << setiosflags(ios::fixed) << setprecision(3);
	for (int i = 0; i < root->num_children(); ++i) {
		c = root->get_child(i);
		if (c->is_group()) {
			print_points(c);
		} else {
			c->get_world_points(pts);
			cout << c->get_name() << endl;
			copy(pts.begin(), pts.end(), ostream_iterator<vec3>(cout, "\n"));
		}
	}
}

int main(int argc, char *argv[]) {
	int lineno = 1, pos;
	string line;
	scene scn("test", "0", false);
	
	while(getline(cin, line)) {
		if ((pos = parse_state_update(line, &scn)) >= 0) {
			cout << "Error at " << lineno << ":" << pos << endl;
			return 1;
		}
	}
	//print_structure(&root); cout << endl;
	print_points(scn.get_node("0"));
}
