// UE4 Procedural Mesh Generation from the Epic Wiki (https://wiki.unrealengine.com/Procedural_Mesh_Generation)
//
// forked from "Engine/Plugins/Runtime/CustomMeshComponent/Source/CustomMeshComponent/Classes/CustomMeshComponent.h"

#pragma once

#include <mutex>
#include "glm.hpp"
#include "CustomProceduralMeshComponent.generated.h"

class UCustomProceduralMeshComponent;
class FProceduralMeshRenderPacket;

class FProceduralMeshTriData
{
public:
	FProceduralMeshTriData()
	{
		FProceduralMeshTriData(nullptr, nullptr, nullptr, 0, 0, nullptr, nullptr);
	}

	FProceduralMeshTriData(glm::uint32 * indices_in,
		glm::float32 * points_in,
		glm::float32 * uvs_in,
		int32 point_num_in,
		int32 indices_num_in,
		TArray<uint8> * region_alphas_in,
		std::mutex * update_lock_in)
	{
		indices = indices_in;
		points = points_in;
		uvs = uvs_in;
		point_num = point_num_in;
		indices_num = indices_num_in;
		region_alphas = region_alphas_in;
		update_lock = update_lock_in;
	}

	glm::uint32 * indices;
	glm::float32 * points;
	glm::float32 * uvs;
	int32 point_num, indices_num;
	TArray<uint8> * region_alphas;
	std::mutex * update_lock;
};

/** Scene proxy */
class FProceduralMeshSceneProxy : public FPrimitiveSceneProxy
{
public:

	FProceduralMeshSceneProxy(UCustomProceduralMeshComponent* Component,
		FProceduralMeshTriData * targetTrisIn);

	virtual ~FProceduralMeshSceneProxy();

	void AddRenderPacket(FProceduralMeshTriData * targetTrisIn);


	void SetActiveRenderPacketIdx(int idxIn);

	void UpdateDynamicIndexData();

	void UpdateDynamicComponentData();

	void SetNeedsMaterialUpdate(bool flag_in);

	void UpdateMaterial();

	void DoneUpdating();

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views,
		const FSceneViewFamily& ViewFamily,
		uint32 VisibilityMap,
		FMeshElementCollector& Collector) const override;


	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View);

	virtual bool CanBeOccluded() const override;

	virtual uint32 GetMemoryFootprint(void) const;

	uint32 GetAllocatedSize(void) const;

	FProceduralMeshRenderPacket * GetActiveRenderPacket();

private:
	UCustomProceduralMeshComponent* parentComponent;
	UMaterialInterface* Material;
	TArray<FProceduralMeshRenderPacket> renderPackets;
	int active_render_packet_idx;

	FMaterialRelevance MaterialRelevance;
	bool needs_updating;
	bool needs_index_updating;
	bool needs_material_updating;
};

USTRUCT(BlueprintType)
struct FProceduralMeshVertex
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category=Triangle)
	FVector Position;

	UPROPERTY(EditAnywhere, Category=Triangle)
	FColor Color;

	UPROPERTY(EditAnywhere, Category=Triangle)
	float U;

	UPROPERTY(EditAnywhere, Category=Triangle)
	float V;
};

USTRUCT(BlueprintType)
struct FProceduralMeshTriangle
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category=Triangle)
	FProceduralMeshVertex Vertex0;

	UPROPERTY(EditAnywhere, Category=Triangle)
	FProceduralMeshVertex Vertex1;

	UPROPERTY(EditAnywhere, Category=Triangle)
	FProceduralMeshVertex Vertex2;
};

/** Component that allows you to specify custom triangle mesh geometry */
UCLASS(editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup=Rendering)
class UCustomProceduralMeshComponent : public UMeshComponent //, public IInterface_CollisionDataProvider
{
	GENERATED_UCLASS_BODY()

public:

	void ForceAnUpdate(int render_packet_idx=-1);

	/** Description of collision */
	UPROPERTY(BlueprintReadOnly, Category="Collision")
	class UBodySetup* ModelBodySetup;

	// Begin Interface_CollisionDataProvider Interface
	/*
	virtual bool GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData) override;
	virtual bool ContainsPhysicsTriMeshData(bool InUseAllTriData) const override;
	virtual bool WantsNegXTriMesh() override{ return false; }
	*/
	// End Interface_CollisionDataProvider Interface

	// Begin UPrimitiveComponent interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual class UBodySetup* GetBodySetup() override;
	// End UPrimitiveComponent interface.

	// Begin UMeshComponent interface.
	virtual int32 GetNumMaterials() const override;
	// End UMeshComponent interface.

	void UpdateBodySetup();
	void UpdateCollision();

	FSphere GetDebugBoundsSphere() const;

	void SetExtraXForm(const FTransform& xformIn);

	void SetBoundsScale(float value_in);

	void SetBoundsOffset(const FVector& offset_in);

	void SetTagString(FString tag_in);

	void RecreateRenderProxy(bool flag_in);

	bool SetProceduralMeshTriData(const FProceduralMeshTriData& TriData);


protected:
	FProceduralMeshTriData defaultTriData;
	FTransform extraXForm;
	FString tagStr;

	void ProcessCalcBounds();

	// Begin USceneComponent interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform & LocalToWorld) const override;

	// Begin USceneComponent interface.

	/** */

	friend class FProceduralMeshSceneProxy;
	float bounds_scale;
	FVector bounds_offset;
	mutable FSphere debugSphere;
	FVector calc_local_vec_min, calc_local_vec_max;
	FProceduralMeshSceneProxy * localRenderProxy;
	bool render_proxy_ready;
	std::mutex local_lock;
	bool recreate_render_proxy;
};
