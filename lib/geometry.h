#pragma once

#include "common.h"
#include <DirectXMath.h>
#include <vector>

/// VERTEX ///

struct Vertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;

	static const std::vector<D3D12_INPUT_ELEMENT_DESC> LAYOUT_FIELDS;
	static const D3D12_INPUT_LAYOUT_DESC LAYOUT_DESC;
};

const std::vector<D3D12_INPUT_ELEMENT_DESC> Vertex::LAYOUT_FIELDS(
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(DirectX::XMFLOAT3),
	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
});

const D3D12_INPUT_LAYOUT_DESC Vertex::LAYOUT_DESC =
{ Vertex::LAYOUT_FIELDS.data(), Vertex::LAYOUT_FIELDS.size() };
