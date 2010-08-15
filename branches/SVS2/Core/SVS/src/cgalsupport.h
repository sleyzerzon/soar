#ifndef CGALSUPPORT_H
#define CGALSUPPORT_H

/* typedefs and utility functions for using CGAL */

#include <assert.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/algorithm.h>

#include <CGAL/Aff_transformation_3.h>
#include <CGAL/aff_transformation_tags.h>

#include <CGAL/Convex_hull_traits_3.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/Bbox_3.h>
#include <list>
#include <algorithm>

//typedef CGAL::Exact_predicates_inexact_constructions_kernel CGALKernel;
typedef CGAL::Simple_cartesian<double>                       CGALKernel;
typedef CGALKernel::Point_3                                  Point3;
typedef CGAL::Vector_3<CGALKernel>                           Vector3;
typedef CGAL::Bbox_3                                         Bbox3;
typedef CGAL::Polyhedron_3<CGALKernel>                       Poly3;
typedef CGAL::Aff_transformation_3<CGALKernel>               Transform3;

typedef CGAL::Convex_hull_traits_3<CGALKernel>::Polyhedron_3 ConvexPoly3;
typedef ConvexPoly3::Vertex_iterator                         VertexIter;

template<class InputIter>
ConvexPoly3* hull_of_hulls(InputIter begin, InputIter end) {
	std::list<Point3> pts;
	std::insert_iterator<std::list<Point3> > ii(pts, pts.begin());
	CGAL::Object* hull = new CGAL::Object();
	ConvexPoly3* result;

	int npts = 0;
	for (; begin != end; ++begin) {
		npts += (**begin).size_of_vertices();
		std::copy((**begin).points_begin(), (**begin).points_end(), ii);
	}
	assert(pts.size() == npts);
	CGAL::convex_hull_3(pts.begin(), pts.end(), *hull);
	result = const_cast<ConvexPoly3*>(CGAL::object_cast<ConvexPoly3>(hull));
	if (!result) {
		delete hull;
		return NULL;
	}
	return result;
}

template<class InputIter, class OutputIter>
void combine_pts(InputIter begin, InputIter end, OutputIter out) {
	InputIter pi;
	for (pi = begin; pi != end; ++pi) {
		std::copy(pi->points_begin(), pi->points_end(), out);
	}
}

template<class InputIter>
ConvexPoly3* convex_hull(InputIter begin, InputIter end) {
	CGAL::Object* hull = new CGAL::Object();
	ConvexPoly3* result;
	CGAL::convex_hull_3(begin, end, *hull);
	result = CGAL::object_cast<ConvexPoly3>(hull);
	if (!result) {
		delete hull;
		return NULL;
	}
	return result;
}

inline Transform3 euler_ypr_transform(Vector3 &ypr) {
	double y = ypr.x(); double p = ypr.y(); double r = ypr.z();
	
	Transform3 ry( cos(y), -sin(y),     0.0,
	               sin(y),  cos(y),     0.0,
	                  0.0,     0.0,     1.0);
	
	Transform3 rp( cos(p),     0.0,  sin(p),
	                  0.0,     1.0,     0.0,
	              -sin(y),     0.0,  cos(p));
	
	Transform3 rr(    1.0,     0.0,     0.0,
	                  0.0,  cos(r), -sin(r),
	                  0.0,  sin(r),  cos(r));
	
	return ry * rp * rr;
}

inline Transform3 scaling_transform(Vector3 &s) {
	return Transform3( s.x(),   0.0,   0.0,
	                   0.0,   s.y(),   0.0,
	                   0.0,     0.0, s.z() );
}

inline Bbox3 bbox_of_poly(Poly3 &p) {
	return Bbox3(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
}

#endif
