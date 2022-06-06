#include "Renderer/Skybox.h"

#include "assimp/Importer.hpp"	// C++ importer interface
#include "assimp/scene.h"		// output data structure
#include "assimp/postprocess.h"	// post processing flags

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Skybox::Skybox

      Summary:  Constructor

      Args:     const std::filesystem::path& cubeMapFilePath
                  Path to the cube map texture to use
                FLOAT scale
                  Scaling factor

      Modifies: [m_cubeMapFileName, m_scale].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Skybox::Skybox definition (remove the comment)
    --------------------------------------------------------------------*/
    Skybox::Skybox(_In_ const std::filesystem::path& cubeMapFilePath, _In_ FLOAT scale)
        : Model(L"Content/Common/Sphere.obj")
        , m_cubeMapFileName(cubeMapFilePath)
        , m_scale(scale)
    {

    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Skybox::Initialize

      Summary:  Initializes the skybox and cube map texture

      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the buffers
                ID3D11DeviceContext* pImmediateContext
                  The Direct3D context to set buffers

      Modifies: [m_aMeshes, m_aMaterials].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Skybox::Initialize definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Skybox::Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
    {
        HRESULT hr = S_OK;
        hr = Model::Initialize(pDevice, pImmediateContext);
        if (FAILED(hr))
        {
            return hr;
        }
        Scale(m_scale, m_scale, m_scale);
        m_aMeshes[0].uMaterialIndex = 0;
        m_aMaterials[0]->pDiffuse = std::make_shared<Texture>(m_cubeMapFileName);
        hr = GetMaterial(0u)->pDiffuse->Initialize(pDevice, pImmediateContext);
        if (FAILED(hr))
        {
            return hr;
        }
        return hr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Skybox::GetSkyboxTexture

      Summary:  Returns the cube map texture

      Returns:  const std::shared_ptr<Texture>&
                  Cube map texture object
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Skybox::GetSkyboxTexture definition (remove the comment)
    --------------------------------------------------------------------*/
    const std::shared_ptr<Texture>& Skybox::GetSkyboxTexture() const
    {
        return GetMaterial(0u)->pDiffuse;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Skybox::initSingleMesh

      Summary:  Initialize single mesh from a given assimp mesh

      Args:     UINT uMeshIndex
                  Mesh index
                const aiMesh* pMesh
                  Point to an assimp mesh object
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Skybox::initSingleMesh definition (remove the comment)
    --------------------------------------------------------------------*/
    void Skybox::initSingleMesh(_In_ UINT uMeshIndex, _In_ const aiMesh* pMesh)
    {
        const aiVector3D zero3d(0.0f, 0.0f, 0.0f);
        for (UINT i = 0; i < pMesh->mNumVertices; i++)
        {
            const aiVector3D& position = pMesh->mVertices[i];
            const aiVector3D& normal = pMesh->mNormals[i];
            const aiVector3D& texCoord = pMesh->HasTextureCoords(0u) ? pMesh->mTextureCoords[0][i] : zero3d;
            const aiVector3D& tangent = pMesh->HasTangentsAndBitangents() ? pMesh->mTangents[i] : zero3d;
            const aiVector3D& Bitangent = pMesh->HasTangentsAndBitangents() ? pMesh->mBitangents[i] : zero3d;
            SimpleVertex vertex = {
                .Position = XMFLOAT3(position.x, position.y, position.z),
                .TexCoord = XMFLOAT2(texCoord.x, texCoord.y),
                .Normal = XMFLOAT3(normal.x, normal.y, normal.z)
            };
            NormalData normalData = {
                .Tangent = XMFLOAT3(tangent.x, tangent.y, tangent.z),
                .Bitangent = XMFLOAT3(Bitangent.x, Bitangent.y, Bitangent.z)
            };
            m_aVertices.push_back(vertex);
            m_aNormalData.push_back(normalData);
        }

        for (UINT i = 0; i < pMesh->mNumFaces; i++)
        {
            const aiFace& Face = pMesh->mFaces[i];
            assert(Face.mNumIndices == 3);
            WORD aIndices[3] = {
                static_cast<WORD>(Face.mIndices[0]),
                static_cast<WORD>(Face.mIndices[1]),
                static_cast<WORD>(Face.mIndices[2])
            };
            m_aIndices.push_back(aIndices[2]);
            m_aIndices.push_back(aIndices[1]);
            m_aIndices.push_back(aIndices[0]);
        }
        initMeshBones(uMeshIndex, pMesh);
    }
}