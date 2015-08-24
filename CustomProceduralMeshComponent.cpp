// UE4 Procedural Mesh Generation from the Epic Wiki (https://wiki.unrealengine.com/Procedural_Mesh_Generation)
//
// forked from "Engine/Plugins/Runtime/CustomMeshComponent/Source/CustomMeshComponent/Private/CustomMeshComponent.cpp"

#include "CustomProceduralMesh.h"
#include "DynamicMeshBuilder.h"
#include "CustomProceduralMeshComponent.h"
#include "Runtime/Launch/Resources/Version.h"

/** Vertex Buffer */
class FProceduralMeshVertexBuffer : public FVertexBuffer
{
public:
	TArray<FDynamicMeshVertex> Vertices;

	virtual void InitRHI() override
	{
		FRHIResourceCreateInfo CreateInfo;
		VertexBufferRHI = RHICreateVertexBuffer(Vertices.Num() * sizeof(FDynamicMeshVertex), BUF_Static, CreateInfo);
		UpdateRenderData();
	}

	void UpdateRenderData() const
	{
		// Copy the vertex data into the vertex buffer.
		void* VertexBufferData = RHILockVertexBuffer(VertexBufferRHI, 0, Vertices.Num() * sizeof(FDynamicMeshVertex), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, Vertices.GetData(), Vertices.Num() * sizeof(FDynamicMeshVertex));
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
};

/** Index Buffer */
class FProceduralMeshIndexBuffer : public FIndexBuffer
{
public:
	TArray<int32> Indices;

	virtual void InitRHI() override
	{
		FRHIResourceCreateInfo CreateInfo;
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(int32), Indices.Num() * sizeof(int32), BUF_Static, CreateInfo);
		// Write the indices to the index buffer.
		void* Buffer = RHILockIndexBuffer(IndexBufferRHI, 0, Indices.Num() * sizeof(int32), RLM_WriteOnly);
		FMemory::Memcpy(Buffer, Indices.GetData(), Indices.Num() * sizeof(int32));
		RHIUnlockIndexBuffer(IndexBufferRHI);
	}
};

/** Vertex Factory */
class FProceduralMeshVertexFactory : public FLocalVertexFactory
{
public:
	FProceduralMeshVertexFactory()
	{
	}

	/** Initialization */
	void Init(const FProceduralMeshVertexBuffer* VertexBuffer)
	{
		// Commented out to enable building light of a level (but no backing is done for the procedural mesh itself)
		//check(!IsInRenderingThread());

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			InitProceduralMeshVertexFactory,
			FProceduralMeshVertexFactory*, VertexFactory, this,
			const FProceduralMeshVertexBuffer*, VertexBuffer, VertexBuffer,
		{
			// Initialize the vertex factory's stream components.
			DataType NewData;
			NewData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer,FDynamicMeshVertex,Position,VET_Float3);
			NewData.TextureCoordinates.Add(
				FVertexStreamComponent(VertexBuffer,STRUCT_OFFSET(FDynamicMeshVertex,TextureCoordinate),sizeof(FDynamicMeshVertex),VET_Float2)
				);
			NewData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer,FDynamicMeshVertex,TangentX,VET_PackedNormal);
			NewData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer,FDynamicMeshVertex,TangentZ,VET_PackedNormal);
			NewData.ColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Color, VET_Color);
			VertexFactory->SetData(NewData);
		});
	}
};



/** Scene proxy */
class FProceduralMeshSceneProxy : public FPrimitiveSceneProxy
{
public:

