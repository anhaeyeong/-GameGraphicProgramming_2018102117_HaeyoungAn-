#include "Renderer/Renderer.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer

      Summary:  Constructor

      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                 m_immediateContext, m_immediateContext1, m_swapChain,
                 m_swapChain1, m_renderTargetView, m_depthStencil,
                 m_depthStencilView, m_cbChangeOnResize, m_camera,
                 m_projection, m_renderables, m_vertexShaders, 
                 m_pixelShaders].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Renderer definition (remove the comment)
    --------------------------------------------------------------------*/
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL)
        , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
        , m_renderables()
        , m_vertexShaders()
        , m_pixelShaders()
        , m_camera({ 0.0f, 0.0f, -5.0f, 0.0f })
        , m_projection()
    {
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize

      Summary:  Creates Direct3D device and swap chain

      Args:     HWND hWnd
                  Handle to the window

      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                 m_d3dDevice1, m_immediateContext1, m_swapChain1,
                 m_swapChain, m_renderTargetView, m_cbChangeOnResize, 
                 m_projection, m_cbLights, m_camera, m_vertexShaders, 
                 m_pixelShaders, m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Initialize definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        UINT createDeviceFlags = 0;
#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT numDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            m_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
                break;
        }
        if (FAILED(hr))
            return hr;


        ComPtr<IDXGIFactory1> dxgiFactory;
        {
            ComPtr<IDXGIDevice> dxgiDevice;

            if (SUCCEEDED(m_d3dDevice.As(&dxgiDevice)))
            {
                ComPtr<IDXGIAdapter> adapter;
                hr = dxgiDevice->GetAdapter(&adapter);
                if (SUCCEEDED(hr))
                {
                    adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
                }
            }
        }
        if (FAILED(hr))
            return hr;

        // Create swap chain
        ComPtr<IDXGIFactory2> dxgiFactory2;
        hr = dxgiFactory.As(&dxgiFactory2);
        if (dxgiFactory2)
        {
            // DirectX 11.1 or later

            if (SUCCEEDED(m_d3dDevice.As(&m_d3dDevice1)))
            {
                m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd = {
            .Width = width,
            .Height = height,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .SampleDesc = {.Count = 1, .Quality = 0 },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 1
            };
            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd = {
            .BufferDesc = {
                            .Width = width,
                            .Height = height,
                            .RefreshRate = {.Numerator = 60,
                                            .Denominator = 1},
                            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                           },
            .SampleDesc = {
                    .Count = 1,
                    .Quality = 0
                           },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 1,
            .OutputWindow = hWnd,
            .Windowed = TRUE,
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);



        if (FAILED(hr))
            return hr;

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer;
        hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr))
            return hr;

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());


        if (FAILED(hr))
            return hr;

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

        // Setup the viewport
        D3D11_VIEWPORT vp = {
        .TopLeftX = 0,
        .TopLeftY = 0,
        .Width = (FLOAT)width,
        .Height = (FLOAT)height,
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f
        };
        m_immediateContext->RSSetViewports(1, &vp);




        D3D11_TEXTURE2D_DESC descDepth = {
            .Width = width,
            .Height = height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {.Count = 1, .Quality = 0 },
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0,
            .MiscFlags = 0
        };

        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
        descDSV.Format = descDepth.Format;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0;
        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }
        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f);
        D3D11_BUFFER_DESC bd = {
        .ByteWidth = sizeof(CBChangeOnResize),
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
        .CPUAccessFlags = 0,
        .MiscFlags = 0,
        };
        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }
        bd.ByteWidth = sizeof(CBLights) * NUM_LIGHTS;
        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbLights.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        bd.ByteWidth = sizeof(InstanceData) * NUM_LIGHTS;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA initData = {
            .pSysMem = &bd
        };

        CBChangeOnResize Pcb;
        Pcb.Projection = XMMatrixTranspose(m_projection);
        m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, NULL, &Pcb, 0, 0);
        m_immediateContext->VSSetConstantBuffers(1u, 1u, m_cbChangeOnResize.GetAddressOf());
        for (auto i : m_vertexShaders)
        {
            i.second->Initialize(m_d3dDevice.Get());
        }

        for (auto i : m_pixelShaders)
        {
            i.second->Initialize(m_d3dDevice.Get());
        }

        for (auto i : m_renderables)
        {
            i.second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
        }
        m_camera.Initialize(m_d3dDevice.Get());

        for (auto i : m_scenes)
        {
            for (auto j : i.second->GetVoxels())
            {
                j->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            }
        }
        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        return hr;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddRenderable

      Summary:  Add a renderable object

      Args:     PCWSTR pszRenderableName
                  Key of the renderable object
                const std::shared_ptr<Renderable>& renderable
                  Shared pointer to the renderable object

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::AddRenderable definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
    {
        HRESULT hr = S_OK;
        if (m_renderables.empty())
        {
            m_renderables.insert({ pszRenderableName, renderable });
        }
        else
        {
            if (m_renderables.find(pszRenderableName) != m_renderables.end())
            {
                hr = E_FAIL;
                return hr;
            }
            else
            {
                m_renderables.insert({ pszRenderableName, renderable });
            }
        }
        return hr;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPointLight

      Summary:  Add a point light

      Args:     size_t index
                  Index of the point light
                const std::shared_ptr<PointLight>& pointLight
                  Shared pointer to the point light object

      Modifies: [m_aPointLights].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::AddPointLight definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::AddPointLight(_In_ size_t index, _In_ const std::shared_ptr<PointLight>& pPointLight)
    {
        HRESULT hr = S_OK;
        if (index >= NUM_LIGHTS)
        {
            hr = E_FAIL;
        }
        else
        {
            m_aPointLights[index] = pPointLight;
        }
        return hr;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddVertexShader

      Summary:  Add the vertex shader into the renderer

      Args:     PCWSTR pszVertexShaderName
                  Key of the vertex shader
                const std::shared_ptr<VertexShader>&
                  Vertex shader to add

      Modifies: [m_vertexShaders].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::AddVertexShader definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
    {
        HRESULT hr = S_OK;
        if (m_vertexShaders.empty())
        {
            m_vertexShaders.insert({ pszVertexShaderName, vertexShader });
        }
        else
        {
            if (m_vertexShaders.find(pszVertexShaderName) != m_vertexShaders.end())
            {
                hr = E_FAIL;
                return hr;
            }
            else
            {
                m_vertexShaders.insert({ pszVertexShaderName, vertexShader });
            }
        }
        return hr;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPixelShader

      Summary:  Add the pixel shader into the renderer

      Args:     PCWSTR pszPixelShaderName
                  Key of the pixel shader
                const std::shared_ptr<PixelShader>&
                  Pixel shader to add

      Modifies: [m_pixelShaders].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::AddPixelShader definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
    {
        HRESULT hr = S_OK;
        if (m_pixelShaders.empty())
        {
            m_pixelShaders.insert({ pszPixelShaderName, pixelShader });
        }
        else
        {
            if (m_pixelShaders.find(pszPixelShaderName) != m_pixelShaders.end())
            {
                hr = E_FAIL;
                return hr;
            }
            else
            {
                m_pixelShaders.insert({ pszPixelShaderName, pixelShader });
            }
        }
        return hr;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddScene

      Summary:  Add a scene

      Args:     PCWSTR pszSceneName
                  Key of a scene
                const std::filesystem::path& sceneFilePath
                  File path to initialize a scene

      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::AddScene definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, const std::filesystem::path& sceneFilePath)
    {
        HRESULT hr = S_OK;
        std::shared_ptr<Scene> scene(new Scene(sceneFilePath));

        if (m_scenes.empty())
        {
            m_scenes.insert({ pszSceneName, scene });
        }
        else
        {
            if (m_scenes.find(pszSceneName) != m_scenes.end())
            {
                hr = E_FAIL;
                return hr;
            }
            else
            {
                m_scenes.insert({ pszSceneName, scene });
            }
        }
        return hr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetMainScene

      Summary:  Set the main scene

      Args:     PCWSTR pszSceneName
                  Name of the scene to set as the main scene

      Modifies: [m_pszMainSceneName].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::SetMainScene definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
    {
        HRESULT hr = S_OK;
        if (m_scenes.find(pszSceneName) != m_scenes.end())
        {
            m_pszMainSceneName = pszSceneName;
            return hr;
        }
        else
        {
            hr = E_FAIL;
            return hr;
        }
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::HandleInput

      Summary:  Add the pixel shader into the renderer and initialize it

      Args:     const DirectionsInput& directions
                  Data structure containing keyboard input data
                const MouseRelativeMovement& mouseRelativeMovement
                  Data structure containing mouse relative input data

      Modifies: [m_camera].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::HandleInput definition (remove the comment)
    --------------------------------------------------------------------*/
    void Renderer::HandleInput(_In_ const DirectionsInput& directions, _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        m_camera.HandleInput(directions, mouseRelativeMovement, deltaTime);
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update

      Summary:  Update the renderables each frame

      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Update definition (remove the comment)
    --------------------------------------------------------------------*/
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        for (auto i : m_renderables)
        {
            i.second->Update(deltaTime);
        }
        m_camera.Update(deltaTime);
        for (auto i : m_aPointLights)
        {
            i->Update(deltaTime);
        }
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render

      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::Render definition (remove the comment)
    --------------------------------------------------------------------*/
    void Renderer::Render()
    {
        float ClearColor[4] = { 0.0f, 0.125f, 0.6f, 1.0f }; // RGBA
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), ClearColor);
        m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
        UINT uStride = sizeof(SimpleVertex);
        UINT uOffset = 0;
        CBChangeOnCameraMovement Vcb;
        Vcb.View = XMMatrixTranspose(m_camera.GetView());
        XMStoreFloat4(&Vcb.CameraPosition, m_camera.GetEye());
        m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0, NULL, &Vcb, 0, 0);
        m_immediateContext->VSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
        CBLights Lcb;
        for (int i = 0; i < NUM_LIGHTS; i++)
        {
            Lcb.LightColors[i] = m_aPointLights[i]->GetColor();
            Lcb.LightPositions[i] = m_aPointLights[i]->GetPosition();
        }
        m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0, NULL, &Lcb, 0, 0);
        m_immediateContext->VSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());
        m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());
        for (auto i : m_renderables) {
            m_immediateContext->IASetVertexBuffers(0u, 1u, i.second->GetVertexBuffer().GetAddressOf(), &uStride, &uOffset);
            m_immediateContext->IASetIndexBuffer(i.second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);
            m_immediateContext->IASetInputLayout(i.second->GetVertexLayout().Get());
            CBChangesEveryFrame Wcb;
            Wcb.World = XMMatrixTranspose(i.second->GetWorldMatrix());
            Wcb.OutputColor = i.second->GetOutputColor();
            m_immediateContext->UpdateSubresource(i.second->GetConstantBuffer().Get(), 0, NULL, &Wcb, 0, 0);
            m_immediateContext->VSSetShader(i.second->GetVertexShader().Get(), nullptr, 0);
            m_immediateContext->VSSetConstantBuffers(2u, 1u, i.second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetShader(i.second->GetPixelShader().Get(), nullptr, 0);
            
            
            if (i.second->HasTexture())
            {
                m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
                for (UINT j = 0; j < i.second->GetNumMeshes(); j++)
                {
                    const UINT MaterialIndex = i.second->GetMesh(j).uMaterialIndex;
                    if (i.second->GetMaterial(MaterialIndex).pDiffuse)
                    {
                        m_immediateContext->PSSetShaderResources(0u, 1u, i.second->GetMaterial(MaterialIndex).pDiffuse->GetTextureResourceView().GetAddressOf());
                        m_immediateContext->PSSetSamplers(0u, 1u, i.second->GetMaterial(MaterialIndex).pDiffuse->GetSamplerState().GetAddressOf());
                    }
                    m_immediateContext->DrawIndexed(i.second->GetMesh(j).uNumIndices, i.second->GetMesh(j).uBaseIndex, i.second->GetMesh(j).uBaseVertex);
                }
            }
            else
            {
                m_immediateContext->PSSetConstantBuffers(2u, 1u, i.second->GetConstantBuffer().GetAddressOf());
                m_immediateContext->DrawIndexed(i.second->GetNumIndices(), 0, 0);
            }
        }
        UINT strides[2] = { sizeof(SimpleVertex), sizeof(InstanceData) };
        UINT offsets[2] = { 0, 0 };
        
        for (auto i : m_scenes)
        {
            if (i.first == m_pszMainSceneName)
            {
                for (auto j : i.second->GetVoxels())
                {
                    ID3D11Buffer* buffers[2] = { j->GetVertexBuffer().Get(), j->GetInstanceBuffer().Get() };
                    m_immediateContext->IASetVertexBuffers(0u, 2u, buffers, strides, offsets);
                    m_immediateContext->IASetIndexBuffer(j->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);
                    m_immediateContext->IASetInputLayout(j->GetVertexLayout().Get());
                    CBChangesEveryFrame Wcb;
                    Wcb.World = XMMatrixTranspose(j->GetWorldMatrix());
                    Wcb.OutputColor = j->GetOutputColor();
                    m_immediateContext->UpdateSubresource(j->GetConstantBuffer().Get(), 0, NULL, &Wcb, 0, 0);
                    m_immediateContext->VSSetShader(j->GetVertexShader().Get(), nullptr, 0);
                    m_immediateContext->VSSetConstantBuffers(2u, 1u, j->GetConstantBuffer().GetAddressOf());
                    m_immediateContext->PSSetShader(j->GetPixelShader().Get(), nullptr, 0);
                    m_immediateContext->PSSetConstantBuffers(2u, 1u, j->GetConstantBuffer().GetAddressOf());
                    
                    //UINT instanceLocation = 0;
                    m_immediateContext->DrawIndexedInstanced(j->GetNumIndices(), j->GetNumInstances(), 0, 0, 0);
                    //instanceLocation += j->GetNumInstances();
                }
            }
        }
        m_swapChain->Present(0, 0);
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfRenderable

      Summary:  Sets the vertex shader for a renderable

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::SetVertexShaderOfRenderable definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
    {
        HRESULT hr = S_OK;
        std::unordered_map<std::wstring, std::shared_ptr<Renderable>>::const_iterator renderable = m_renderables.find(pszRenderableName);
        std::unordered_map<std::wstring, std::shared_ptr<VertexShader>>::const_iterator vs = m_vertexShaders.find(pszVertexShaderName);
        if (renderable != m_renderables.end() && vs != m_vertexShaders.end())
        {
            renderable->second->SetVertexShader(vs->second);
        }
        else
        {
            hr = E_FAIL;
            return hr;
        }
        return hr;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfRenderable

      Summary:  Sets the pixel shader for a renderable

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::SetPixelShaderOfRenderable definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
    {
        HRESULT hr = S_OK;
        std::unordered_map<std::wstring, std::shared_ptr<Renderable>>::const_iterator renderable = m_renderables.find(pszRenderableName);
        std::unordered_map<std::wstring, std::shared_ptr<PixelShader>>::const_iterator ps = m_pixelShaders.find(pszPixelShaderName);
        if (renderable != m_renderables.end() && ps != m_pixelShaders.end())
        {
            renderable->second->SetPixelShader(ps->second);
        }
        else
        {
            hr = E_FAIL;
            return hr;
        }
        return hr;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfScene

      Summary:  Sets the vertex shader for the voxels in a scene

      Args:     PCWSTR pszSceneName
                  Key of the scene
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::SetVertexShaderOfScene definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::SetVertexShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszVertexShaderName)
    {
        HRESULT hr = S_OK;
        std::unordered_map<std::wstring, std::shared_ptr<Scene>>::const_iterator scene = m_scenes.find(pszSceneName);
        std::unordered_map<std::wstring, std::shared_ptr<VertexShader>>::const_iterator vs = m_vertexShaders.find(pszVertexShaderName);
        if (scene != m_scenes.end() && vs != m_vertexShaders.end())
        {
            for (auto i : scene->second->GetVoxels())
            {
                i->SetVertexShader(vs->second);
            }
        }
        else
        {
            hr = E_FAIL;
            return hr;
        }
        return hr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfScene

      Summary:  Sets the pixel shader for the voxels in a scene

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::SetPixelShaderOfScene definition (remove the comment)
    --------------------------------------------------------------------*/
    HRESULT Renderer::SetPixelShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszPixelShaderName)
    {
        HRESULT hr = S_OK;
        std::unordered_map<std::wstring, std::shared_ptr<Scene>>::const_iterator scene = m_scenes.find(pszSceneName);
        std::unordered_map<std::wstring, std::shared_ptr<PixelShader>>::const_iterator ps = m_pixelShaders.find(pszPixelShaderName);
        if (scene != m_scenes.end() && ps != m_pixelShaders.end())
        {
            for (auto i : scene->second->GetVoxels())
            {
                i->SetPixelShader(ps->second);
            }
        }
        else
        {
            hr = E_FAIL;
            return hr;
        }
        return hr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetDriverType

      Summary:  Returns the Direct3D driver type

      Returns:  D3D_DRIVER_TYPE
                  The Direct3D driver type used
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    /*--------------------------------------------------------------------
      TODO: Renderer::GetDriverType definition (remove the comment)
    --------------------------------------------------------------------*/
    D3D_DRIVER_TYPE Renderer::GetDriverType() const
    {
        return m_driverType;
    }
}