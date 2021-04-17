#ifndef _MESHIMPORTER_H_
#define _MESHIMPORTER_H_

#include "Core/Globals.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Mesh;
class Material;
class Model;

class MeshImporter
{
	friend class Resources;
private:

	static Ref<Model> LoadModel(const std::string& filepath);

	static void ProcessAssimpNode(const aiScene* ai_scene, aiNode* ai_node, Mesh* mesh);
	static Ref<Mesh>* ProcessAssimpMesh(const aiScene* ai_scene, aiMesh* ai_mesh);
	static const Ref<Material>* ProcessAssimpMaterial(aiMaterial* ai_material, const std::string& directory);

	static std::string LoadMaterialTexture(aiMaterial* ai_material, aiTextureType texture_type, const std::string& directory);
};

#endif //_MESHIMPORTER_H_