	FProceduralMeshSceneProxy(UCustomProceduralMeshComponent* Component)
		: FPrimitiveSceneProxy(Component)
		, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
	{
		parentComponent = Component;
		needs_updating = false;

		// Add each triangle to the vertex/index buffer
		for(int TriIdx = 0; TriIdx<Component->ProceduralMeshTris.Num(); TriIdx++)
		{
			FProceduralMeshTriangle& Tri = Component->ProceduralMeshTris[TriIdx];

			const FVector Edge01 = (Tri.Vertex1.Position - Tri.Vertex0.Position);
			const FVector Edge02 = (Tri.Vertex2.Position - Tri.Vertex0.Position);

			const FVector TangentX = Edge01.GetSafeNormal();
			const FVector TangentZ = (Edge02 ^ Edge01).GetSafeNormal();
			const FVector TangentY = (TangentX ^ TangentZ).GetSafeNormal();

			FDynamicMeshVertex Vert0;
			Vert0.Position = Tri.Vertex0.Position;
			Vert0.Color = Tri.Vertex0.Color;
			Vert0.SetTangents(TangentX, TangentY, TangentZ);
			Vert0.TextureCoordinate.Set(Tri.Vertex0.U, Tri.Vertex0.V);
			int32 VIndex = VertexBuffer.Vertices.Add(Vert0);
			IndexBuffer.Indices.Add(VIndex);

			FDynamicMeshVertex Vert1;
			Vert1.Position = Tri.Vertex1.Position;
			Vert1.Color = Tri.Vertex1.Color;
			Vert1.SetTangents(TangentX, TangentY, TangentZ);
			Vert1.TextureCoordinate.Set(Tri.Vertex1.U, Tri.Vertex1.V);
			VIndex = VertexBuffer.Vertices.Add(Vert1);
			IndexBuffer.Indices.Add(VIndex);

			FDynamicMeshVertex Vert2;
			Vert2.Position = Tri.Vertex2.Position;
			Vert2.Color = Tri.Vertex2.Color;
			Vert2.SetTangents(TangentX, TangentY, TangentZ);
			Vert2.TextureCoordinate.Set(Tri.Vertex2.U, Tri.Vertex2.V);
			VIndex = VertexBuffer.Vertices.Add(Vert2);
			IndexBuffer.Indices.Add(VIndex);
		}

		// Init vertex factory
		VertexFactory.Init(&VertexBuffer);

		// Enqueue initialization of render resource
		BeginInitResource(&VertexBuffer);
		BeginInitResource(&IndexBuffer);
		BeginInitResource(&VertexFactory);

		// Grab material
		Material = Component->GetMaterial(0);
		if(Material == NULL)
		{
			Material = UMaterial::GetDefaultMaterial(MD_Surface);
		}
	}

	virtual ~FProceduralMeshSceneProxy()
	{
		VertexBuffer.ReleaseResource();
		IndexBuffer.ReleaseResource();
		VertexFactory.ReleaseResource();
	}

	void UpdateDynamicComponentData()
	{
		if (VertexBuffer.Vertices.Num() != parentComponent->ProceduralMeshTris.Num() * 3)
		{
			return;
		}

		int cnter = 0;
		FDynamicMeshVertex FillVert;

		for (int TriIdx = 0; TriIdx<parentComponent->ProceduralMeshTris.Num(); TriIdx++)
		{
			FProceduralMeshTriangle& Tri = parentComponent->ProceduralMeshTris[TriIdx];

			// Fill in data
			const FVector Edge01 = (Tri.Vertex1.Position - Tri.Vertex0.Position);
			const FVector Edge02 = (Tri.Vertex2.Position - Tri.Vertex0.Position);

			const FVector TangentX = Edge01.GetSafeNormal();
			const FVector TangentZ = (Edge02 ^ Edge01).GetSafeNormal();
			const FVector TangentY = (TangentX ^ TangentZ).GetSafeNormal();

			//FillVert.Position = Tri.Vertex0.Position;
			//FillVert.Color = Tri.Vertex0.Color;
			//FillVert.SetTangents(TangentX, TangentY, TangentZ);
			//FillVert.TextureCoordinate.Set(Tri.Vertex0.U, Tri.Vertex0.V);

			//VertexBuffer.Vertices[cnter] = FillVert;
			VertexBuffer.Vertices[cnter].Position = Tri.Vertex0.Position;
			VertexBuffer.Vertices[cnter].Color = Tri.Vertex0.Color;
			VertexBuffer.Vertices[cnter].SetTangents(TangentX, TangentY, TangentZ);
			VertexBuffer.Vertices[cnter].TextureCoordinate.Set(Tri.Vertex0.U, Tri.Vertex0.V);

			cnter++;

			//FillVert.Position = Tri.Vertex1.Position;
			//FillVert.Color = Tri.Vertex1.Color;
			//FillVert.SetTangents(TangentX, TangentY, TangentZ);
			//FillVert.TextureCoordinate.Set(Tri.Vertex1.U, Tri.Vertex1.V);

			//VertexBuffer.Vertices[cnter] = FillVert;
			VertexBuffer.Vertices[cnter].Position = Tri.Vertex1.Position;
			VertexBuffer.Vertices[cnter].Color = Tri.Vertex1.Color;
			VertexBuffer.Vertices[cnter].SetTangents(TangentX, TangentY, TangentZ);
			VertexBuffer.Vertices[cnter].TextureCoordinate.Set(Tri.Vertex1.U, Tri.Vertex1.V);

			cnter++;

			//FillVert.Position = Tri.Vertex2.Position;
			//FillVert.Color = Tri.Vertex2.Color;
			//FillVert.SetTangents(TangentX, TangentY, TangentZ);
			//FillVert.TextureCoordinate.Set(Tri.Vertex2.U, Tri.Vertex2.V);

			//VertexBuffer.Vertices[cnter] = FillVert;
			VertexBuffer.Vertices[cnter].Position = Tri.Vertex2.Position;
			VertexBuffer.Vertices[cnter].Color = Tri.Vertex2.Color;
			VertexBuffer.Vertices[cnter].SetTangents(TangentX, TangentY, TangentZ);
			VertexBuffer.Vertices[cnter].TextureCoordinate.Set(Tri.Vertex2.U, Tri.Vertex2.V);

			cnter++;
		}

		needs_updating = true;
	}

