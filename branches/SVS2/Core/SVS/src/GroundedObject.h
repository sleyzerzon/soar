
// A GroundedObject is a shape that has a definite location in a coordinate system,
// like the spatial scene. GroundedObject are implicit in the scene graph, this is
// an explicit representation. These are cached in SVSObjects for efficiency.

// This is a base class that will get specialized for each interpretation:
// point2/3,bbox2/3,poly2/3

// I'm assuming for now that local geometry is never needed (i.e., access to
// CGAL structures where the points are the points in the local FOR as the
// scene graph stores). If that is needed, copy the polyhedron and
// transform by the inverse of Wm3 global transform.
//
// In SVS2, all objects will have the local FOR origin at their centroid. This will require
// some preprocessing of imported models before they can be used, but
// simplifies things internally.

class GroundedObject {  
  public:

    // ALL of these are pure virtual:
    
    // constructor taking a SVSObject*: this poly will be the convex hull of the
    // scene graph below the given node, at the global location.
    virtual GroundedObject(SVSObject* shape) = 0;

    // constructor taking a Spatial* and an SVSObject*: the poly will be the
    // convex hull of the scene graph below the given node, ground at its
    // location in the FOR of the frameSource
//    virtual GroundedObject(SVSObject* shape, SVSObject* frameSource) = 0;
//    DO WE NEED THIS?

    // constructor taking a (generic) GroundedObject
    virtual GroundedObject(GroundedObject*) = 0;
    // The last will be used, e.g., to get a centroid:
    // GroundedObjectPoint3 centroid(Some3dPolyhedronGroundedObject);
    
    // in addition, each derived must have its own constructor taking, E.g., a
    // CGALPolygon2 for the 2d polygon object class
    
    // Move the shape through space
    // This is used to efficiently update grounded shapes (e.g., do not recalculate the
    // convex hull of the Wm3 structure at every movement).
    virtual void setGlobalTransformation(Transformation newTx) = 0;

    // Get the grounded direction corresponding to the "front" of the object.
    // The local FOR of the object has an origin at the centroid, and a front
    // direction pointing to 0,1,0. 
    //
    // This is implicit in the transform
    //virtual CGALDirection getIntrinsicFront() = 0; May be unnecessary

    // Build a WM3 geometry object, where the coordinates are in the FOR where
    // 0,0,0 is the centroid and 0,1,0 points to the front.
    // Caller owns the new object.
    virtual Geometry* makeNewGeometry() = 0;

    // Build a WM3 transformation, going from the global FOR to place the
    // object at its grounded location.
    virtual Transformation getGlobalTransformation();

  private:
    Transformation transform;

};

// TODO: fill these all in
// they should all cache internal state and update when transform() is called

class GroundedObjectPolyhedron3d : public GroundedObject {
};
class GroundedObjectPolygon2d : public GroundedObject {
};
class GroundedObjectBoundingBox3d : public GroundedObject {
};
class GroundedObjectBoundingBox2d : public GroundedObject {
};
class GroundedObjectPoint3d : public GroundedObject {
};
class GroundedObjectPoint2d : public GroundedObject {
};
