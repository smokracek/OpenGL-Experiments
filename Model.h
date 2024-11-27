#pragma once


#include <assimp/scene.h>

#include "Mesh.h"

#include <vector>
#include <string>

class Model
{
public:
	std::string path;
	std::vector<Mesh> meshes;

	Model(const char* path);
	void draw();
	void printSummaryStats();

private:
	void processNode(aiNode* node, const aiScene* scene);
};


