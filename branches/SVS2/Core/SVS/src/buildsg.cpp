/* Read SGEL from stdin and print world coordinates of all points to
 * stdout.
 */

#include <iostream>
#include <iomanip>
#include <list>
#include "cgalsupport.h"
#include "nsg_node.h"
#include "sgel_interp.h"

using namespace std;

void print_structure(sg_node *root) {
	if (!root->is_group()) {
		cout << root->get_name();
	} else {
		cout << "(" << root->get_name() << " ";
		for (int i = 0; i < root->get_nchilds(); ++i) {
			print_structure(root->get_child(i));
			cout << " ";
		}
		cout << ")";
	}
}

void print_points(sg_node *root) {
	sg_node *c;
	list<Point3> pts;

	//cout << setiosflags(ios::fixed) << setprecision(3);
	for (int i = 0; i < root->get_nchilds(); ++i) {
		c = root->get_child(i);
		if (c->is_group()) {
			print_points(c);
		} else {
			c->get_world_points(pts);
			cout << c->get_name() << endl;
			copy(pts.begin(), pts.end(), ostream_iterator<Point3>(cout, "\n"));
		}
	}
}

int main(int argc, char *argv[]) {
	int lineno = 1, pos;
	string line;
	nsg_node root("0");
	sgel_interp interp(&root);
	
	while(getline(cin, line)) {
		if ((pos = interp.parse_line(line)) >= 0) {
			cout << "Error at " << lineno << ":" << pos << endl;
			return 1;
		}
	}
	//print_structure(&root); cout << endl;
	print_points(&root);
}