	void DoneUpdating()
	{
		needs_updating = false;
	}

    virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views,
		const FSceneViewFamily& ViewFamily,
		uint32 VisibilityMap,
		FMeshElementCollector& Collector) const override
	{
		if (needs_updating) {
			VertexBuffer.UpdateRenderData();
			(const_cast<FProceduralMeshSceneProxy*>(this))->DoneUpdating();
		}

		QUICK_SCOPE_CYCLE_COUNTER( STAT_ProceduralMeshSceneProxy_GetDynamicMeshElements );
		
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy(IsSelected()) : NULL,
			FLinearColor(0, 0.5f, 1.f)
			);

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

		FMaterialRenderProxy* MaterialProxy = NULL;
		if(bWireframe)
		{
			MaterialProxy = WireframeMaterialInstance;
		}
		else
		{
			MaterialProxy = Material->GetRenderProxy(IsSelected());
		}

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];
				// Draw the mesh.

				FMeshBatch& Mesh = Collector.AllocateMesh();
				FMeshBatchElement& BatchElement = Mesh.Elements[0];
				BatchElement.IndexBuffer = &IndexBuffer;
				Mesh.bWireframe = bWireframe;
				Mesh.VertexFactory = &VertexFactory;
				Mesh.MaterialRenderProxy = MaterialProxy;
				BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = IndexBuffer.Indices.Num() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = VertexBuffer.Vertices.Num() - 1;
				Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
				Mesh.Type = PT_TriangleList;
				Mesh.DepthPriorityGroup = SDPG_World;
				Mesh.bCanApplyViewModeOverrides = false;
				Collector.AddMesh(ViewIndex, Mesh);
			}
		}
	}

	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI, const FSceneView* View)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_ProceduralMeshSceneProxy_DrawDynamicElements);

		const bool bWireframe = AllowDebugViewmodes() && View->Family->EngineShowFlags.Wireframe;

		FColoredMaterialRenderProxy WireframeMaterialInstance(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy(IsSelected()) : NULL,
			FLinearColor(0, 0.5f, 1.f)
			);

		FMaterialRenderProxy* MaterialProxy = NULL;
		if(bWireframe)
		{
			MaterialProxy = &WireframeMaterialInstance;
		}
		else
		{
			MaterialProxy = Material->GetRenderProxy(IsSelected());
		}

		// Draw the mesh.
		FMeshBatch Mesh;
		FMeshBatchElement& BatchElement = Mesh.Elements[0];
		BatchElement.IndexBuffer = &IndexBuffer;
		Mesh.bWireframe = bWireframe;
		Mesh.VertexFactory = &VertexFactory;
		Mesh.MaterialRenderProxy = MaterialProxy;

		FBoxSphereBounds WorldBounds = GetBounds();
		FBoxSphereBounds LocalBounds = GetLocalBounds();

		BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), WorldBounds, LocalBounds, true, UseEditorDepthTest());
		BatchElement.FirstIndex = 0;
		BatchElement.NumPrimitives = IndexBuffer.Indices.Num() / 3;
		BatchElement.MinVertexIndex = 0;
		BatchElement.MaxVertexIndex = VertexBuffer.Vertices.Num() - 1;
		Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
		Mesh.Type = PT_TriangleList;
		Mesh.DepthPriorityGroup = SDPG_World;
		PDI->DrawMesh(Mesh);
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = true; // IsShown(View);
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bDynamicRelevance = true;
		MaterialRelevance.SetPrimitiveViewRelevance(Result);
		return Result;
	}

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

		virtual uint32 GetMemoryFootprint(void) const
	{
		return(sizeof(*this) + GetAllocatedSize());
	}

	uint32 GetAllocatedSize(void) const
	{
		return(FPrimitiveSceneProxy::GetAllocatedSize());
	}

