typedef struct Matrix4x3
{
	float4 r[3];
} Matrix4x3;

float4 mul(Matrix4x3 a, float4 b)
{
	float4 ret = {dot(a.r[0], b), dot(a.r[1], b), dot(a.r[2], b), 1};
	return ret;
}

typedef struct Vertex
{
	float4 skinPos;
	float weight[4];
	uint boneIdx[4];
} Vertex;

typedef struct Bone
{
	Matrix4x3	t;
	float		margin;
	float		reserved[3];
}Bone;

__kernel void updateVertices(
	__global Bone* bones,
	__global Vertex* vertices,
	__global float4* out)
{
	int idx = get_global_id(0);
	Vertex v = vertices[idx];
	float4 pos = 0;
	for(int i=0; i<4; ++i)
	{
		if(v.weight[i] > FLT_EPSILON)
		{
			float4 p = {v.skinPos.x, v.skinPos.y, v.skinPos.z, 1};
			p = mul(bones[v.boneIdx[i]].t, p);
			p.w = bones[v.boneIdx[i]].margin;
			p *= v.weight[i];
			pos += p;
		}
	}
	out[idx] = pos;
}