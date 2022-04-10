#pragma once
#include "Cube/BaseCube.h"

class ChildCube : public BaseCube
{
public:
    ChildCube();
	virtual void Update(_In_ FLOAT deltaTime);
    virtual HRESULT Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext) override;
private:
    XMMATRIX mSpin;
    XMMATRIX mOrbit;
    XMMATRIX mTranslate;
    XMMATRIX mScale;
    void SetSpin(_In_ FLOAT deltaTime)
    {
        mSpin *= XMMatrixRotationZ(deltaTime);
    }

    void SetOrbit(_In_ FLOAT deltaTime)
    {
        mOrbit *= XMMatrixRotationY(-deltaTime * 2.0f);
    }
};

ChildCube::ChildCube()
{
    mSpin = XMMatrixIdentity();
    mOrbit = XMMatrixIdentity();
    mTranslate = XMMatrixIdentity();
    mScale = XMMatrixIdentity();
}

void ChildCube::Update(_In_ FLOAT deltaTime)
{
    mTranslate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
    mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);
    m_world = mScale * mSpin * mTranslate * mOrbit;

    SetSpin(deltaTime);
    SetOrbit(deltaTime);
}

HRESULT ChildCube::Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
{
    HRESULT hr = Renderable::initialize(pDevice, pImmediateContext);
    return hr;
}
