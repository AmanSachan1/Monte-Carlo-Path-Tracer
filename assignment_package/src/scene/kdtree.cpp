#include "kdtree.h"

struct KdNode
{
    KdNode(): bounds( Bounds3f() ) {}
    KdNode( Bounds3f bounds): bounds(bounds) {}

    Bounds3f bounds; // for the bounding box
    KdNode* parent; //parent node of current node

    std::vector<Photon*> nodePhotons_Indirect;//all the photons in that node that were stored on the indirect map,
    std::vector<Photon*> nodePhotons_Caustic; //all the photons in that node that were store on the caustic map,
                                              //if empty then the node has more sub-nodes
    std::vector<KdNode*> children;
};

struct BucketSplit
{
    int difference;
    Bounds3f greaterThanSide;
    Bounds3f lessThanSide;
};


KdTree::KdTree(const Scene &scene, std::vector<std::vector<Photon *> *> &indirectMap, std::vector<std::vector<Photon *> *> &causticMap )
{
    QTime timer = QTime();
    timer.start();

    maxPhotonsInNode = 50;

    KdNode* node = new KdNode();
    node->parent = nullptr;
    node->bounds = Bounds3f(); // not equating to the scene_bounds obtained from the BVH
    //because the entire scene may not be lit, in which case youre biasing the tree in
    //terms of space division

    for(int i=0; i<indirectMap.size(); i++)
    {
        for(int j=0; j<indirectMap[i]->size(); j++)
        {
            node->nodePhotons_Indirect.push_back((*indirectMap[i])[j]);
            node->bounds = Union( node->bounds, (*indirectMap[i])[j]->position );
        }
    }

    for(int i=0; i<causticMap.size(); i++)
    {
        for(int j=0; j<causticMap[i]->size(); j++)
        {
            node->nodePhotons_Caustic.push_back((*causticMap[i])[j]);
            node->bounds = Union( node->bounds, (*causticMap[i])[j]->position );
        }
    }

    int depth = 0;
    constructHelper( node, depth );
    root = node;

    int timeElapsed = timer.elapsed();
    std::string s = std::to_string(timeElapsed);
    std::cout<< "kd Tree construction Time: " + s + " milliseconds" <<std::endl;
}

