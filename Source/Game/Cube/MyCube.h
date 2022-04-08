#pragma once
#include "Cube/BaseCube.h"

class MyCube : public BaseCube
{
public:
	virtual void Update(_In_ FLOAT deltaTime);
};

void MyCube::Update(_In_ FLOAT deltaTime)
{
	m_world *= XMMatrixRotationY(deltaTime);
}
