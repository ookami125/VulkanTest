#pragma once 
#include <cstdint>
#include <vector>

struct Mesh_Vertex
{
	float loc[3];
};

// indices, one polygon is made out of 3 index values that form a triangle, pointing to vertices
struct Mesh_Polygon
{
	uint32_t vertex_ids[3];
};

// A mesh object is a static collection of vertices and indices
// used by other objects to create a 3D model
// for static models this data can be accessed directly
class Mesh
{
public:
	Mesh();
	~Mesh();

	//void		Load(std::string filepath);

	std::vector<Mesh_Vertex> _vertices;
	std::vector<Mesh_Polygon> _indices;

	uint64_t GetVerticesListByteSize();
	uint64_t GetIndicesListByteSize();

private:
};
