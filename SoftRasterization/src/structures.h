#pragma once

#include <glm.hpp>
#include <vector>

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct Triangle
{
	Vertex* point[3];
};

struct TriangleOrdinary
{
	glm::vec3 point[3];
};

struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

struct Model
{
	std::vector<Mesh*> meshes;
};

struct Texture
{
	unsigned char* texture;
	int width;
	int height;
	int nrComponents;
};