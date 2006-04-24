#include "include/GroupManager.h"
#include "include/general.h"
#include <iostream>

#include "SoarGameGroup.h"
#include "Sorts.h"
#include "SoarInterface.h"

using namespace std;

/*
  GroupManager.cpp
  SORTS project
  Sam Wintermute, 2006

*/


GroupManager::GroupManager() {
  // this default should be reflected in the agent's assumptions
  // (1024 = 32^2)
  groupingRadiusSquared = 1024;

  // the number of objects near the focus point to add
  // agent can change this, if it wishes to cheat
  numObjects = 5;
  
  focusX = (int) (sorts->OrtsIO->getMapXDim() / 2.0);
  focusY = (int) (sorts->OrtsIO->getMapYDim() / 2.0);

}
void GroupManager::updateVision() {
  prepareForReGroup();
    // prune empty groups (if units died)
    // prepare list of group categories that need to be reGrouped
    // recalculate the center member for groups that changed
  
  reGroup();
    // re-calculate the groups
    
  generateGroupData();
    // prune groups emptied during reGrouping
    // aggregate data about the groups
  
  adjustAttention();
    // determine which groups are attended to,
    // and send them to Soar
  
  updateFeatureMaps(false);
    // update feature maps, inhibiting groups attended to,
    // and send them to Soar
    
  return;
}

bool GroupManager::assignActions() {
  // through the sorts->getSoarInterface(), look for any new actions, and assign them to groups
  // actions have a list of params and a list of groups,
  // the first group (must exist) is the group the action will be applied to
    
  list <ObjectAction> newActions;
  
  sorts->SoarIO->getNewObjectActions(newActions);
  list <ObjectAction>::iterator actionIter = newActions.begin();
 
  list <SoarGameGroup*>::iterator groupIter;
  bool success = true;
  list<SoarGameGroup*> targetGroups;
  SoarGameGroup* sourceGroup;
  
  while (actionIter != newActions.end()){
    targetGroups.clear();
    list<SoarGameGroup*>& groups = (*actionIter).groups;
    groupIter = groups.begin();
    
    assert(groupIter != groups.end());
    // the first group is the group the action is applied to, it must exist
    
    sourceGroup = *groupIter;
    groupIter++;
    
    while (groupIter != groups.end()) {
      targetGroups.push_back(*groupIter);
      groupIter++;
    }
    
    success &= sourceGroup->assignAction(
            (*actionIter).type, (*actionIter).params, targetGroups);
    
    actionIter++;
  }

  return success;
}

void GroupManager::processVisionCommands() {
  cout << "processVision" << endl;
  // called when Soar changes the view window, wants to attend to an item
  // in a feature map, or changes a grouping parameter.

  // view window change:
  // call sorts->getFeatureMapManager()->changeViewWindow()
  // updateFeatureMaps(true)

  // attention shift (w/o view window shift):
  // call adjustAttention() to select the new groups
  // call updateFeatureMaps(false) to inhibit any newly-attended to groups

  // attention shift w/ view window shift:
  // sorts->getFeatureMapManager()->changeViewWindow()
  // adjustAttention()
  // updateFeatureMaps(true)

  // grouping change:
  // this is the same as updateVision, except we don't need prepareForReGroup,
  // since none of the objects in the world actually changed:
  // reGroup()
  // generateGroupData()
  // adjustAttention()
  // updateFeatureMaps(false)
  
  list<AttentionAction> actions;
  sorts->SoarIO->getNewAttentionActions(actions);

  int radius;
  SoarGameGroup* centerGroup;
        list<int>::iterator it;

  for (list<AttentionAction>::iterator i = actions.begin();
                                       i != actions.end();
                                       i++) {
    switch (i->type) {
      case AA_LOOK_LOCATION:
        assert(i->params.size() == 2); // x,y
        it = i->params.begin();
        focusX = *it;
        it++;
        focusY = *it;
        
        // recalc all center distances and rebuild the order of the groups
        remakeGroupSet();
        adjustAttention(); 

        break;
      case AA_LOOK_FEATURE:
        // attention shift (w/o view window shift)

        // first (and only) param is the sector number
        assert(i->params.size() == 1);
        centerGroup = sorts->featureMapManager->getGroup(i->fmName, 
                                                        *(i->params.begin()));
        if (centerGroup == NULL) {
          cout << "ERROR: sector " << *(i->params.begin()) << " of map " <<
            i->fmName << " is empty! Ignoring command\n";
          return;
        }
        
        // set focus point the center of the group
        centerGroup->getCenterLoc(focusX, focusY);

        // recalc all center distances and rebuild the order of the groups
        remakeGroupSet();
        adjustAttention(); 
        break;
      case AA_RESIZE:

        break;
      case AA_MOVE_LOCATION:
        break;
      case AA_MOVE_FEATURE:
        break;
      case AA_GROUPING_RADIUS:
      // grouping change:
      // this is the same as updateVision, except we don't need prepareForReGroup,
      // since none of the objects in the world actually changed:
        assert(i->params.size() == 1);
        radius = *(i->params.begin());
        radius *= radius;
        if (radius != groupingRadiusSquared) {
          groupingRadiusSquared = radius;
          setAllCategoriesStale();
          reGroup();
          generateGroupData();
          adjustAttention();
          updateFeatureMaps(false);
        }
        break;
      case AA_NUM_OBJECTS:
        assert(i->params.size() == 1);
        numObjects = *(i->params.begin());
        adjustAttention();
        break;
      case AA_NO_SUCH_ACTION:
        break;
      default:
        break;
    }
  }
}

