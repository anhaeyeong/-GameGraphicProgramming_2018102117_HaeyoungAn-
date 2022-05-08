#pragma once
#include "Cube/BaseCube.h"

class StrangeCube : public BaseCube
{
public:
    virtual void Update(_In_ FLOAT deltaTime) override;
};


void StrangeCube::Update(_In_ FLOAT deltaTime)
{
    static FLOAT s_totalTime = 0.0f;
    s_totalTime += deltaTime;
    XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);

    m_world = mScale * XMMatrixRotationY(s_totalTime) * XMMatrixTranslation(-3.0f, XMScalarSin(s_totalTime), 0.0f) * XMMatrixRotationZ(s_totalTime) * XMMatrixRotationX(s_totalTime);
}

