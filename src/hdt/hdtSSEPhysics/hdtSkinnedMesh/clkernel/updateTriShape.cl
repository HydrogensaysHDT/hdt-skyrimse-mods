typedef union Aabb
{
	struct{
		float3 aabbMin;
		float _reserved0;
		float3 aabbMax;
		float _reserved1;
	};
	struct{ float4 a[2]; };
} Aabb;

void aabbMerge(Aabb* aabb, float3 pt)
{
	aabb->aabbMin = fmin(aabb->aabbMin, pt);
	aabb->aabbMax = fmax(aabb->aabbMax, pt);
}

void aabbExpand(Aabb* aabb, float margin)
{
	aabb->aabbMin -= margin;
	aabb->aabbMax += margin;
}

typedef struct Collider
{
	Aabb aabb;
	uint isKinematic;
	uint vertex;
	uint reserved[2];
} Collider;

__kernel void updateCollider(__global float4* vertices, __global Collider* colliders, float margin)
{
	int idx = get_global_id(0);
	float3 p0 = vertices[colliders[idx].vertex].s012;
	
	Aabb aabb;
	aabb.aabbMin = p0 - margin;
	aabb.aabbMax = p0 + margin;
	
	colliders[idx].aabb = aabb;
}