void GroupManager::reGroup() {
  // iterate through staleGroupCategories set
  //  find all the groups of each type
  //  add the members to a big list, centers first
  //  keep a struct for each object:
  //    ptr to the obj, flag for if it has been assigned a group, ptr to group
  //  go through each obj1 in the list
  //    if not a group-center and not flagged, rm from old group and make a new group
  //    check each object (obj2) below in list:
  //      if objs are close and obj2 not flagged, flag obj2 and bring to same group
  //      if objs are close and obj2 flagged, check oldGroup flag
  //        merge groups, preferring to keep groups w/ oldgroup flag
  //        if neither is set, choice is arbitrary
  //        if both are set, prefer the larger group

  set<pair<string, int> >::iterator catIter = staleGroupCategories.begin();
  objectGroupingStruct objectData;
  list<SoarGameObject*> groupMembers;
  set<SoarGameGroup*>::iterator groupIter;
  list<SoarGameObject*>::iterator objectIter;
  
  list<objectGroupingStruct> groupingList;
  list<objectGroupingStruct> centerGroupingList;

  // save all the to-merge pairs in a list
  // do all the merges at the end
  // this prevents invalid groups in the list (groups are deleted after a merge)
  list<pair<SoarGameGroup*, SoarGameGroup*> > toMergeList;
  
  SoarGameObject* centerObject;

  while (catIter != staleGroupCategories.end()) {
  //  cout << "doing type " << catIter->first << endl;
    groupingList.clear();
    centerGroupingList.clear();
    
    for (groupIter = groups.begin(); groupIter != groups.end(); groupIter++) {
      if (not (*groupIter)->getSticky() and
              (*groupIter)->getCategory() == *catIter) {
   //     cout << "group " << (int) (*groupIter) << endl;
        // group is of the type we are re-grouping
        
        objectData.group = *groupIter;
        objectData.assigned = false;
        centerObject = (*groupIter)->getCenterMember();
    
        // centers are stored in a separate list
        objectData.object = centerObject;
        objectData.x = *centerObject->gob->sod.x;
        objectData.y = *centerObject->gob->sod.y;
        // oldGroup means the group has been around for at least one cycle
        objectData.oldGroup = (*groupIter)->isOld();
        
        centerGroupingList.push_back(objectData);
        objectData.oldGroup = false;
        groupMembers = (*groupIter)->getMembers();
        objectIter = groupMembers.begin();
        while (objectIter != groupMembers.end()) {
          if ((*objectIter) != centerObject){
            // don't add the center object to this list
            objectData.object = *objectIter;
            
            objectData.x = *(*objectIter)->gob->sod.x;
            objectData.y = *(*objectIter)->gob->sod.y;
            groupingList.push_back(objectData);
          }
          objectIter++;
        }
      }
      // else it was a group of a different type
    }
    // the lists are now built, centers in a separate list we will
    // treat as being "before" the other list
    
    // now follow the grouping procedure as outlined above
    bool obj1IsACenter = true;
    list<objectGroupingStruct>::iterator obj1StructIter, obj2StructIter;
    objectGroupingStruct obj1Struct;
    
    obj1StructIter = centerGroupingList.begin();
    if (obj1StructIter == centerGroupingList.end()) {
      // this really should not happen
      obj1StructIter = groupingList.begin();
      obj1IsACenter = false;
    }
    while (obj1StructIter != groupingList.end()) {
      obj1Struct = *obj1StructIter;
      
      if (not obj1IsACenter and not obj1Struct.assigned) {
        // make a new group for this object- no existing 
        // group has claimed it yet
        obj1Struct.group->removeUnit(obj1Struct.object);
        addGroup(obj1Struct.object);
        obj1Struct.group = obj1Struct.object->getGroup();
        obj1Struct.assigned = true;
        //cout << "XXX making new group " << (int) obj1Struct.group << endl; 
      }
     
      // iterate through all lower objects to see if they should join the group
      obj2StructIter = obj1StructIter;
      obj2StructIter++;
      if (obj2StructIter == centerGroupingList.end()) {
        obj2StructIter = groupingList.begin();
      }
      while (obj2StructIter != groupingList.end()) {
        if (squaredDistance(obj1Struct.x, obj1Struct.y, 
                            (*obj2StructIter).x, (*obj2StructIter).y)
            <= groupingRadiusSquared) {
          if ((*obj2StructIter).assigned) {
            // obj2 already has been grouped- groups should merge
            pair<SoarGameGroup*, SoarGameGroup*> groups;

            if (obj1Struct.oldGroup and not (*obj2StructIter).oldGroup) {
              // obj1's group isn't new, and 2's is, keep 1's group
              groups.second = obj1Struct.group;
              groups.first = (*obj2StructIter).group;
            }
            else if (not obj1Struct.oldGroup and (*obj2StructIter).oldGroup) {
              // vice versa
              groups.first = obj1Struct.group;
              groups.second = (*obj2StructIter).group;
            }
            else if (not obj1Struct.oldGroup and not (*obj2StructIter).oldGroup) {
              // arbitrary
              groups.first = obj1Struct.group;
              groups.second = (*obj2StructIter).group;
            }
            else {
              // both old- keep the bigger
              if (obj1Struct.group->getSize() > (*obj2StructIter).group->getSize()) {
                groups.second = obj1Struct.group;
                groups.first = (*obj2StructIter).group;
              }
              else {
                groups.first = obj1Struct.group;
                groups.second = (*obj2StructIter).group;
              }
            }
            toMergeList.push_back(groups);
         //   cout << "XXX will merge " << (int) groups.first << " -> " << (int) groups.second << endl;
          }
          else {
            // obj2 has not been assigned. Assign it to obj1's group.
         //   cout << "XXX obj from group " << (int) (*obj2StructIter).group <<
         //           " joining " << (int) obj1Struct.group << endl;
            (*obj2StructIter).assigned = true;
            (*obj2StructIter).group->removeUnit((*obj2StructIter).object);
            (*obj2StructIter).group = obj1Struct.group;
            (*obj2StructIter).group->addUnit((*obj2StructIter).object);
            (*obj2StructIter).oldGroup = obj1Struct.oldGroup;
            
          }
        //  cout << "grouped!" << endl;
        }
        else {
       //   cout << "not grouped!" << endl;
        }
        obj2StructIter++; 
        if (obj2StructIter == centerGroupingList.end()) {
          obj2StructIter = groupingList.begin();
        }
      }
      // jump the iterator between the two lists
      obj1StructIter++;
      if (obj1StructIter == centerGroupingList.end()) {
        obj1StructIter = groupingList.begin();
        obj1IsACenter = false;
      }
    }
    catIter++;
  } // end iterating through all the types that need re-grouping
  
  // do merges- always merge the first group to the second
  
  list<pair<SoarGameGroup*, SoarGameGroup*> >::iterator toMergeIter;
  list<pair<SoarGameGroup*, SoarGameGroup*> >::iterator toMergeIter2;

  // if two groups merge, we need to ensure that the subsumed group
  // does not have any outstanding merges
  toMergeIter = toMergeList.begin();
  while (toMergeIter != toMergeList.end()) {
    //cout << "groups " << (int)  (*toMergeIter).first << " and " << (int) (*toMergeIter).second << " will merge\n";
    if ((*toMergeIter).first == (*toMergeIter).second) {
      // do nothing- the groups were already merged
    }
    else {
      // merge first into second
      
      toMergeIter2 = toMergeIter;
      toMergeIter2++;
      while (toMergeIter2 != toMergeList.end()) {
        // replace all occurrences of the squashed group with the
        // new combined group
        if ((*toMergeIter2).first == (*toMergeIter).first) {
          (*toMergeIter2).first = (*toMergeIter).second;
        }
        if ((*toMergeIter2).second == (*toMergeIter).first) {
          (*toMergeIter2).second = (*toMergeIter).second;
        }
        toMergeIter2++;
      }

      (*toMergeIter).first->mergeTo((*toMergeIter).second);
    }
    
    toMergeIter++;
  }
 
  staleGroupCategories.clear();
  //cout << "XXX regroup done" << endl;
  return;
}

