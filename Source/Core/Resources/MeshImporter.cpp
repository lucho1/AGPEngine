#include "MeshImporter.h"

#include "Resources.h"
#include "Core/Utils/FileStringUtils.h"
#include "Renderer/Resources/Buffers.h"
#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/Mesh.h"


// ------------------------------------------------------------------------------
Ref<Model> MeshImporter::LoadModel(const std::string& filepath)
{
    // -- Load Scene --
    const aiScene* scene = aiImportFile(filepath.c_str(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals //| aiProcess_FlipUVs // FlipUVs gives problem with UVs, I think because STB already flips them
        | aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices | aiProcess_ImproveCacheLocality | aiProcess_OptimizeMeshes | aiProcess_SortByPType);

    //const aiScene* scene = aiImportFile(filepath.c_str(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        ENGINE_LOG("Error Opening Assimp Scene from '%s'\nAssimp Error: %s", filepath.c_str(), aiGetErrorString());
        return nullptr;
    }

    if (scene->mNumMeshes == 0)
    {
        ENGINE_LOG("Error Loading Model, there were no meshes to load!");
        return nullptr;
    }

    // -- Load Materials --
    std::vector<uint> materials;
    for (uint i = 0; i < scene->mNumMaterials; ++i)
    {
        const Ref<Material>* mat = ProcessAssimpMaterial(scene->mMaterials[i], FileUtils::GetDirectory(filepath));
        if (mat != nullptr)
            materials.push_back((*mat)->GetID());
    }

    // -- Load Meshes --
    aiMesh* first_mesh = scene->mMeshes[0];
    Ref<Model> model = CreateRef<Model>(new Model(filepath));

    model->m_RootMesh = ProcessAssimpMesh(scene, first_mesh)->get();
    model->m_RootMesh->m_MaterialIndex = first_mesh->mMaterialIndex == 0 ? 0 : materials[first_mesh->mMaterialIndex - 1];
    model->m_RootMesh->m_Name = first_mesh->mName.length > 0 ? first_mesh->mName.C_Str() : "unnamed";

    ProcessAssimpNode(scene, scene->mRootNode, model->m_RootMesh, materials);

    // -- Release Assimp & Return --
    aiReleaseImport(scene);
    return model;
}


void MeshImporter::ProcessAssimpNode(const aiScene* ai_scene, aiNode* ai_node, Mesh* mesh, const std::vector<uint> loaded_materials)
{
    // -- Process Node Meshes --
    std::vector<Ref<Mesh>*> meshes;
    for (uint i = 0; i < ai_node->mNumMeshes; ++i)
    {
        if (ai_node->mMeshes[i] != 0)
        {
            aiMesh* ai_mesh = ai_scene->mMeshes[ai_node->mMeshes[i]];
            meshes.push_back(ProcessAssimpMesh(ai_scene, ai_mesh));

            if (ai_mesh->mName.length > 0)
                (*meshes[i])->SetName(ai_mesh->mName.C_Str());

            if (ai_mesh->mMaterialIndex != 0)
                (*meshes[i])->m_MaterialIndex = loaded_materials[ai_mesh->mMaterialIndex - 1]; // -1 Because we are not loading assimp's default material
        }
    }

    // -- Fill Mesh' Submeshes --
    for (uint i = 0; i < meshes.size(); ++i)
        if (meshes[i] != nullptr)
            mesh->AddSubmesh(meshes[i]);
    
    // -- Process Node Children Meshes --
    for (uint i = 0; i < ai_node->mNumChildren; i++)
        ProcessAssimpNode(ai_scene, ai_node->mChildren[i], mesh, loaded_materials);
}



