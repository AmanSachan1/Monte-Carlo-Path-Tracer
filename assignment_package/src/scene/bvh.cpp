#include "bvh.h"
#include <queue>

// Feel free to ignore these structs entirely!
// They are here if you want to implement any of PBRT's
// methods for BVH construction.

struct BVHPrimitiveInfo {
    BVHPrimitiveInfo() {}
    BVHPrimitiveInfo(size_t primitiveNumber, const Bounds3f &bounds)
        : primitiveNumber(primitiveNumber),
          bounds(bounds),
          centroid(.5f * bounds.min + .5f * bounds.max) {}
    int primitiveNumber;
    Bounds3f bounds;
    Point3f centroid;
};

struct BVHBuildNode {
    // BVHBuildNode Public Methods
    void InitLeaf(int first, int n, const Bounds3f &b) {
        firstPrimOffset = first;
        nPrimitives = n;
        bounds = b;
        children[0] = children[1] = nullptr;
    }
    void InitInterior(int axis, BVHBuildNode *c0, BVHBuildNode *c1) {
        children[0] = c0;
        children[1] = c1;
        bounds = Union(c0->bounds, c1->bounds);
        splitAxis = axis;
        nPrimitives = 0;
    }
    Bounds3f bounds;
    BVHBuildNode *children[2];
    int splitAxis, firstPrimOffset, nPrimitives;
};

struct MortonPrimitive {
    int primitiveIndex;
    unsigned int mortonCode;
};

struct LBVHTreelet {
    int startIndex, nPrimitives;
    BVHBuildNode *buildNodes;
};

struct LinearBVHNode {
    Bounds3f bounds;
    union {
        int primitivesOffset;   // leaf
        int secondChildOffset;  // interior
    };
    unsigned short nPrimitives;  // 0 -> interior node, 16 bytes
    unsigned char axis;          // interior node: xyz, 8 bytes
    unsigned char pad[1];        // ensure 32 byte total size
};

struct BVHNode {
    BVHNode(): bounds( Bounds3f() ) {}
    BVHNode( Bounds3f bounds): bounds(bounds) {}

    Bounds3f bounds; // for the bounding box
    QList<std::shared_ptr<Primitive>> nodePrimitives; //all the primitives in that node,
                                //if empty then the node has more sub-nodes
    std::vector<BVHNode*> children;
};

BVHAccel::~BVHAccel()
{
    delete [] nodes;
}

// Constructs an array of BVHPrimitiveInfos, recursively builds a node-based BVH
// from the information, then optimizes the memory of the BVH
BVHAccel::BVHAccel(const std::vector<std::shared_ptr<Primitive> > &p, int maxPrimsInNode)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), primitives(p)
{
    QTime timer = QTime();
    timer.start();

    std::vector< glm::vec3 > centroid;
    std::vector< Bounds3f > primitiveBounds;

    Bounds3f centroid_bounds = Bounds3f();
    Bounds3f scene_bounds = Bounds3f();

    //find the min and max of each primitive in the scene and also their centroids and their surface area
    int l = 0;    

    BVHNode* node = new BVHNode();

    for( int i=0; i<primitives.size(); i++ )
    {
        std::shared_ptr<Primitive> prim = primitives.at(i);
        Bounds3f b = prim->shape->WorldBound();
        primitiveBounds.push_back(b);

        l = primitiveBounds.size() - 1;

        centroid.push_back( glm::vec3((primitiveBounds[l].min.x + primitiveBounds[l].max.x)/2.0f,
                                      (primitiveBounds[l].min.y + primitiveBounds[l].max.y)/2.0f,
                                      (primitiveBounds[l].min.z + primitiveBounds[l].max.z)/2.0f) );

        centroid_bounds = Union( centroid_bounds, centroid[l] );
        scene_bounds = Union( scene_bounds, primitiveBounds[l] );

        node->nodePrimitives.push_back(primitives[i]);
    }

    node->bounds = scene_bounds;

    int depth = 0;
    int count =0;
    //we have the root node of the tree so now create all of its children in a recursive manner
    constructHelper( centroid_bounds.min, centroid_bounds.max, node, depth, count);

    root = node;

    int timeElapsed = timer.elapsed();
    std::string s = std::to_string(timeElapsed);
    std::cout<< "BVH construction Time: " + s + " milliseconds" <<std::endl;
}