void GroupManager::generateGroupData() {
  // iterate through all the groups, if they are stale,
  // refresh them (re-calc stats)
 
  set<SoarGameGroup*>::iterator groupIter;
  list<set<SoarGameGroup*>::iterator> toErase;
  list<set<SoarGameGroup*>::iterator> toReinsert;

  for (groupIter = groups.begin(); groupIter != groups.end(); groupIter++) {
    if ((*groupIter)->getHasStaleMembers()) {
      if ((*groupIter)->isEmpty()) {
        toErase.push_back(groupIter);
        removeGroup(*groupIter);
      }
      else {
        (*groupIter)->generateData();
        // groups that have stale members need to be removed and reinserted
        // this is because the set is maintained in order of distance from
        // the focus center, and a stale-membered group could have had this 
        // distance changed
        toReinsert.push_back(groupIter);
      }
    }
  }
  
  for (list<set<SoarGameGroup*>::iterator>::iterator it= toErase.begin();
      it != toErase.end();
      it++) {
    groups.erase(*it);
  }
  
  SoarGameGroup* grp;
  for (list<set<SoarGameGroup*>::iterator>::iterator it= toReinsert.begin();
      it != toReinsert.end();
      it++) {
    grp = **it;
    groups.erase(*it);
    groups.insert(grp);
    // groups have no knowledge of the focus point, only their distance to it.
    grp->calcDistToFocus(focusX, focusY);
  }

  return;
}

