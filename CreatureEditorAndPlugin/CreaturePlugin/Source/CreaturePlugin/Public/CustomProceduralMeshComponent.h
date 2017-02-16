// UE4 Procedural Mesh Generation from the Epic Wiki (https://wiki.unrealengine.com/Procedural_Mesh_Generation)
//
// forked from "Engine/Plugins/Runtime/CustomMeshComponent/Source/CustomMeshComponent/Classes/CustomMeshComponent.h"

#pragma once

#include  <glm/glm.hpp>
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
		TSharedPtr<FCriticalSection, ESPMode::ThreadSafe> update_lock_in)
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
	TSharedPtr<FCriticalSection, ESPMode::ThreadSafe> update_lock;
};

/** Scene proxy */
class FCProceduralMeshSceneProxy : public FPrimitiveSceneProxy
{
public:

	FCProceduralMeshSceneProxy(
		UCustomProceduralMeshComponent* Component,
		FProceduralMeshTriData * targetTrisIn,
		const FColor& startColorIn);

	virtual ~FCProceduralMeshSceneProxy();

	void AddRenderPacket(FProceduralMeshTriData * targetTrisIn, const FColor& startColorIn);

	void ResetAllRenderPackets();
	
	void SetActiveRenderPacketIdx(int idxIn);

	void UpdateDynamicComponentData();

	void SetNeedsMaterialUpdate(bool flag_in);

	void SetNeedsIndexUpdate(bool flag_in);

	void UpdateMaterial();
	
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views,
		const FSceneViewFamily& ViewFamily,
		uint32 VisibilityMap,
		FMeshElementCollector& Collector) const override;


	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;

	virtual bool CanBeOccluded() const override;

	virtual uint32 GetMemoryFootprint(void) const;

	uint32 GetAllocatedSize(void) const;

	FProceduralMeshRenderPacket * GetActiveRenderPacket();
	bool GetDoesActiveRenderPacketHaveVertices() const;

	void SetDynamicData_RenderThread();

private:
	UCustomProceduralMeshComponent* parentComponent;
	UMaterialInterface* Material;
	TArray<FProceduralMeshRenderPacket> renderPackets;
	int active_render_packet_idx;

	FMaterialRelevance MaterialRelevance;
	bool needs_index_updating;
	bool needs_material_updating;

	mutable FCriticalSection renderPacketsCS;
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
class CREATUREPLUGIN_API UCustomProceduralMeshComponent : public UMeshComponent //, public IInterface_CollisionDataProvider
{
	GENERATED_UCLASS_BODY()

public:

	void ForceAnUpdate(int render_packet_idx=-1, bool markDirty = true);

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

	virtual void InitializeComponent();

	// Begin UMeshComponent interface.
	virtual int32 GetNumMaterials() const override;
	// End UMeshComponent interface.

	void UpdateBodySetup();
	void UpdateCollision();

	FSphere GetDebugBoundsSphere() const;

	void SetBoundsScale(float value_in);

	void SetBoundsOffset(const FVector& offset_in);

	void SetTagString(FString tag_in);

	void RecreateRenderProxy(bool flag_in);

	bool SetProceduralMeshTriData(const FProceduralMeshTriData& TriData);

	/** Called to send dynamic data for this component to the rendering thread */
	virtual void SendRenderDynamicData_Concurrent() override;

protected:
	FProceduralMeshTriData defaultTriData;
	FString tagStr;

	void ProcessCalcBounds(FCProceduralMeshSceneProxy *localRenderProxy);

	// Begin USceneComponent interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform & LocalToWorld) const override;

	// Begin USceneComponent interface.

	/** */

	friend class FCProceduralMeshSceneProxy;
	float bounds_scale;
	FVector bounds_offset;
	mutable FSphere debugSphere;
	FVector calc_local_vec_min, calc_local_vec_max;
	FCProceduralMeshSceneProxy * GetLocalRenderProxy()
	{
		return (FCProceduralMeshSceneProxy*)SceneProxy;
	}
	bool render_proxy_ready;
	FCriticalSection local_lock;
	bool recreate_render_proxy;
};
