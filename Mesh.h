#pragma once

#include <assimp/mesh.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
};

class Mesh
{
public:
	GLuint VAO;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	Mesh(aiMesh* mesh);
	void draw();
};

