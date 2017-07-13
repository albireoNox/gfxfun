#pragma once

#include "common.h"

template<typename T>
class UploadBuffer : public NON_COPYABLE
{
public:
	UploadBuffer(ID3D12Device* device, uint elementCount, bool isConstant) :
		elementCount(elementCount),
	    isConstant(isConstant), 
		// Make elements in constant buffers be 256-byte aligned.  
		byteCountPerElement(isConstant ? (sizeof(T) + 255) & ~0xff : sizeof(T))
	{
		hrThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(this->byteCountPerElement * elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(this->uploadBuffer.GetAddressOf())));

		hrThrowIfFailed(this->uploadBuffer->Map(0, nullptr, &this->mappedData));
	}

	~UploadBuffer()
	{
		if (this->uploadBuffer != nullptr)
			this->uploadBuffer->Unmap(0, nullptr);
		this->uploadBuffer = nullptr;
	}

	ID3D12Resource*
	resource()const
	{
		return this->uploadBuffer.Get();
	}

	void
	copyData(uint elementIndex, const T& data)
	{
		const uint64 byteOffset = elementIndex * this->byteCountPerElement;
		memcpy(&this->mappedData[byteOffset], &data, sizeof(T));
	}

	const uint byteCountPerElement;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
	void* mappedData = nullptr;

	uint elementCount;
	bool isConstant;
};