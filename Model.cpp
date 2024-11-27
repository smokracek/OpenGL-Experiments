#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>

#include <iostream>

Model::Model(const char* path) {
	this->path = std::string(path);
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, 
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace | 
		aiProcess_JoinIdenticalVertices
	);
	
	if (!scene) {
		std::cout << "MODEL IMPORT ERROR: " << importer.GetErrorString() << std::endl;
		return;
	}

	processNode(scene->mRootNode, scene);
	printSummaryStats();
}

void Model::draw() {
	for (Mesh& mesh : meshes) {
		mesh.draw();
	}
}

void Model::printSummaryStats() {
	unsigned int vertTotal = 0;
	for (Mesh& mesh : meshes) {
		vertTotal += mesh.vertices.size();
	}
	unsigned int meshTotal = meshes.size();

	float mbTotal = (float)(vertTotal * sizeof(Vertex)) / (1024 * 1024);

	std::cout << "Loaded model " << path << " with " << meshTotal << " meshes and " << vertTotal << " vertices (" << mbTotal << "MB)" << std::endl;
}

void Model::processNode(aiNode* node, const aiScene* scene) {
	if (node->mNumMeshes > 0) {
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			unsigned int meshIndex = node->mMeshes[i];
			meshes.push_back(Mesh(scene->mMeshes[meshIndex]));
		}
	}

	if (node->mNumChildren > 0) {
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}
	}
}