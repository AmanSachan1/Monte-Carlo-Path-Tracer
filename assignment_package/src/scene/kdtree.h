#pragma once
#include "geometry/primitive.h"
#include <QTime>

// Forward declarations of structs used by our kd tree
// They are defined in the .cpp file
struct KdNode;

class KdTree
{
public:
    KdTree();

    //members
    int maxPhotonsInNode;

    KdNode* root;
};
