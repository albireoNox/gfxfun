#pragma once

#include "lib/mesh.h"
#include <array>

class DemoBox : public StaticMesh
{
public:
	virtual uint vertexCount()const override;
	virtual const Vertex* vertexData()const override;

	virtual uint indexCount()const override;
	virtual const uint16* indexData()const override;

private:
	static const std::array<Vertex, 8> vertices;
	static const std::array<uint16, 36> indices;
};