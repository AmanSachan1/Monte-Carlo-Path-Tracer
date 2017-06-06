#include "kdtree.h"

struct KdNode
{
    KdNode(): bounds( Bounds3f() ) {}
    KdNode( Bounds3f bounds): bounds(bounds) {}

    Bounds3f bounds; // for the bounding box
    std::vector<Photon*> nodePhotons; //all the photons in that node,
                                //if empty then the node has more sub-nodes
    std::vector<KdNode*> children;
};


KdTree::KdTree(Scene& scene, std::vector<std::vector<Photon*>> indirectMap, std::vector<std::vector<Photon*>> CausticMap )
{
    QTime timer = QTime();
    timer.start();

    KdNode* node = new KdNode();
    node->bounds = scene.bounds;

    for(int i=0; i<indirectMap.size(); i++)
    {
        for(int j=0; j<indirectMap[i].size(); j++)
        {
            node->nodePhotons.push_back(indirectMap[i][j]);
        }
    }
    for(int i=0; i<causticMap.size(); i++)
    {
        for(int j=0; j<causticMap[i].size(); j++)
        {
            node->nodePhotons.push_back(indirectMap[i][j]);
        }
    }

    int depth = 0;
    int count = 0;

    root = node;

    int timeElapsed = timer.elapsed();
    std::string s = std::to_string(timeElapsed);
    std::cout<< "kd Tree construction Time: " + s + " milliseconds" <<std::endl;
}

void constructHelper( glm::vec3 bv_min, glm::vec3 bv_max,
                      KdTree* parent_node, int& depth,
                      int &count )
{

}