void GroupManager::prepareForReGroup() {
  // iterate through all the groups, if they are stale,
  // recalculate the center member
 
  // prune empty groups

  // add the group type of stale groups to
  // the staleGroupCategories set, so reGroup will run on them
 
  set<SoarGameGroup*>::iterator groupIter;

  list<set<SoarGameGroup*>::iterator> toErase;
  
  for (groupIter = groups.begin(); groupIter != groups.end(); groupIter++) {
    if ((*groupIter)->getHasStaleMembers()) {
      if ((*groupIter)->isEmpty()) {
        toErase.push_back(groupIter);
        // can't delete from sets like this- the iterator gets screwed up
        // keep a list, delete everything at once
        //groups.erase(groupIter);
        removeGroup(*groupIter);
      }
      else {
        (*groupIter)->updateCenterMember();
        staleGroupCategories.insert((*groupIter)->getCategory());
      }
    }
  }
  for (list<set<SoarGameGroup*>::iterator>::iterator it= toErase.begin();
      it != toErase.end();
      it++) {
    groups.erase(*it);
  }
  //cout << "end ref" << endl;
  return;
}

void GroupManager::addGroup(SoarGameObject* object) {
  int size1 = groups.size();
  groups.insert(new SoarGameGroup(object, false, sorts));

  // make sure the insertion takes, the compare function could
  // make the elements seem identical, which would not let them both
  // in the set.
  assert(groups.size() == (unsigned int)(size1 + 1));
  
  //sorts->getFeatureMapManager()->addGroup(object->getGroup());
  // refresh will handle this fine, no need to have an addGroup
  return;
}

void GroupManager::removeGroup(SoarGameGroup* group) {
  sorts->SoarIO->removeGroup(group);
  sorts->featureMapManager->removeGroup(group);
  delete group;
}