void BVHAccel::constructHelper( glm::vec3 bv_min,
                                glm::vec3 bv_max,
                                BVHNode* parent_node,
                                int& depth,
                                int &count)
{
    depth++;
    BVHNode* node1 = new BVHNode();
    BVHNode* node2 = new BVHNode();

    //find the longest axis and use 7 dividing lines(8 buckets) to compare the surface areas around
    Bounds3f tmpCB(bv_min, bv_max);
    int longest_axis = tmpCB.MaximumExtent();

    float line = bv_max[longest_axis] - bv_min[longest_axis];
    float division_line = 0.0f;

    int nSegments = 12;

    //50% binary tree
    int cost_array_length = nSegments-1;
    float cost[cost_array_length];
    for( int j=1; j<nSegments; j++ )
    {
        Bounds3f b0 = Bounds3f(), b1 = Bounds3f();
        int count0 = 0, count1 = 0;
        division_line = bv_min[longest_axis] + line*(j/12.0f);

        for( int i=0; i<parent_node->nodePrimitives.size(); i++ )
        {
            Bounds3f currPrimBounds = parent_node->nodePrimitives[i]->shape->WorldBound();

            glm::vec3 centroid_current = glm::vec3( ( currPrimBounds.max.x + currPrimBounds.min.x )/2.0f,
                                                    ( currPrimBounds.max.y + currPrimBounds.min.y )/2.0f,
                                                    ( currPrimBounds.max.z + currPrimBounds.min.z )/2.0f );

            if( centroid_current[longest_axis] >= division_line )
            {
                count0++;
                b0 = Union( b0, currPrimBounds );
            }

            if( centroid_current[longest_axis] < division_line )
            {
                count1++;
                b1 = Union( b1, currPrimBounds );
            }
        }

        cost[j-1] = (count0 * b0.SurfaceArea() +
                     count1 * b1.SurfaceArea()) / parent_node->bounds.SurfaceArea();
    }

    float minCost = std::numeric_limits<Float>::infinity();
    int minCostSplitLine = -1;
    for( int i=1; i<nSegments; i++ )
    {
        if( cost[i-1] < minCost )
        {
            minCost = cost[i-1];
            minCostSplitLine = i;
        }
    }

    if(minCostSplitLine == -1)
    {
        int count =0;
        while( count<2 && minCostSplitLine==-1 )
        {
            longest_axis++;
            longest_axis = longest_axis%3;
            float line = bv_max[longest_axis] - bv_min[longest_axis];
            count++;

            for( int j=1; j<nSegments; j++ )
            {
                Bounds3f b0 = Bounds3f(), b1 = Bounds3f();
                int count0 = 0, count1 = 0;
                division_line = bv_min[longest_axis] + line*(j/12.0f);

                for( int i=0; i<parent_node->nodePrimitives.size(); i++ )
                {
                    Bounds3f currPrimBounds = parent_node->nodePrimitives[i]->shape->WorldBound();

                    glm::vec3 centroid_current = glm::vec3( ( currPrimBounds.max.x + currPrimBounds.min.x )/2.0f,
                                                            ( currPrimBounds.max.y + currPrimBounds.min.y )/2.0f,
                                                            ( currPrimBounds.max.z + currPrimBounds.min.z )/2.0f );

                    if( centroid_current[longest_axis] >= division_line )
                    {
                        count0++;
                        b0 = Union( b0, centroid_current );
                    }

                    if( centroid_current[longest_axis] < division_line )
                    {
                        count1++;
                        b1 = Union( b1, centroid_current );
                    }
                }

                cost[j-1] = (count0 * b0.SurfaceArea() +
                             count1 * b1.SurfaceArea()) / parent_node->bounds.SurfaceArea();
            }

            for( int i=1; i<nSegments; i++ )
            {
                if( cost[i-1] < minCost )
                {
                    minCost = cost[i-1];
                    minCostSplitLine = i;
                }
            }
        }
    }

    if(minCostSplitLine == -1)
    {
        return;
    }

    float leafCost = parent_node->nodePrimitives.size();

    if( parent_node->nodePrimitives.size()>maxPrimsInNode || minCost<leafCost && minCostSplitLine != -1)
    {
        division_line = bv_min[longest_axis] + line*(minCostSplitLine/12.0f);
        //for the selected division line create the bounds for each node and
        //pass in primitives into their respective primitive lists
        Bounds3f b0 = Bounds3f(), b1 = Bounds3f();
        Bounds3f cb0 = Bounds3f(), cb1 = Bounds3f();

        for( int i=0; i<parent_node->nodePrimitives.size(); i++ )
        {
            Bounds3f currPrimBounds = parent_node->nodePrimitives[i]->shape->WorldBound();

            glm::vec3 centroid_current = glm::vec3( ( currPrimBounds.max.x + currPrimBounds.min.x )/2.0f,
                                                    ( currPrimBounds.max.y + currPrimBounds.min.y )/2.0f,
                                                    ( currPrimBounds.max.z + currPrimBounds.min.z )/2.0f );

            if( centroid_current[longest_axis] >= division_line )
            {
                b0 = Union( b0, currPrimBounds );
                cb0 = Union( cb0, centroid_current );
                node1->nodePrimitives.push_back( parent_node->nodePrimitives[i] );
            }

            if( centroid_current[longest_axis] < division_line )
            {
                b1 = Union( b1, currPrimBounds );
                cb1 = Union( cb1, centroid_current );
                node2->nodePrimitives.push_back( parent_node->nodePrimitives[i] );
            }
        }

        node1->bounds = b0;
        node2->bounds = b1;

        bool flag = false;

        parent_node->nodePrimitives.clear();

        if( node1->nodePrimitives.size()>maxPrimsInNode && node1->bounds.SurfaceArea()==node1->bounds.SurfaceArea() )
        {
            parent_node->children.push_back(node1);
            constructHelper( cb0.min, cb0.max, node1, depth, count );
            flag = true;
        }
        else if( node1->nodePrimitives.size()==maxPrimsInNode )
        {
            parent_node->nodePrimitives.append(node1->nodePrimitives);
        }

        if( node2->nodePrimitives.size()>maxPrimsInNode && node2->bounds.SurfaceArea()==node2->bounds.SurfaceArea() )
        {
            parent_node->children.push_back(node2);
            constructHelper( cb1.min, cb1.max, node2, depth, count );
            flag = true;
        }
        else if( node2->nodePrimitives.size()==maxPrimsInNode )
        {
            parent_node->nodePrimitives.append(node2->nodePrimitives);
        }

        if( flag )
        {
            count++;
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

bool BVHAccel::Intersect(const Ray &ray, Intersection *isect) const
{
    std::queue<BVHNode*> q;
    q.push(root);
    std::vector<Intersection> intersectionList;

    while(!q.empty())
    {
        BVHNode* node = q.front();
        q.pop();
        bool flag_hit = false;
        float* t;
        flag_hit = node->bounds.Intersect(ray, t);
        if(flag_hit)
        {
            for(int i=0; i<node->children.size(); i++)
            {
                q.push( node->children[i] );
            }

            for(int i=0; i<node->nodePrimitives.size(); i++)
            {
                bool flag_intersect = false;
                flag_intersect = node->nodePrimitives[i]->Intersect(ray, isect);
                if(flag_intersect)
                {
                    intersectionList.push_back( *isect );
                }
            }
        }
    }

    //have all intersections, now sort them
    float min = std::numeric_limits<Float>::infinity();
    for(int i=0; i<intersectionList.size(); i++)
    {
        Intersection isect_temp = intersectionList[i];
        if(min > isect_temp.t)
        {
            *isect = isect_temp;
            min = isect_temp.t;
        }
    }

    if(min != std::numeric_limits<Float>::infinity())
    {
        return true;
    }
    return false;
}
