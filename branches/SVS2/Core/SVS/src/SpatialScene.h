#ifndef SPATIALSCENE_H
#define SPATIALSCENE_H

#include "SVSSystem.h"
#include "SVS.h"
#include "SVSSceneGraphBuilder.h"

namespace svs {
  class SpatialScene {

  public:
  
    struct DeadNodeData {
      SVSSGNode::Type type;
      string classId;
      bool imagined;
      
      DeadNodeData(){}
      DeadNodeData(SVSSGNode::Type _type, string _classId, bool _imagined) {
        type = _type;
        classId = _classId;
        imagined = _imagined;
      }
    };

    SpatialScene();
    ~SpatialScene();

    bool setView(Direction position, bool approximate = true, double distance = -1, string frameSource = "", string referenceObject = "");
    bool setView(string sourceObject);
    bool setView(string sourceObject, string targetObject);

    void getCameraState(Matrix3f& axes, Vector3f& location);
    void setCameraState(Matrix3f& axes, Vector3f& location);

    void setBackground(SpatialPtr bgObj);

    static string getSceneInstanceId() {
      return ":scene:i";
    }

    void resetIteration();
    SpatialPtr nextIteration();
    int iterationDepth();

    NodePtr getSceneGraph() { return sceneGraph; }

    // interface used for LTM retrieval, SoarIO, and environment
    void addObject(string parent, vector<SVSSGNode*> nodes, bool real = false);
    void removeObject(string rootName);
    void updateTransformationOfObject(string objectId, Vector3f position);

    // lower-level interface: notify the scene that a node has changed
    // used by drive simulator, which updates position and rotation
    // DEPRECATED, instead, add a new (optional) parameter for a new rotation
    // in updateTransformationOfObject
    void changeNode(SVSSGNode* node);


    // used to get a copy of an object in the scene, like it was retrieved from
    // LTM
    vector<SVSSGNode*> retrieveObject(string id);

    bool expand(string rootName, bool allTheWay);
    bool collapse(string rootName);

    bool enableVisualization(string nodeName);
    bool disableVisualization(string nodeName);

    //void replace(string oldPropertyName, SVSSGNode* newProperty); disabling
    // 2/23/09, not called anywhere (although works AFAIK)


    map<string, SVSSGNode*>::iterator nodesBegin() {
      return nodes.begin();
    }
    map<string, SVSSGNode*>::iterator nodesEnd() {
      return nodes.end();
    }

    bool nodePresent(string id);
    SVSSGNode* getNode(string nodeName);

    // interface to input-link to report Scene contents
    vector< SVSSGNode* > getAddNodes();
    vector< SVSSGNode* > getChangeNodes();

    // remove needs the id of the removed node, since the ptr is no longer
    // valid by the point it is read
    map< string, DeadNodeData > getRemoveNodes();

    // interface to SoarIO to re-evaluate queries. This is different from the
    // above getChangeNodes():
    // this is only for Objects, not all nodes (since queries are about objects)
    // this is set recursively to the leafs, since changes above can impact the
    // coordinates of things below
    // this is cleared delibrately, since all queries need to get all the changes
    // (ChangeNodes are cleared when read)
    set< string > getChangedObjects();
    void clearChangedObjects();
    // also used for error reporting: if a command references an object that
    // is missing, that can be OK if it just changed (implying it was
    // removed this decision, which the agent hasn't had a chance to react to
    // yet)
    bool isChangedObject(string object);

    // debugging
    Spatial* getSpatial(string name);

  private:
    NodePtr sceneGraph;
    SVSSGTransform* SVSSceneGraph;
    CameraPtr camera;
    Renderer* renderer;
    Light* sceneLight;
    SVSSceneGraphBuilder* sceneGraphBuilder;

    SVS* svs;

    vector<SpatialPtr> iterationParents;
    vector<int> iterationChildNums; 
    bool iterationFresh;

    void setCameraParameters();
    void createSceneGraphStructure();

    void clear();

    void addNode(SVSSGNode* node);
    void removeNode(SVSSGNode* node);

    SVSSGObject* SVSSGObjectsNode;

    // map from ids to svssg nodes
    map<string, SVSSGNode*> nodes;

    // used so we can know which SVSSGObjects to update when the environment adds
    // a node
    map<SpatialPtr, SVSSGObject*> wm3Spatial2Object;

    // lists of changes to give to input-link
    // these are implemented as sets: we need to ensure nothing in the add or change
    // list is also being removed (and hence is already deleted), so we need fast
    // searching when adding removeNodes
    // in addition, it is legal for an object to be removed, and another with
    // the same id to be added in the same cycle, so removeNodes needs to also
    // be searchable, so false removals can be handled
    set< SVSSGNode* > addNodes;
    set< SVSSGNode* > changeNodes;
    map< string, DeadNodeData > removeNodes;


    void changeNodeToRoot(SVSSGNode* node);
    void changeNodeToLeaves(SVSSGNode* node);

    void changeNodeStructurally(SVSSGNode* node);

    set< string > changedObjects;
  };

} // namespace svs
#endif