private:
	UCustomProceduralMeshComponent* parentComponent;
	UMaterialInterface* Material;
	FProceduralMeshVertexBuffer VertexBuffer;
	FProceduralMeshIndexBuffer IndexBuffer;
	FProceduralMeshVertexFactory VertexFactory;

	FMaterialRelevance MaterialRelevance;
	bool needs_updating;
};


//////////////////////////////////////////////////////////////////////////

UCustomProceduralMeshComponent::UCustomProceduralMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	bounds_scale = 15.0f;
	bounds_offset = FVector(0, 0, 0);
	localRenderProxy = NULL;
	render_proxy_ready = false;

//	SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
}

bool UCustomProceduralMeshComponent::SetProceduralMeshTriangles(const TArray<FProceduralMeshTriangle>& Triangles)
{
	ProceduralMeshTris = Triangles;

	//UpdateCollision();

	// Need to recreate scene proxy to send it over
	MarkRenderStateDirty();

	return true;
}

void UCustomProceduralMeshComponent::RecreateRenderProxy(bool flag_in)
{
	recreate_render_proxy = flag_in;
}

void UCustomProceduralMeshComponent::ForceAnUpdate()
{
	// Need to recreate scene proxy to send it over
	if (recreate_render_proxy)
	{
		MarkRenderStateDirty();
		recreate_render_proxy = false;
		return;
	}

	std::lock_guard<std::mutex> cur_lock(local_lock);
	if (render_proxy_ready && localRenderProxy) {
		localRenderProxy->UpdateDynamicComponentData();
	}
}

void 
UCustomProceduralMeshComponent::SetTagString(FString tag_in)
{
	tagStr = tag_in;
}

TArray<FProceduralMeshTriangle>& 
UCustomProceduralMeshComponent::GetProceduralTriangles()
{
	return ProceduralMeshTris;
}

void UCustomProceduralMeshComponent::AddProceduralMeshTriangles(const TArray<FProceduralMeshTriangle>& Triangles)
{
	ProceduralMeshTris.Append(Triangles);

	// Need to recreate scene proxy to send it over
	MarkRenderStateDirty();
}

void  UCustomProceduralMeshComponent::ClearProceduralMeshTriangles()
{
	ProceduralMeshTris.Reset();

	// Need to recreate scene proxy to send it over
	MarkRenderStateDirty();
}

FPrimitiveSceneProxy* UCustomProceduralMeshComponent::CreateSceneProxy()
{
	std::lock_guard<std::mutex> cur_lock(local_lock);

	FPrimitiveSceneProxy* Proxy = NULL;
	// Only if have enough triangles
	if(ProceduralMeshTris.Num() > 0)
	{
		localRenderProxy = new FProceduralMeshSceneProxy(this);
		Proxy = localRenderProxy;
		render_proxy_ready = true;
	}

	return Proxy;
}

int32 UCustomProceduralMeshComponent::GetNumMaterials() const
{
	return 1;
}

