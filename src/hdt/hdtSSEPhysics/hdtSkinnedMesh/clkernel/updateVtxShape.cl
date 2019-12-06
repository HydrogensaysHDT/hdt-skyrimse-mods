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
	uint vertices[3];
} Collider;

__kernel void updateCollider(__global float4* vertices, __global Collider* colliders, float penetration, float margin)
{
	int idx = get_global_id(0);
	float3 p0 = vertices[colliders[idx].vertices[0]].s012;
	float3 p1 = vertices[colliders[idx].vertices[1]].s012;
	float3 p2 = vertices[colliders[idx].vertices[2]].s012;
	
	Aabb aabb;
	aabb.aabbMin = aabb.aabbMax = p0;
	aabbMerge(&aabb, p1);
	aabbMerge(&aabb, p2);
	aabbExpand(&aabb, margin);
	
	if(penetration > FLT_EPSILON || penetration < -FLT_EPSILON)
	{
		float3 normal = cross((p1-p0).s012, (p2-p0).s012);
		float len2 = dot(normal, normal);
		if(len2 > FLT_EPSILON * FLT_EPSILON)
		{
			normal = normalize(normal) * penetration;
			aabb.aabbMin = fmin(aabb.aabbMin, aabb.aabbMin - normal);
			aabb.aabbMax = fmax(aabb.aabbMax, aabb.aabbMax - normal);
		}
	}
	colliders[idx].aabb = aabb;
}