void KdTree::constructHelper( KdNode* parent_node, int& depth )
{
    depth++;

    //find the longest axis and use 12 dividing lines(13 buckets) to compare the photon count around
    int nSegments = 12;

    Bounds3f tmpCB(parent_node->bounds.min, parent_node->bounds.max);
    int longest_axis = tmpCB.MaximumExtent();

    float line = tmpCB.max[longest_axis] - tmpCB.min[longest_axis];
    float division_line = 0.0f;

    //50% binary tree
    int cost_array_length = nSegments-1;
    BucketSplit bucketSplits[cost_array_length];

    SegregatePhotons(nSegments, division_line, line, longest_axis, tmpCB, parent_node, bucketSplits);

    float minDifference = std::numeric_limits<float>::infinity();
    int optimalSplitLine = -1;

    for( int i=0; i<nSegments; i++ )
    {
        if( ((float)bucketSplits[i].difference) < minDifference )
        {
            minDifference = (float)bucketSplits[i].difference;
            optimalSplitLine = i;
        }
    }

    if(optimalSplitLine == -1) //Will this case even occur?
                               //Photons are all just points in space,
                               //not arbitrary shapes with area
    {
        int count =0;
        while( count<2 && optimalSplitLine==-1 )
        {
            count++;
            longest_axis++;
            longest_axis = longest_axis%3;
            float line = tmpCB.max[longest_axis] - tmpCB.min[longest_axis];;

            SegregatePhotons(nSegments, division_line, line, longest_axis, tmpCB, parent_node, bucketSplits);

            for( int i=0; i<nSegments; i++ )
            {
                if( bucketSplits[i].difference < minDifference )
                {
                    minDifference = bucketSplits[i].difference;
                    optimalSplitLine = i;
                }
            }
        }
    }

    if(optimalSplitLine == -1)
    {
        return;
    }

    if( (parent_node->nodePhotons_Indirect.size() + parent_node->nodePhotons_Caustic.size())>maxPhotonsInNode )
    {
        KdNode* node1 = new KdNode();
        KdNode* node2 = new KdNode();

        node1->parent = parent_node;
        node2->parent = parent_node;

        node1->bounds = bucketSplits[optimalSplitLine].greaterThanSide;
        node2->bounds = bucketSplits[optimalSplitLine].lessThanSide;

        division_line = tmpCB.min[longest_axis] + line*(optimalSplitLine/12.0f);

        for( int i=0; i<parent_node->nodePhotons_Indirect.size(); i++ )
        {
            Photon* p = parent_node->nodePhotons_Indirect[i];
            if( p->position[longest_axis] >= division_line )
            {
                node1->nodePhotons_Indirect.push_back(p);
            }
            else
            {
                node2->nodePhotons_Indirect.push_back(p);
            }
        }

        for( int i=0; i<parent_node->nodePhotons_Caustic.size(); i++ )
        {
            Photon* p = parent_node->nodePhotons_Caustic[i];
            if( p->position[longest_axis] >= division_line )
            {
                node1->nodePhotons_Caustic.push_back(p);
            }
            else
            {
                node2->nodePhotons_Caustic.push_back(p);
            }
        }

        parent_node->nodePhotons_Indirect.clear();
        parent_node->nodePhotons_Caustic.clear();

        if( (node1->nodePhotons_Indirect.size() + node1->nodePhotons_Caustic.size())>maxPhotonsInNode )
        {
            parent_node->children.push_back(node1);
            constructHelper( node1, depth );
        }
        else //if( (node1->nodePhotons_Indirect.size() + node1->nodePhotons_Caustic.size())==maxPhotonsInNode )
        {
            parent_node->nodePhotons_Indirect.insert(std::end(parent_node->nodePhotons_Indirect),
                                                          std::begin(node1->nodePhotons_Indirect),
                                                            std::end(node1->nodePhotons_Indirect));
            parent_node->nodePhotons_Indirect.insert(std::end(parent_node->nodePhotons_Caustic),
                                                          std::begin(node1->nodePhotons_Caustic),
                                                            std::end(node1->nodePhotons_Caustic));
        }

        if( (node2->nodePhotons_Indirect.size() + node2->nodePhotons_Caustic.size())>maxPhotonsInNode )
        {
            parent_node->children.push_back(node2);
            constructHelper( node2, depth );
        }
        else
        {
            parent_node->nodePhotons_Indirect.insert(std::end(parent_node->nodePhotons_Indirect),
                                                          std::begin(node2->nodePhotons_Indirect),
                                                            std::end(node2->nodePhotons_Indirect));
            parent_node->nodePhotons_Indirect.insert(std::end(parent_node->nodePhotons_Caustic),
                                                          std::begin(node2->nodePhotons_Caustic),
                                                            std::end(node2->nodePhotons_Caustic));
        }

        return;
    }
    else
    {
        //terminate and create a leafNode with the current parnetNode
        parent_node->children.clear();
        return;
    }
}

void KdTree::SegregatePhotons(int& nSegments, float& division_line, float& line,
                              int& longest_axis, Bounds3f& tmpCB, KdNode* parent_node,
                              BucketSplit bucketSplits[])
{
    for( int j=1; j<nSegments; j++ )
    {
        Bounds3f b0 = Bounds3f(), b1 = Bounds3f();
        int count0 = 0, count1 = 0;
        division_line = tmpCB.min[longest_axis] + line*(j/12.0f);

        for( int i=0; i<parent_node->nodePhotons_Indirect.size(); i++ )
        {
            Photon* p = parent_node->nodePhotons_Indirect[i];
            LeftOrRightofDivisionLine( p, longest_axis, division_line, count0, count1, b0, b1);
        }

        for( int i=0; i<parent_node->nodePhotons_Caustic.size(); i++ )
        {
            Photon* p = parent_node->nodePhotons_Caustic[i];
            LeftOrRightofDivisionLine( p, longest_axis, division_line, count0, count1, b0, b1);
        }

        bucketSplits[j-1].difference = std::abs(count1 - count0);
        bucketSplits[j-1].greaterThanSide = b0;
        bucketSplits[j-1].lessThanSide = b1;
    }
}

