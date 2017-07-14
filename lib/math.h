#pragma once

#include <DirectXMath.h>

namespace Math
{
	static DirectX::XMFLOAT4X4 Identity4x4()
	{
		static DirectX::XMFLOAT4X4 I(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

		return I;
	}

	void printMx(DirectX::XMFLOAT4X4& mx);

	const float Infinity = FLT_MAX;
	const float Pi = 3.1415926535f;
}

