#include <iostream>
#include <SOLID/SOLID.h>

using namespace std;

DT_Bool collision_callback(void *client_data, void *obj1, void *obj2, const DT_CollData *coll_data) {
	cout << "COLLISION" << endl;
	return DT_CONTINUE;
}

DT_ShapeHandle make_cube() {
	float cube[][3] = {
		{0., 0., 0.},
		{1., 0., 0.},
		{1., 1., 0.},
		{0., 1., 0.},
		{0., 0., 1.},
		{1., 0., 1.},
		{1., 1., 1.},
		{0., 1., 1.},
	};
	DT_ShapeHandle s = DT_NewComplexShape(NULL);
	DT_Begin();
	for (int i = 0; i < 8; ++i) {
		DT_Vertex(cube[i]);
	}
	DT_End();
	DT_EndComplexShape();
	return s;
}

int main(int argc, char *argv[]) {
	DT_SceneHandle scene = DT_CreateScene();
	DT_RespTableHandle table = DT_CreateRespTable();
	DT_ResponseClass cls = DT_GenResponseClass(table);
	
	DT_AddDefaultResponse(table, collision_callback, DT_SIMPLE_RESPONSE, NULL);
	DT_ObjectHandle a = DT_CreateObject(NULL, make_cube());
	DT_ObjectHandle b = DT_CreateObject(NULL, make_cube());
	DT_AddObject(scene, a);
	DT_AddObject(scene, b);
	DT_SetResponseClass(table, a, cls);
	DT_SetResponseClass(table, b, cls);
	cout << DT_Test(scene, table) << endl;
	
	return 0;
}
