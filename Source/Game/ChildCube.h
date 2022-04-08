#pragma once
#include "Cube/BaseCube.h"

class ChildCube : public BaseCube
{
public:
	virtual void Update(_In_ FLOAT deltaTime);
    virtual HRESULT Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext) override;
};

void ChildCube::Update(_In_ FLOAT deltaTime)
{
    XMMATRIX mSpin = XMMatrixRotationZ(-deltaTime);
    XMMATRIX mOrbit = XMMatrixRotationY(-deltaTime * 2.0f);
    m_world *= mOrbit * mSpin;
}

HRESULT ChildCube::Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
{
    HRESULT hr = Renderable::initialize(pDevice, pImmediateContext);
    XMMATRIX mTranslate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
    XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);
    m_world *= mScale * mTranslate;
    return hr;
}
