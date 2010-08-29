#include <iostream>
#include "../bbox.h"

using namespace std;

int main() {
	vec3 pt1(0.0, 1.0, 2.0);
	vec3 pt2(-1.0, 3.0, 10.0);
	vec3 pt3(-10, -10, -10);
	vec3 pt4(10, 10, 10);
	ptlist pts;
	pts.push_back(pt1);
	
	bbox bb1(pts);
	
	cout << bb1 << endl;
	
	bb1.include(pt2);
	
	cout << bb1 << endl;
	
	bbox bb2(pt3);
	
	cout << bb2 << endl;
	
	bb2.include(pt4);
	
	cout << bb2 << endl;
	
	cout << bb1.intersects(bb2) << endl;
	
	return 0;
}
