#include "MeshImporter.h"

#include "Resources.h"
#include "Core/Utils/FileStringUtils.h"
#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/Mesh.h"


// ------------------------------------------------------------------------------
uint MeshImporter::LoadModel(const std::string& filepath)
{
    // -- Load Scene --
    const aiScene* scene = aiImportFile(filepath.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace //| aiProcess_FlipUVs // TODO: Maybe this gives errors on texture load!!!!
        | aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices | aiProcess_ImproveCacheLocality | aiProcess_OptimizeMeshes | aiProcess_SortByPType);

    if (!scene)
    {
        ENGINE_LOG("Error Opening Assimp Scene from '%s'\nAssimp Error: %s", filepath.c_str(), aiGetErrorString());
        return 0;
    }

    //TODO: Test, delete this
    std::string dir = FileUtils::GetDirectory(filepath);
    for (uint i = 0; i < scene->mNumMaterials; ++i)
        ProcessAssimpMaterial(scene->mMaterials[i], dir); // TODO: Ojo aqui con este get directory part Ojo q tambien carga un "DefaultMaterial"

    //ProcessAssimpNode(scene, scene->mRootNode, myMesh);
    //aiReleaseImport(scene);
}


void MeshImporter::ProcessAssimpNode(const aiScene* ai_scene, aiNode* ai_node, const Ref<Mesh>* mesh)
{
    // -- Process Node Meshes --
    //for (uint i = 0; i < ai_node->mNumMeshes; i++)
    //    ProcessAssimpMesh(ai_scene, ai_scene->mMeshes[ai_node->mMeshes[i]], myMesh);
    //
    //// -- Process Node Children Meshes --
    //for (uint i = 0; i < ai_node->mNumChildren; i++)
    //    ProcessAssimpNode(ai_scene, ai_node->mChildren[i], myMesh);
}



// ------------------------------------------------------------------------------
void MeshImporter::ProcessAssimpMesh(const aiScene* ai_scene, aiMesh* ai_mesh, const Ref<Mesh>* mesh)
{
}


void MeshImporter::ProcessAssimpMaterial(aiMaterial* ai_material, const std::string& directory)
{
    // -- Load Material Variables --
    aiString name = aiString("unnamed");
    ai_real shininess = 0.0f, opacity = 1.0f;
    ai_int two_sided = 0;
    aiColor3D diffuse, emissive, specular;

    AI_DEFAULT_MATERIAL_NAME;

    //ai_material->Get(AI_MATKEY_COLOR_SPECULAR, specular); <- Useless
    ai_material->Get(AI_MATKEY_NAME, name);
    ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);         // Kd
    ai_material->Get(AI_MATKEY_COLOR_SPECULAR, specular);       // Ks
    ai_material->Get(AI_MATKEY_OPACITY, opacity);               // d
    ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive);       // Ke
    ai_material->Get(AI_MATKEY_SHININESS, shininess);           // Ns
    ai_material->Get(AI_MATKEY_TWOSIDED, two_sided);

    // Tests -- (?) = Nose per a que serveix, ? = Nose quin valor es en el .mtl
    //aiColor3D amb, transp;
    //ai_material->Get(AI_MATKEY_COLOR_AMBIENT, amb);           // Ka (?)
    //ai_material->Get(AI_MATKEY_COLOR_TRANSPARENT, transp);    // ? (?)
    //
    //ai_int nn;
    //ai_real nn2;
    //ai_material->Get(AI_MATKEY_REFRACTI, nn);                 // Ni -> Maybe the "Metallic" value? (?)
    //ai_material->Get(AI_MATKEY_BUMPSCALING, nn2);             // ? (?)
    //ai_material->Get(AI_MATKEY_REFLECTIVITY, nn2);            // ? (?) There's also a reflectivity color (wtf?)
    //ai_material->Get(AI_MATKEY_SHININESS_STRENGTH, nn2);      // ? (?)


    // -- Create Material & Set Variables --
    const Ref<Material>& mat = *Resources::CreateMaterial(name.C_Str());
    mat->AlbedoColor = glm::vec4(diffuse.r, diffuse.g, diffuse.b, opacity);
    mat->EmissiveColor= glm::vec4(emissive.r, emissive.g, emissive.b, 1.0f);
    mat->Smoothness = shininess / 256.0f;

    mat->IsTwoSided = (!two_sided && opacity < 1.0f) ? true : two_sided;
    mat->IsEmissive = emissive.IsBlack() ? false : true;
    mat->IsTransparent = opacity < 1.0f;

    // -- Set Material Textures --
    if (ai_material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        mat->Albedo = Resources::CreateTexture(LoadMaterialTexture(ai_material, aiTextureType_DIFFUSE, directory));

    if (ai_material->GetTextureCount(aiTextureType_EMISSIVE) > 0)
        mat->Emissive = Resources::CreateTexture(LoadMaterialTexture(ai_material, aiTextureType_EMISSIVE, directory));

    if (ai_material->GetTextureCount(aiTextureType_SPECULAR) > 0)
        mat->Specular = Resources::CreateTexture(LoadMaterialTexture(ai_material, aiTextureType_SPECULAR, directory));

    if (ai_material->GetTextureCount(aiTextureType_NORMALS) > 0)
        mat->Normal = Resources::CreateTexture(LoadMaterialTexture(ai_material, aiTextureType_NORMALS, directory));

    if (ai_material->GetTextureCount(aiTextureType_HEIGHT) > 0)
        mat->Bump = Resources::CreateTexture(LoadMaterialTexture(ai_material, aiTextureType_HEIGHT, directory));
}



// ------------------------------------------------------------------------------
std::string MeshImporter::LoadMaterialTexture(aiMaterial* ai_material, aiTextureType texture_type, const std::string& directory)
{
    aiString ai_filepath;
    ai_material->GetTexture(texture_type, 0, &ai_filepath);
    return FileUtils::MakePath(directory, ai_filepath.C_Str());
}
