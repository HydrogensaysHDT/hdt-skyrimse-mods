typedef struct Aabb
{
	float4 aabbMin;
	float4 aabbMax;
} Aabb;

__kernel void updateCollider(__global float4* vertices, __global uint4* colliders, __global Aabb* aabbs, float margin)
{
	int idx = get_global_id(0);
	float3 p0 = vertices[colliders[idx].x].s012;
	
	Aabb aabb;
	aabb.aabbMin.xyz = p0 - margin;
	aabb.aabbMax.xyz = p0 + margin;
	
	aabbs[idx] = aabb;
}