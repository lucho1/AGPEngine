#ifndef _MESHIMPORTER_H_
#define _MESHIMPORTER_H_

#include "Core/Globals.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Mesh;

class MeshImporter
{
	friend class Resources;
private:

	static uint LoadModel(const std::string& filepath);
	static void ProcessAssimpNode(const aiScene* ai_scene, aiNode* ai_node, const Ref<Mesh>* mesh);

	static void ProcessAssimpMesh(const aiScene* ai_scene, aiMesh* ai_mesh, const Ref<Mesh>* mesh);
	static void ProcessAssimpMaterial(aiMaterial* ai_material, const std::string& directory);

	static std::string LoadMaterialTexture(aiMaterial* ai_material, aiTextureType texture_type, const std::string& directory);
};

#endif //_MESHIMPORTER_H_