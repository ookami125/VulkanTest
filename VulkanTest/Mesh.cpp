#include "Mesh.h"
Mesh::Mesh()
{
	_vertices.resize(3);
	_indices.resize(1);

	_vertices[0].loc[0] = 0.0f;
	_vertices[0].loc[1] = -1.0f;
	_vertices[0].loc[2] = 0.5f;

	_vertices[1].loc[0] = -1.0f;
	_vertices[1].loc[1] = 1.0f;
	_vertices[1].loc[2] = 0.5f;

	_vertices[2].loc[0] = 1.0f;
	_vertices[2].loc[1] = 1.0f;
	_vertices[2].loc[2] = 0.5f;

	_indices[0].vertex_ids[0] = 0;
	_indices[0].vertex_ids[1] = 1;
	_indices[0].vertex_ids[2] = 2;
}


Mesh::~Mesh()
{
}

uint64_t Mesh::GetVerticesListByteSize()
{
	return _vertices.size() * sizeof(Mesh_Vertex);
}

uint64_t Mesh::GetIndicesListByteSize()
{
	return _indices.size() * sizeof(Mesh_Polygon);
}