// ------------------------------------------------------------------------------
Ref<Mesh>* MeshImporter::ProcessAssimpMesh(const aiScene* ai_scene, aiMesh* ai_mesh)
{
    if (ai_mesh->mNumVertices == 0 || ai_mesh->mNumFaces == 0)
        return nullptr;

    // -- Process Vertices --
    std::vector<float> vertices;
    std::vector<uint> indices;
    
    for (uint i = 0; i < ai_mesh->mNumVertices; ++i)
    {
        // Positions & Normals
        glm::vec3 positions = glm::vec3(0.0f), normals = glm::vec3(0.0f);
        glm::vec2 texture_coords = glm::vec2(0.0f);
        
        if (ai_mesh->HasPositions())
            positions = { ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z };

        if (ai_mesh->mTextureCoords[0])
            texture_coords = { ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y };
        else
            int a = 0;

        if (ai_mesh->HasNormals())
            normals = { ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z };

        vertices.push_back(positions.x);
        vertices.push_back(positions.y);
        vertices.push_back(positions.z);
        vertices.push_back(texture_coords.x);
        vertices.push_back(texture_coords.y);
        vertices.push_back(normals.x);
        vertices.push_back(normals.y);
        vertices.push_back(normals.z);

        // Tangents & Bitangents
        glm::vec3 tangents = glm::vec3(0.0f), bitangents = glm::vec3(0.0f); // TODO: Ojo aqui q diu en jesús q estan flipped
        if (ai_mesh->HasTangentsAndBitangents())
        {
            tangents = { ai_mesh->mTangents[i].x, ai_mesh->mTangents[i].y, ai_mesh->mTangents[i].z };
            bitangents = { ai_mesh->mBitangents[i].x, ai_mesh->mBitangents[i].y, ai_mesh->mBitangents[i].z };
        }

        vertices.push_back(tangents.x);
        vertices.push_back(tangents.y);
        vertices.push_back(tangents.z);
        vertices.push_back(bitangents.x);
        vertices.push_back(bitangents.y);
        vertices.push_back(bitangents.z);
    }

    // -- Process Indices --
    for (uint i = 0; i < ai_mesh->mNumFaces; ++i)
    {
        aiFace face = ai_mesh->mFaces[i];
        for (uint j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    // -- Create Buffers & Mesh --
    BufferLayout layout = { { SHADER_DATA::FLOAT3, "a_Position" }, { SHADER_DATA::FLOAT2, "a_TexCoord" }, { SHADER_DATA::FLOAT3, "a_Normal" },
                            { SHADER_DATA::FLOAT3, "a_Tangent" }, { SHADER_DATA::FLOAT3, "a_Bitangent" } };

    Ref<VertexBuffer> vbo = CreateRef<VertexBuffer>(vertices.data(), vertices.size() * sizeof(float));
    Ref<IndexBuffer> ibo = CreateRef<IndexBuffer>(indices.data(), indices.size());
    Ref<VertexArray> vao = CreateRef<VertexArray>();

    vbo->SetLayout(layout);
    vao->AddVertexBuffer(vbo);
    vao->SetIndexBuffer(ibo);
    vao->Unbind(); vbo->Unbind(); ibo->Unbind();
    return Resources::CreateMesh(vao);
}


const Ref<Material>* MeshImporter::ProcessAssimpMaterial(aiMaterial* ai_material, const std::string& directory)
{
    // -- Ignore Assimp Default Material --
    aiString name = aiString("unnamed");
    ai_material->Get(AI_MATKEY_NAME, name);
    if (name.C_Str() == std::string(AI_DEFAULT_MATERIAL_NAME))
        return nullptr;

    // -- Load Material Variables --
    ai_real shininess = 0.0f, opacity = 1.0f;
    ai_int two_sided = 0;
    aiColor3D diffuse, emissive, specular;

    //ai_material->Get(AI_MATKEY_COLOR_SPECULAR, specular); <- Useless
    ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);         // Kd
    ai_material->Get(AI_MATKEY_COLOR_SPECULAR, specular);       // Ks
    ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive);       // Ke
    ai_material->Get(AI_MATKEY_SHININESS, shininess);           // Ns
    ai_material->Get(AI_MATKEY_OPACITY, opacity);               // d
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

    // -- Return Material --
    return &mat;
}



// ------------------------------------------------------------------------------
std::string MeshImporter::LoadMaterialTexture(aiMaterial* ai_material, aiTextureType texture_type, const std::string& directory)
{
    aiString texture_filename;
    ai_material->GetTexture(texture_type, 0, &texture_filename);
    return FileUtils::MakePath(directory, texture_filename.C_Str());
}
