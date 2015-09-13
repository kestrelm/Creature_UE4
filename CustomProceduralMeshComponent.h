// UE4 Procedural Mesh Generation from the Epic Wiki (https://wiki.unrealengine.com/Procedural_Mesh_Generation)
//
// forked from "Engine/Plugins/Runtime/CustomMeshComponent/Source/CustomMeshComponent/Classes/CustomMeshComponent.h"

#pragma once

#include <mutex>
#include "CustomProceduralMeshComponent.generated.h"

class UCustomProceduralMeshComponent;
class FProceduralMeshRenderPacket;

/** Scene proxy */
class FProceduralMeshSceneProxy : public FPrimitiveSceneProxy
{
public:

	FProceduralMeshSceneProxy(UCustomProceduralMeshComponent* Component,
		TArray<FProceduralMeshTriangle> * targetTrisIn);

	virtual ~FProceduralMeshSceneProxy();

	void AddRenderPacket(TArray<FProceduralMeshTriangle> * targetTrisIn);


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
	/** Set the geometry to use on this triangle mesh */
	UFUNCTION(BlueprintCallable, Category="Components|CustomProceduralMesh")
	bool SetProceduralMeshTriangles(const TArray<FProceduralMeshTriangle>& Triangles);


	/** Add to the geometry to use on this triangle mesh.  This may cause an allocation.  Use SetCustomMeshTriangles() instead when possible to reduce allocations. */
	UFUNCTION(BlueprintCallable, Category="Components|CustomProceduralMesh")
	void AddProceduralMeshTriangles(const TArray<FProceduralMeshTriangle>& Triangles);

	/** Removes all geometry from this triangle mesh.  Does not deallocate memory, allowing new geometry to reuse the existing allocation. */
	UFUNCTION(BlueprintCallable, Category="Components|CustomProceduralMesh")
	void ClearProceduralMeshTriangles();

	TArray<FProceduralMeshTriangle>& GetProceduralTriangles();

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

protected:
	FTransform extraXForm;
	FString tagStr;

	// Begin USceneComponent interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform & LocalToWorld) const override;

	// Begin USceneComponent interface.

	/** */
	TArray<FProceduralMeshTriangle> ProceduralMeshTris;

	friend class FProceduralMeshSceneProxy;
	float bounds_scale;
	FVector bounds_offset;
	mutable FSphere debugSphere;
	FProceduralMeshSceneProxy * localRenderProxy;
	bool render_proxy_ready;
	std::mutex local_lock;
	bool recreate_render_proxy;
};
