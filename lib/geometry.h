#pragma once

#include "common.h"
#include <DirectXMath.h>
#include <vector>

struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;

	static const std::vector<D3D12_INPUT_ELEMENT_DESC> LAYOUT_FIELDS;
	static const D3D12_INPUT_LAYOUT_DESC LAYOUT_DESC;
};

