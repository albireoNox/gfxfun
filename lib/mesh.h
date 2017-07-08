#pragma once

#include "common.h"
#include "geometry.h"
#include <string>

class StaticMesh
{
public:
	void loadGeometry(ID3D12Device *device, ID3D12GraphicsCommandList* cmdList);
	void cleanUpLoadArtifacts();
	void draw(ID3D12GraphicsCommandList* cmdList)const; 

	inline uint vertexBufferByteCount()const { return this->vertexCount() * sizeof(Vertex); }
	static const UINT VERTEX_BYTE_STRIDE = sizeof(Vertex);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView()const;

	inline uint indexBufferByteCount()const { return this->indexCount() * sizeof(uint16); }
	static const DXGI_FORMAT INDEX_FORMAT = DXGI_FORMAT_R16_UINT;
	D3D12_INDEX_BUFFER_VIEW indexBufferView()const;

	// Give it a name so we can look it up by name.
	const std::wstring name;

protected:

	virtual uint vertexCount()const = 0;
	virtual const Vertex* vertexData()const = 0;
	
	virtual uint indexCount()const = 0;
	virtual const uint16* indexData()const = 0;

	// TODO: Have multiple meshes share buffers via some mesh manager. 
	// System memory copies.  Use Blobs because the vertex/index format can be generic.
	// It is up to the client to cast appropriately.  
	Microsoft::WRL::ComPtr<ID3DBlob> vertexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> indexBufferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUploader = nullptr;
};
