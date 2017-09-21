#pragma once
#include "geometry/primitive.h"
#include <QTime>
#include "scene/photons.h"

// Forward declarations of structs used by our kd tree
// They are defined in the .cpp file
struct KdNode;
struct BucketSplit;
class Scene;
class Photon;

class KdTree
{
public:
    KdTree(const Scene &scene, std::vector<std::vector<Photon*>*> &indirectMap, std::vector<std::vector<Photon*>*> &causticMap);

    void constructHelper(KdNode* parent_node, int& depth);
    void SegregatePhotons(int& nSegments, float& division_line, float& line,
                          int& longest_axis, Bounds3f& tmpCB, KdNode* parent_node,
                          BucketSplit bucketSplits[]);
    void LeftOrRightofDivisionLine( Photon* p, int& longest_axis, float& division_line,
                                    int& count0, int& count1, Bounds3f& b0, Bounds3f& b1);

    KdNode* FindNode( Vector3f& pos );
    void FindAllNeighborNodes( KdNode* originalNode, std::vector<KdNode*>&nodeList, Vector3f& pos, float& searchRadius );
    void checkChildren( std::vector<KdNode*>&nodeList, KdNode* testnode, Vector3f& encompassingSphereMin, Vector3f& encompassingSphereMax );

    //members
    int maxPhotonsInNode;
    KdNode* root;
};
