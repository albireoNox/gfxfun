struct ObjectData {
	// Matrix equal to model * view * projection 
	float4x4 worldViewProj;
};
ConstantBuffer<ObjectData> gObjData : register(b0);

void main(
	float3 pos : POSITION, 
	float4 color : COLOR, 
	out float4 posOut : SV_POSITION, 
	out float4 colorOut : COLOR)
{
	posOut = mul(float4(pos, 1.0), gObjData.worldViewProj);
	colorOut = color;
}