FBoxSphereBounds UCustomProceduralMeshComponent::CalcBounds(const FTransform & LocalToWorld) const
{
	// Only if have enough triangles
	if(ProceduralMeshTris.Num() > 0)
	{
		// Minimum Vector: It's set to the first vertex's position initially (NULL == FVector::ZeroVector might be required and a known vertex vector has intrinsically valid values)
		FVector vecMin = ProceduralMeshTris[0].Vertex0.Position;

		// Maximum Vector: It's set to the first vertex's position initially (NULL == FVector::ZeroVector might be required and a known vertex vector has intrinsically valid values)
		FVector vecMax = ProceduralMeshTris[0].Vertex0.Position;

		// Get maximum and minimum X, Y and Z positions of vectors
		FVector vecMidPt(0, 0, 0);
		for(int32 TriIdx = 0; TriIdx < ProceduralMeshTris.Num(); TriIdx++)
		{
			vecMin.X = (vecMin.X > ProceduralMeshTris[TriIdx].Vertex0.Position.X) ? ProceduralMeshTris[TriIdx].Vertex0.Position.X : vecMin.X;
			vecMin.X = (vecMin.X > ProceduralMeshTris[TriIdx].Vertex1.Position.X) ? ProceduralMeshTris[TriIdx].Vertex1.Position.X : vecMin.X;
			vecMin.X = (vecMin.X > ProceduralMeshTris[TriIdx].Vertex2.Position.X) ? ProceduralMeshTris[TriIdx].Vertex2.Position.X : vecMin.X;

			vecMin.Y = (vecMin.Y > ProceduralMeshTris[TriIdx].Vertex0.Position.Y) ? ProceduralMeshTris[TriIdx].Vertex0.Position.Y : vecMin.Y;
			vecMin.Y = (vecMin.Y > ProceduralMeshTris[TriIdx].Vertex1.Position.Y) ? ProceduralMeshTris[TriIdx].Vertex1.Position.Y : vecMin.Y;
			vecMin.Y = (vecMin.Y > ProceduralMeshTris[TriIdx].Vertex2.Position.Y) ? ProceduralMeshTris[TriIdx].Vertex2.Position.Y : vecMin.Y;

			vecMin.Z = (vecMin.Z > ProceduralMeshTris[TriIdx].Vertex0.Position.Z) ? ProceduralMeshTris[TriIdx].Vertex0.Position.Z : vecMin.Z;
			vecMin.Z = (vecMin.Z > ProceduralMeshTris[TriIdx].Vertex1.Position.Z) ? ProceduralMeshTris[TriIdx].Vertex1.Position.Z : vecMin.Z;
			vecMin.Z = (vecMin.Z > ProceduralMeshTris[TriIdx].Vertex2.Position.Z) ? ProceduralMeshTris[TriIdx].Vertex2.Position.Z : vecMin.Z;

			vecMax.X = (vecMax.X < ProceduralMeshTris[TriIdx].Vertex0.Position.X) ? ProceduralMeshTris[TriIdx].Vertex0.Position.X : vecMax.X;
			vecMax.X = (vecMax.X < ProceduralMeshTris[TriIdx].Vertex1.Position.X) ? ProceduralMeshTris[TriIdx].Vertex1.Position.X : vecMax.X;
			vecMax.X = (vecMax.X < ProceduralMeshTris[TriIdx].Vertex2.Position.X) ? ProceduralMeshTris[TriIdx].Vertex2.Position.X : vecMax.X;

			vecMax.Y = (vecMax.Y < ProceduralMeshTris[TriIdx].Vertex0.Position.Y) ? ProceduralMeshTris[TriIdx].Vertex0.Position.Y : vecMax.Y;
			vecMax.Y = (vecMax.Y < ProceduralMeshTris[TriIdx].Vertex1.Position.Y) ? ProceduralMeshTris[TriIdx].Vertex1.Position.Y : vecMax.Y;
			vecMax.Y = (vecMax.Y < ProceduralMeshTris[TriIdx].Vertex2.Position.Y) ? ProceduralMeshTris[TriIdx].Vertex2.Position.Y : vecMax.Y;

			vecMax.Z = (vecMax.Z < ProceduralMeshTris[TriIdx].Vertex0.Position.Z) ? ProceduralMeshTris[TriIdx].Vertex0.Position.Z : vecMax.Z;
			vecMax.Z = (vecMax.Z < ProceduralMeshTris[TriIdx].Vertex1.Position.Z) ? ProceduralMeshTris[TriIdx].Vertex1.Position.Z : vecMax.Z;
			vecMax.Z = (vecMax.Z < ProceduralMeshTris[TriIdx].Vertex2.Position.Z) ? ProceduralMeshTris[TriIdx].Vertex2.Position.Z : vecMax.Z;

			vecMidPt += ProceduralMeshTris[TriIdx].Vertex0.Position;
			vecMidPt += ProceduralMeshTris[TriIdx].Vertex1.Position;
			vecMidPt += ProceduralMeshTris[TriIdx].Vertex2.Position;
		}
		
		const float lscale = bounds_scale;
		FVector lScaleVec(lscale, lscale, lscale);
		vecMidPt = (vecMax + vecMin) * 0.5f;
		vecMax = (vecMax - vecMidPt) * lScaleVec + vecMidPt;
		vecMin = (vecMin - vecMidPt) * lScaleVec + vecMidPt;

		FTransform curXForm = extraXForm;

		vecMin = curXForm.TransformPosition(vecMin);
		vecMax = curXForm.TransformPosition(vecMax);

		FBox curBox(vecMin, vecMax);
		FBoxSphereBounds retBounds(curBox);
		//retBounds.Origin.Y = -retBounds.SphereRadius;
		retBounds.Origin += bounds_offset;

		// Debugging
		FSphere sphereBounds = retBounds.GetSphere();
		debugSphere = sphereBounds;

		return retBounds;
	}
	else
	{
		return FBoxSphereBounds();
	}
}

