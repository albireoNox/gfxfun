#include "mesh.h"
#include "d3d_util.h"
#include <d3dcompiler.h>

void
StaticMesh::loadGeometry(
	ID3D12Device *device, ID3D12GraphicsCommandList* cmdList)
{
	const uint vbByteCount = this->vertexBufferByteCount();
	const uint ibByteCount = this->indexBufferByteCount();

	hrThrowIfFailed(D3DCreateBlob(vbByteCount, this->vertexBufferCPU.GetAddressOf()));
	CopyMemory(this->vertexBufferCPU->GetBufferPointer(), this->vertexData(), vbByteCount);

	hrThrowIfFailed(D3DCreateBlob(ibByteCount, this->indexBufferCPU.GetAddressOf()));
	CopyMemory(this->indexBufferCPU->GetBufferPointer(), this->indexData(), ibByteCount);

	this->vertexBufferGPU = createDefaultBuffer(
		device, cmdList, this->vertexData(), vbByteCount, this->vertexBufferUploader);

	this->indexBufferGPU = createDefaultBuffer(
		device, cmdList, this->indexData(), ibByteCount, this->indexBufferUploader);
}

void
StaticMesh::cleanUpLoadArtifacts()
{
	// This will release uploader resources. 
	this->vertexBufferUploader = nullptr;
	this->indexBufferUploader = nullptr;
}

void 
StaticMesh::draw(ID3D12GraphicsCommandList* cmdList)const
{
	cmdList->IASetVertexBuffers(0, 1, &this->vertexBufferView());
	cmdList->IASetIndexBuffer(&this->indexBufferView());
	cmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->DrawIndexedInstanced(
		this->indexCount(), 1, 0, 0, 0);
}

D3D12_VERTEX_BUFFER_VIEW
StaticMesh::vertexBufferView()const
{
	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = this->vertexBufferGPU->GetGPUVirtualAddress();
	vbv.StrideInBytes = StaticMesh::VERTEX_BYTE_STRIDE;
	vbv.SizeInBytes = this->vertexBufferByteCount();

	return vbv;
}

D3D12_INDEX_BUFFER_VIEW
StaticMesh::indexBufferView()const
{
	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = this->indexBufferGPU->GetGPUVirtualAddress();
	ibv.Format = StaticMesh::INDEX_FORMAT;
	ibv.SizeInBytes = this->indexBufferByteCount();

	return ibv;
}
