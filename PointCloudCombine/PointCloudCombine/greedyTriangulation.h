/*
 *
 *
 */
#ifndef __greedyTriangulation_h_
#define __greadyTriangulation_h_
#include "arrayList.h"
#include "geometryMesh.h"

arrayListui* greedyTriangulation(geometryMesh* model, geometryMesh* data, arrayListui* pointsToTrangulate, float maxSQDistance);

geometryMesh* combimeMeshes(geometryMesh* model, geometryMesh* data, arrayListui* cutEdgePoints, float maxSQDistance);

#endif //greadyTriangulation_h