void GroupManager::updateFeatureMaps(bool refreshAll) {
  // update the feature maps:
  // if refreshAll, all groups in the world will be updated
  // (needed after a view window change, since that purges the maps)
  // otherwise, only refresh groups that changed
  
  // since this is always the last thing called after a regroup, clear the 
  // staleProperties flags if set
  
  set<SoarGameGroup*>::iterator groupIter;
  for (groupIter = groups.begin(); groupIter != groups.end(); groupIter++) {
    if (refreshAll) {
      sorts->featureMapManager->refreshGroup(*groupIter);
    }
    else if ((*groupIter)->getHasStaleProperties()) {
      sorts->featureMapManager->refreshGroup(*groupIter);
      (*groupIter)->setHasStaleProperties(false);
    }
  }

  sorts->featureMapManager->updateSoar();
  return;
  
}

void GroupManager::adjustAttention() {
  // iterate through all staleProperties groups, if in attn. range,
  // send params to Soar
  
  set<SoarGameGroup*>::iterator groupIter;
  int i=0;
  for (groupIter = groups.begin(); groupIter != groups.end(); groupIter++) {
    if (i < numObjects) {
      if (not (*groupIter)->getInSoar()) {
        sorts->SoarIO->addGroup(*groupIter);
        sorts->SoarIO->refreshGroup(*groupIter);
        (*groupIter)->setInSoar(true);
       // cout << "AAA adding group " << (int)*groupIter << ", dist " <<
       //   (*groupIter)->getDistToFocus() << endl;
        // manually refresh in the feature maps so it is inhibited
        sorts->featureMapManager->refreshGroup(*groupIter);
      }
      if ((*groupIter)->getHasStaleProperties()) {
        sorts->SoarIO->refreshGroup(*groupIter);
        //cout << "AAA refreshing group " << (int)*groupIter << ", dist " <<
        //  (*groupIter)->getDistToFocus() << endl;
      }
    }
    else { 
      //cout << "AAA not adding group " << (int)*groupIter << ", dist " <<
      //  (*groupIter)->getDistToFocus() << endl;
      if ((*groupIter)->getInSoar() == true) {
        (*groupIter)->setInSoar(false);
        sorts->SoarIO->removeGroup(*groupIter);
        // manually refresh in the feature maps so it added back
        sorts->featureMapManager->refreshGroup(*groupIter);
      }
    }
    i++;
  }
    
  return;
}


GroupManager::~GroupManager() {
  set<SoarGameGroup*>::iterator groupIter;
  for (groupIter = groups.begin(); groupIter != groups.end(); groupIter++) {
    delete (*groupIter);
  }
}


SoarGameGroup* GroupManager::getGroupNear(string type, int owner, int x, int y) {
  // this should eventually use the mapmanager,
  // for now just search through all known groups

  // return NULL if no groups of that type are known
  
  //cout << "search for " << type << " owned by " << owner << endl;
  pair<string, int> targetCategory;
  targetCategory.first = type;
  targetCategory.second = owner;
  
  int currentX, currentY;
  double currentDistance;
  double closestDistance = 99999999;
  SoarGameGroup* closestGroup = (SoarGameGroup*) NULL;
  
  set<SoarGameGroup*>::iterator groupIter;

  for (groupIter = groups.begin(); groupIter != groups.end(); groupIter++) {
    if ((*groupIter)->getCategory() == targetCategory) {
      (*groupIter)->getCenterLoc(currentX, currentY);
      currentDistance = squaredDistance(x, y, currentX, currentY);
      if (currentDistance < closestDistance) {
        closestDistance = currentDistance;
        closestGroup = *groupIter;
      }
    }
  }
  
  return closestGroup;
}

void GroupManager::setAllCategoriesStale() {
  // add all categories to the stale list
  // (used to force all groups to refresh)
  staleGroupCategories.clear();
  
  set<SoarGameGroup*>::iterator groupIter;
  // jump iterator between lists

  for (groupIter = groups.begin(); groupIter != groups.end(); groupIter++) {
    staleGroupCategories.insert((*groupIter)->getCategory());
  }
}

void GroupManager::remakeGroupSet() {
  // if the focus point changes, all groups need to be reinserted in the
  // group set, since it is maintained in order of distance from the center

  set<SoarGameGroup*>::iterator groupIter;
 
  set<SoarGameGroup*, ltGroupPtr> newSet;
  
  for (groupIter = groups.begin(); groupIter != groups.end(); groupIter++) {
    (*groupIter)->calcDistToFocus(focusX, focusY);
    newSet.insert(*groupIter);
  }

  groups = newSet;
}
