#include "BFS.h"

#include <iostream>
#include <vector>
#include <algorithm>

BFS::BFS(Point rootLoc) {
  idCounter = 0;
  combined = false;
  createNode(NULL, rootLoc);
}

BFS::~BFS() {
  if (!combined) {
    for(map<int, BFSNode*>::iterator
        i  = id2Node.begin();
        i != id2Node.end();
        ++i)
    {
      delete i->second;
    }
  }
  // otherwise let the search that took over this one take care of deleting 
}

void BFS::createNode(BFSNode* par, Point loc) {
  assert(loc2Node.find(loc) == loc2Node.end());
  BFSNode* newNode = new BFSNode(idCounter++, par, loc);

  id2Node[newNode->id] = newNode;
  loc2Node[loc] = newNode;

  if (par != NULL) {
    par->children.push_back(newNode);
  }
}

void BFS::findCycle(BFSNode* n1, BFSNode* n2, list<Point>& cycle) {
  cycle.clear();
  vector<int> trace;

  BFSNode* n = n1;
  while (n != NULL) {
    trace.push_back(n->id);
    n = n->par;
  }

  n = n2;
  while (n != NULL) {
    trace.push_back(n->id);
    n = n->par;
  }

  sort(trace.begin(), trace.end());

  int meet = -1;
  vector<int>::iterator i = trace.begin();
  vector<int>::iterator j = i++;

  for(; i != trace.end(); ++i, ++j) {
    if (*i == *j) {
      meet = *i;
      break;
    }
  }

  assert(meet >= 0);

  n = n1;
  while (n->id != meet) {
    cycle.push_back(n->loc);
    // this effectively isolates the subtrees that grow down from this path
    // so that next time when we are searching for the meet in those trees,
    // we don't have to search all the way back up to the real root
    BFSNode* p = n->par;
    n->par = NULL;
    n = p;
  }
  
  n = n2;
  while (n->id != meet) {
    cycle.push_front(n->loc);
    BFSNode* p = n->par;
    n->par = NULL;
    n = p;
  }
  
  assert(n->id == meet);
  cycle.push_front(n->loc);

  // this completes the cycle
}

bool BFS::insertEdge(Point p1, Point p2, list<Point>* cycle) {
  map<Point, BFSNode*>::iterator i1 = loc2Node.find(p1);
  map<Point, BFSNode*>::iterator i2 = loc2Node.find(p2);

  if (i1 != loc2Node.end() && i2 != loc2Node.end()) {
    // found cycle
    findCycle(i1->second, i2->second, *cycle);
    return true;
  }
  else if (i1 != loc2Node.end()) {
    cout << "CREATING NODE" << endl;
    createNode(i1->second, p2);
  }
  else if (i2 != loc2Node.end()) {
    cout << "CREATING NODE" << endl;
    createNode(i2->second, p1);
  }
  else {
    cout << "AT FAULT: (" << p1.x << "," << p1.y << ") (" << p2.x << ", " << p2.y << ")" << endl;
    assert(false);
  }

  return false;
}

void BFS::takeOver(BFS* other, Point myPoint, Point otherPoint) {
  assert(loc2Node.find(myPoint) != loc2Node.end());
  assert(other->loc2Node.find(otherPoint) != other->loc2Node.end());

  BFSNode* par = loc2Node.find(myPoint)->second;
  BFSNode* child = other->loc2Node.find(otherPoint)->second;

  other->combined = true;

  for(map<int, BFSNode*>::iterator
      i  = other->id2Node.begin();
      i != other->id2Node.end();
      ++i)
  {
    BFSNode* n = i->second;
    n->id = idCounter++;
    id2Node[n->id] = n;
    loc2Node[n->loc] = n;
  }
  child->rotate(par);
}