void UCustomProceduralMeshComponent::SetBoundsScale(float value_in)
{
	bounds_scale = value_in;
}

void UCustomProceduralMeshComponent::SetBoundsOffset(const FVector& offset_in)
{
	bounds_offset = offset_in;
}

void 
UCustomProceduralMeshComponent::SetExtraXForm(const FTransform& xformIn)
{
	extraXForm = xformIn;
}

FSphere 
UCustomProceduralMeshComponent::GetDebugBoundsSphere() const
{
	return debugSphere;
}

/*
bool UCustomProceduralMeshComponent::GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
{
	FTriIndices Triangle;

	for(int32 i = 0; i<ProceduralMeshTris.Num(); i++)
	{
		const FProceduralMeshTriangle& tri = ProceduralMeshTris[i];

		Triangle.v0 = CollisionData->Vertices.Add(tri.Vertex0.Position);
		Triangle.v1 = CollisionData->Vertices.Add(tri.Vertex1.Position);
		Triangle.v2 = CollisionData->Vertices.Add(tri.Vertex2.Position);

		CollisionData->Indices.Add(Triangle);
		CollisionData->MaterialIndices.Add(i);
	}

	CollisionData->bFlipNormals = true;

	return true;
}

bool UCustomProceduralMeshComponent::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
{
	return (ProceduralMeshTris.Num() > 0);
}
*/

void UCustomProceduralMeshComponent::UpdateBodySetup()
{
	if(ModelBodySetup == NULL)
	{
		/*
		ModelBodySetup = ConstructObject<UBodySetup>(UBodySetup::StaticClass(), this);
		ModelBodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
		ModelBodySetup->bMeshCollideAll = true;
		*/
	}
}

void UCustomProceduralMeshComponent::UpdateCollision()
{
	/*
	if(bPhysicsStateCreated)
	{
		DestroyPhysicsState();
		UpdateBodySetup();
		CreatePhysicsState();

		// Works in Packaged build only since UE4.5:
		ModelBodySetup->InvalidatePhysicsData();
		ModelBodySetup->CreatePhysicsMeshes();
	}
	*/
}

UBodySetup* UCustomProceduralMeshComponent::GetBodySetup()
{
	UpdateBodySetup();
	return ModelBodySetup;
}
