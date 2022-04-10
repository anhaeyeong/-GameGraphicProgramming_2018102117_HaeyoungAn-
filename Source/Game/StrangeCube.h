#pragma once
#include "Cube/BaseCube.h"

class StrangeCube : public BaseCube
{
public:
    StrangeCube();
    virtual void Update(_In_ FLOAT deltaTime);
    virtual HRESULT Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext) override;
private:
    XMMATRIX mSpin;
    XMMATRIX mOrbit;
    XMMATRIX mTranslate;
    XMMATRIX mScale;
    XMMATRIX mZSpin;
    void SetSpin(_In_ FLOAT deltaTime)
    {
        mSpin *= XMMatrixRotationX(deltaTime * 5.0f);
    }

    void SetOrbit(_In_ FLOAT deltaTime)
    {
        mOrbit *= XMMatrixRotationY(-deltaTime * 5.0f);
    }

    void SetZ(_In_ FLOAT deltaTime)
    {
        mZSpin *= XMMatrixRotationZ(deltaTime * 5.0f);
    }
};

StrangeCube::StrangeCube()
{
    mSpin = XMMatrixIdentity();
    mOrbit = XMMatrixIdentity();
    mTranslate = XMMatrixIdentity();
    mScale = XMMatrixIdentity();
    mZSpin = XMMatrixIdentity();
}

void StrangeCube::Update(_In_ FLOAT deltaTime)
{
    mTranslate = XMMatrixTranslation(0.0f, 2.0f, 2.0f);
    mScale = XMMatrixScaling(0.1f, 0.7f, 0.1f);
    m_world = mScale * mTranslate * mSpin * mOrbit * mZSpin;

    SetSpin(deltaTime);
    SetOrbit(deltaTime);
    SetZ(deltaTime);
    
}

HRESULT StrangeCube::Initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
{
    HRESULT hr = Renderable::initialize(pDevice, pImmediateContext);
    return hr;
}