void KdTree::LeftOrRightofDivisionLine( Photon* p, int& longest_axis, float& division_line,
                                        int& count0, int& count1, Bounds3f& b0, Bounds3f& b1)
{
    if( p->position[longest_axis] >= division_line )
    {
        count0++;
        b0 = Union( b0, p->position );
    }
    else
    {
        count1++;
        b1 = Union( b1, p->position );
    }
}

KdNode* KdTree::FindNode( Vector3f& pos )
{
    KdNode* encompassingNode;
    KdNode* node = root;
    while(node->children.size()>0)
    {
        for(int i=0; i<node->children.size(); i++)
        {
            if(node->children[i]->bounds.Contains(pos))
            {
                node = node->children[i];
                //since photons are infintesmally small they can only be in one node
                break; //therefore we can break out of the for loop
            }
        }
    }

    //node is now the lowest leaf of the kdTree that contains the photon
    return node;
}

void KdTree::FindAllNeighborNodes( KdNode* originalNode, std::vector<KdNode*>&nodeList, Vector3f& pos, float& searchRadius )
{
    //now find all neighboring nodes that would be encompassed in the search radius
    Vector3f encompassingSphereMin = Vector3f(pos.x - searchRadius, pos.y - searchRadius, pos.z - searchRadius);
    Vector3f encompassingSphereMax = Vector3f(pos.x + searchRadius, pos.y + searchRadius, pos.z + searchRadius);
    KdNode* encompassingNode = originalNode;

    while(!(encompassingNode->bounds.Contains(encompassingSphereMin) && encompassingNode->bounds.Contains(encompassingSphereMax)))
    {
        if(encompassingNode->parent != nullptr)
        {
            encompassingNode = encompassingNode->parent;
        }
    }

    //now we have the node that for sure encompasses the entire search radius
    //successively check all of its children to see if they have any photons in them, if they do add them to the list
    checkChildren(nodeList, encompassingNode, encompassingSphereMin, encompassingSphereMax);
}

void KdTree::checkChildren( std::vector<KdNode*>&nodeList, KdNode* testnode, Vector3f& encompassingSphereMin, Vector3f& encompassingSphereMax )
{
    if(testnode->nodePhotons_Caustic.size()>0 || testnode->nodePhotons_Indirect.size()>0)
    {
        nodeList.push_back(testnode);
    }
    for(int i=0; i<testnode->children.size(); i++)
    {
        if(testnode->children[i]->bounds.Contains(encompassingSphereMin) && testnode->children[i]->bounds.Contains(encompassingSphereMax))
        {
            checkChildren( nodeList, testnode->children[i], encompassingSphereMin, encompassingSphereMax);
        }
    }
}

Color3f KdTree::BlendPhotons(std::vector<KdNode*> &nodeList, Vector3f &position)
{
    Color3f averagedColor = Color3f(0.0f);
    int count = 0;
    for(int i=0; i<nodeList.size(); i++)
    {
        for(int j=0; j<nodeList[i]->nodePhotons_Indirect.size(); j++)
        {
            Photon* photon = nodeList[i]->nodePhotons_Indirect[j];
            if(WithinSearchRadius( photon->position, position ))
            {
                averagedColor += photon->color;
                count ++;
            }
        }

        for(int j=0; j<nodeList[i]->nodePhotons_Caustic.size(); j++)
        {
            Photon* photon = nodeList[i]->nodePhotons_Caustic[j];
            if(WithinSearchRadius( photon->position, position ))
            {
                averagedColor += photon->color;
                count ++;
            }
        }
    }

    averagedColor /= count;
}

bool KdTree::WithinSearchRadius( Vector3f& photonPos, Vector3f& testpoint)
{
    if((photonPos.x < testpoint.x + searchRadius) && (photonPos.y < testpoint.y + searchRadius) && (photonPos.z < testpoint.z + searchRadius) &&
       (photonPos.x > testpoint.x - searchRadius) && (photonPos.y > testpoint.y - searchRadius) && (photonPos.z > testpoint.z - searchRadius))
    {
        return true;
    }
    return false;
}
