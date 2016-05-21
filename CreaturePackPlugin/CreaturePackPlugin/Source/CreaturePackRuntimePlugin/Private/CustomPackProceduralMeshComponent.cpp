// UE4 Procedural Mesh Generation from the Epic Wiki (https://wiki.unrealengine.com/Procedural_Mesh_Generation)
//
// forked from "Engine/Plugins/Runtime/CustomMeshComponent/Source/CustomMeshComponent/Private/CustomMeshComponent.cpp"

#include "CreaturePackRuntimePluginPCH.h"
#include "DynamicMeshBuilder.h"
#include "CustomPackProceduralMeshComponent.h"
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
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(int32), Indices.Num() * sizeof(int32), BUF_Dynamic, CreateInfo);
		UpdateRenderData();
	}

	void UpdateRenderData() const
	{
		// Copy the index data into the indices buffer
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

/** Mesh Render Packet**/
class FProceduralPackMeshRenderPacket
{
public:
	FProceduralPackMeshRenderPacket(FProceduralPackMeshTriData * data_in)
	{
		indices = data_in->indices;
		points = data_in->points;
		uvs = data_in->uvs;
		point_num = data_in->point_num;
		indices_num = data_in->indices_num;
		region_alphas = data_in->region_alphas;
		update_lock = data_in->update_lock;
		should_release = false;
	}

	virtual ~FProceduralPackMeshRenderPacket()
	{
		if (should_release) {
			VertexBuffer.ReleaseResource();
			IndexBuffer.ReleaseResource();
			VertexFactory.ReleaseResource();
		}
	}

	void InitForRender()
	{
		BeginInitResource(&VertexBuffer);
		BeginInitResource(&IndexBuffer);
		BeginInitResource(&VertexFactory);

		should_release = true;
	}

	void UpdateDirectVertexData() const
	{
		const int x_id = 0;
		const int y_id = 2;
		const int z_id = 1;

		std::lock_guard<std::mutex> scope_lock(*update_lock);

		FDynamicMeshVertex* VertexBufferData = (FDynamicMeshVertex *)RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0, this->point_num * sizeof(FDynamicMeshVertex), RLM_WriteOnly);

		for (int32 i = 0; i < this->point_num; i++)
		{
			FDynamicMeshVertex* curVert = VertexBufferData + i;

			int pos_idx = i * 3;
			curVert->Position = FVector(this->points[pos_idx + x_id],
				this->points[pos_idx + y_id],
				this->points[pos_idx + z_id]);

			float set_alpha = (*this->region_alphas)[i];
			curVert->Color = FColor(set_alpha, set_alpha, set_alpha, set_alpha);

			int uv_idx = i * 2;
			curVert->TextureCoordinate.Set(this->uvs[uv_idx], this->uvs[uv_idx + 1]);
		}

		// Set Tangents
		for (int cur_indice = 0; cur_indice < indices_num; cur_indice += 3)
		{
			FDynamicMeshVertex * vert0 = VertexBufferData + (indices[cur_indice]);
			FDynamicMeshVertex * vert1 = VertexBufferData + (indices[cur_indice + 1]);
			FDynamicMeshVertex * vert2 = VertexBufferData + (indices[cur_indice + 2]);

			const FVector Edge01 = (vert1->Position - vert0->Position);
			const FVector Edge02 = (vert2->Position - vert0->Position);

			const FVector TangentX = Edge01.GetSafeNormal();
			const FVector TangentZ = (Edge02 ^ Edge01).GetSafeNormal();
			const FVector TangentY = (TangentX ^ TangentZ).GetSafeNormal();

			vert0->SetTangents(TangentX, TangentY, TangentZ);
			vert1->SetTangents(TangentX, TangentY, TangentZ);
			vert2->SetTangents(TangentX, TangentY, TangentZ);
		}

		RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);
	}

	void UpdateDirectIndexData() const
	{
		std::lock_guard<std::mutex> scope_lock(*update_lock);
		void* Buffer = RHILockIndexBuffer(IndexBuffer.IndexBufferRHI, 0, indices_num * sizeof(int32), RLM_WriteOnly);

		FMemory::Memcpy(Buffer, indices, indices_num * sizeof(int32));

		RHIUnlockIndexBuffer(IndexBuffer.IndexBufferRHI);
	}

	FProceduralMeshVertexBuffer VertexBuffer;
	FProceduralMeshIndexBuffer IndexBuffer;
	FProceduralMeshVertexFactory VertexFactory;
	uint32 * indices;
	float * points;
	float * uvs;
	int32 point_num, indices_num;
	TArray<uint8> * region_alphas;
	std::mutex * update_lock;
	bool should_release;
};

/** Scene proxy */

FCProceduralPackMeshSceneProxy::FCProceduralPackMeshSceneProxy(UCustomPackProceduralMeshComponent* Component,
	FProceduralPackMeshTriData * targetTrisIn)
	: FPrimitiveSceneProxy(Component),
	MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
{
	parentComponent = Component;
	needs_updating = false;
	needs_index_updating = false;
	active_render_packet_idx = INDEX_NONE;

	UpdateMaterial();

	// Add each triangle to the vertex/index buffer
	if (targetTrisIn)
	{
		AddRenderPacket(targetTrisIn);
	}
}

FCProceduralPackMeshSceneProxy::~FCProceduralPackMeshSceneProxy()
{
}

FProceduralPackMeshRenderPacket * 
FCProceduralPackMeshSceneProxy::GetActiveRenderPacket()
{
	if (!renderPackets.IsValidIndex(active_render_packet_idx))
	{
		return nullptr;
	}

	return &renderPackets[active_render_packet_idx];
}

void FCProceduralPackMeshSceneProxy::UpdateMaterial()
{
	// Grab material
	Material = parentComponent->GetMaterial(0);
	if (Material == NULL)
	{
		Material = UMaterial::GetDefaultMaterial(MD_Surface);
	}
}

void FCProceduralPackMeshSceneProxy::AddRenderPacket(FProceduralPackMeshTriData * targetTrisIn)
{
	FProceduralPackMeshRenderPacket new_packet(targetTrisIn);
	renderPackets.Add(new_packet);

	FProceduralPackMeshRenderPacket& cur_packet = renderPackets[renderPackets.Num() - 1];

	auto& IndexBuffer = cur_packet.IndexBuffer;
	auto& VertexBuffer = cur_packet.VertexBuffer;
	auto& VertexFactory = cur_packet.VertexFactory;

	IndexBuffer.Indices.SetNum(cur_packet.indices_num);
	VertexBuffer.Vertices.SetNum(cur_packet.point_num);

	// Set topology/indices
	for (int32 i = 0; i < cur_packet.indices_num; i++)
	{
		IndexBuffer.Indices[i] = cur_packet.indices[i];
	}

	const int x_id = 0;
	const int y_id = 2;
	const int z_id = 1;

	// Fill initial points
	for (int32 i = 0; i < cur_packet.point_num; i++)
	{
		FDynamicMeshVertex Vert0;
		int pos_idx = i * 3;
		Vert0.Position = FVector(cur_packet.points[pos_idx + x_id], cur_packet.points[pos_idx + y_id], cur_packet.points[pos_idx + z_id]);

		float set_alpha = (*cur_packet.region_alphas)[i];
		Vert0.Color = FColor(set_alpha, set_alpha, set_alpha, set_alpha);
		Vert0.SetTangents(FVector(1, 0, 0), FVector(0, 1, 0), FVector(0, 0, 1));

		int uv_idx = i * 2;
		Vert0.TextureCoordinate.Set(cur_packet.uvs[uv_idx], cur_packet.uvs[uv_idx + 1]);
		VertexBuffer.Vertices[i] = Vert0;
	}

	// Set Initial Rest Tangents
	for (int cur_indice = 0; cur_indice < cur_packet.indices_num; cur_indice += 3)
	{
		FDynamicMeshVertex& vert0 = VertexBuffer.Vertices[cur_packet.indices[cur_indice]];
		FDynamicMeshVertex& vert1 = VertexBuffer.Vertices[cur_packet.indices[cur_indice + 1]];
		FDynamicMeshVertex& vert2 = VertexBuffer.Vertices[cur_packet.indices[cur_indice + 2]];

		const FVector Edge01 = (vert1.Position - vert0.Position);
		const FVector Edge02 = (vert2.Position - vert0.Position);

		const FVector TangentX = Edge01.GetSafeNormal();
		const FVector TangentZ = (Edge02 ^ Edge01).GetSafeNormal();
		const FVector TangentY = (TangentX ^ TangentZ).GetSafeNormal();

		vert0.SetTangents(TangentX, TangentY, TangentZ);
		vert1.SetTangents(TangentX, TangentY, TangentZ);
		vert2.SetTangents(TangentX, TangentY, TangentZ);
	}

	// Init vertex factory
	VertexFactory.Init(&VertexBuffer);

	// Enqueue initialization of render resource
	cur_packet.InitForRender();

	if (active_render_packet_idx == INDEX_NONE)
	{
		active_render_packet_idx = 0;
	}
}

void FCProceduralPackMeshSceneProxy::ResetAllRenderPackets()
{
	renderPackets.Reset();
	active_render_packet_idx = INDEX_NONE;
}

void FCProceduralPackMeshSceneProxy::SetActiveRenderPacketIdx(int idxIn)
{
	active_render_packet_idx = idxIn;
}

void FCProceduralPackMeshSceneProxy::UpdateDynamicComponentData()
{
	if (active_render_packet_idx < 0)
	{
		return;
	}

	if (needs_material_updating)
	{
		UpdateMaterial();
	}

	/*
	auto& cur_packet = renderPackets[active_render_packet_idx];
	auto& VertexBuffer = cur_packet.VertexBuffer;
	auto& targetTris = cur_packet.GetTargetTris();

	if (VertexBuffer.Vertices.Num() != targetTris.Num() * 3)
	{
		return;
	}

	int cnter = 0;
	FDynamicMeshVertex FillVert;

	for (int TriIdx = 0; TriIdx<targetTris.Num(); TriIdx++)
	{
		FProceduralMeshTriangle& Tri = targetTris[TriIdx];

		// Fill in data
		const FVector Edge01 = (Tri.Vertex1.Position - Tri.Vertex0.Position);
		const FVector Edge02 = (Tri.Vertex2.Position - Tri.Vertex0.Position);

		const FVector TangentX = Edge01.GetSafeNormal();
		const FVector TangentZ = (Edge02 ^ Edge01).GetSafeNormal();
		const FVector TangentY = (TangentX ^ TangentZ).GetSafeNormal();

		VertexBuffer.Vertices[cnter].Position = Tri.Vertex0.Position;
		VertexBuffer.Vertices[cnter].Color = Tri.Vertex0.Color;
		VertexBuffer.Vertices[cnter].SetTangents(TangentX, TangentY, TangentZ);
		VertexBuffer.Vertices[cnter].TextureCoordinate.Set(Tri.Vertex0.U, Tri.Vertex0.V);

		cnter++;

		VertexBuffer.Vertices[cnter].Position = Tri.Vertex1.Position;
		VertexBuffer.Vertices[cnter].Color = Tri.Vertex1.Color;
		VertexBuffer.Vertices[cnter].SetTangents(TangentX, TangentY, TangentZ);
		VertexBuffer.Vertices[cnter].TextureCoordinate.Set(Tri.Vertex1.U, Tri.Vertex1.V);

		cnter++;

		VertexBuffer.Vertices[cnter].Position = Tri.Vertex2.Position;
		VertexBuffer.Vertices[cnter].Color = Tri.Vertex2.Color;
		VertexBuffer.Vertices[cnter].SetTangents(TangentX, TangentY, TangentZ);
		VertexBuffer.Vertices[cnter].TextureCoordinate.Set(Tri.Vertex2.U, Tri.Vertex2.V);

		cnter++;
	}
	*/

	needs_updating = true;
}

void FCProceduralPackMeshSceneProxy::DoneUpdating()
{
	needs_updating = false;
	needs_index_updating = false;
	needs_material_updating = false;
}

void FCProceduralPackMeshSceneProxy::SetNeedsMaterialUpdate(bool flag_in)
{
	needs_material_updating = flag_in;
}

void FCProceduralPackMeshSceneProxy::SetNeedsIndexUpdate(bool flag_in)
{
	needs_index_updating = flag_in;
}

void FCProceduralPackMeshSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views,
	const FSceneViewFamily& ViewFamily,
	uint32 VisibilityMap,
	FMeshElementCollector& Collector) const
{
	if (active_render_packet_idx < 0)
	{
		return;
	}

	auto& cur_packet = renderPackets[active_render_packet_idx];
	auto& VertexBuffer = cur_packet.VertexBuffer;
	auto& IndexBuffer = cur_packet.IndexBuffer;
	auto& VertexFactory = cur_packet.VertexFactory;

	if (cur_packet.point_num <= 0)
	{
		return;
	}

	if (needs_updating) {
		cur_packet.UpdateDirectVertexData();
		if (needs_index_updating) {
			cur_packet.UpdateDirectIndexData();
		}
	}

	(const_cast<FCProceduralPackMeshSceneProxy*>(this))->DoneUpdating();

	QUICK_SCOPE_CYCLE_COUNTER(STAT_ProceduralMeshSceneProxy_GetDynamicMeshElements);

	const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

	auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
		GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy(IsSelected()) : NULL,
		FLinearColor(0, 0.5f, 1.f)
		);

	Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

	FMaterialRenderProxy* MaterialProxy = NULL;
	if (bWireframe)
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

FPrimitiveViewRelevance FCProceduralPackMeshSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = true;// IsShown(View);
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bDynamicRelevance = true;
	MaterialRelevance.SetPrimitiveViewRelevance(Result);

	return Result;
}

bool FCProceduralPackMeshSceneProxy::CanBeOccluded() const
{
	return !MaterialRelevance.bDisableDepthTest;
}

uint32 FCProceduralPackMeshSceneProxy::GetMemoryFootprint(void) const
{
	return(sizeof(*this) + GetAllocatedSize());
}

uint32 FCProceduralPackMeshSceneProxy::GetAllocatedSize(void) const
{
	return(FPrimitiveSceneProxy::GetAllocatedSize());
}



//////////////////////////////////////////////////////////////////////////

UCustomPackProceduralMeshComponent::UCustomPackProceduralMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	bounds_scale = 1.0f;
	bounds_offset = FVector(0, 0, 0);
	render_proxy_ready = false;
	calc_local_vec_min = FVector(FLT_MIN, FLT_MIN, FLT_MIN);
	calc_local_vec_max = FVector(FLT_MAX, FLT_MAX, FLT_MAX);
	bWantsInitializeComponent = true;
	recreate_render_proxy = false;

//	SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
}

bool UCustomPackProceduralMeshComponent::SetProceduralMeshTriData(const FProceduralPackMeshTriData& TriData)
{
	defaultTriData = TriData;

	//UpdateCollision();

	// Need to recreate scene proxy to send it over
	MarkRenderStateDirty();

	return true;
}

void UCustomPackProceduralMeshComponent::RecreateRenderProxy(bool flag_in)
{
	recreate_render_proxy = flag_in;
}

void UCustomPackProceduralMeshComponent::ForceAnUpdate(int render_packet_idx)
{
	// Need to recreate scene proxy to send it over
	if (recreate_render_proxy)
	{
		MarkRenderStateDirty();
		recreate_render_proxy = false;
		return;
	}

	std::lock_guard<std::mutex> cur_lock(local_lock);
	FCProceduralPackMeshSceneProxy *localRenderProxy = GetLocalRenderProxy();
	if (render_proxy_ready && localRenderProxy) {
		if (render_packet_idx >= 0)
		{
			localRenderProxy->SetActiveRenderPacketIdx(render_packet_idx);
		}

		localRenderProxy->UpdateDynamicComponentData();
		ProcessCalcBounds(localRenderProxy);
		MarkRenderTransformDirty();
	}
}

void 
UCustomPackProceduralMeshComponent::SetTagString(FString tag_in)
{
	tagStr = tag_in;
}

FPrimitiveSceneProxy* UCustomPackProceduralMeshComponent::CreateSceneProxy()
{
	std::lock_guard<std::mutex> cur_lock(local_lock);
	
	FCProceduralPackMeshSceneProxy* Proxy = NULL;
	// Only if have enough triangles
	if(defaultTriData.point_num > 0)
	{
		Proxy = new FCProceduralPackMeshSceneProxy(this, &defaultTriData);
		render_proxy_ready = true;
		ProcessCalcBounds(Proxy);
	}
	
	return Proxy;
}

int32 UCustomPackProceduralMeshComponent::GetNumMaterials() const
{
	return 1;
}

void UCustomPackProceduralMeshComponent::ProcessCalcBounds(FCProceduralPackMeshSceneProxy *localRenderProxy)
{
	FProceduralPackMeshRenderPacket * cur_packet = nullptr;
	bool can_calc = false;
	if (render_proxy_ready && localRenderProxy)
	{
		cur_packet = localRenderProxy->GetActiveRenderPacket();
		if (cur_packet)
		{
			can_calc = (cur_packet->point_num > 0);
		}
	}

	const float bounds_max_scalar = 100000.0f;
	calc_local_vec_min = FVector(-bounds_max_scalar, -bounds_max_scalar, -bounds_max_scalar);
	calc_local_vec_max = FVector(bounds_max_scalar, bounds_max_scalar, bounds_max_scalar);
	
	// Only if have enough triangles
	if (can_calc)
	{
		const int x_id = 0;
		const int y_id = 2;
		const int z_id = 1;

		auto cur_pts = cur_packet->points;

		// Minimum Vector: It's set to the first vertex's position initially (NULL == FVector::ZeroVector might be required and a known vertex vector has intrinsically valid values)
		FVector vecMin = FVector(cur_pts[x_id], cur_pts[y_id], cur_pts[z_id]);
		if ( (vecMin.X == FLT_MIN) || (vecMin.Y == FLT_MIN) || (vecMin.Z == FLT_MIN)
			|| (vecMin.X == FLT_MAX) || (vecMin.Y == FLT_MAX) || (vecMin.Z == FLT_MAX))
		{
			vecMin.Set(0, 0, 0);
		}

		// Maximum Vector: It's set to the first vertex's position initially (NULL == FVector::ZeroVector might be required and a known vertex vector has intrinsically valid values)
		FVector vecMax = vecMin;

		// Get maximum and minimum X, Y and Z positions of vectors
		FVector vecMidPt(0, 0, 0);
		for (int32 i = 0; i < cur_packet->point_num; i++)
		{
			int32 ptIdx = i * 3;
			auto posX = cur_pts[ptIdx + x_id];
			auto posY = cur_pts[ptIdx + y_id];
			auto posZ = cur_pts[ptIdx + z_id];

			bool not_flt_min = (posX != FLT_MIN) && (posY != FLT_MIN) && (posZ != FLT_MIN);
			bool not_flt_max = (posX != FLT_MAX) && (posY != FLT_MAX) && (posZ != FLT_MAX);

			if (not_flt_min && not_flt_max) {
				vecMin.X = (vecMin.X > posX) ? posX : vecMin.X;

				vecMin.Y = (vecMin.Y > posY) ? posY : vecMin.Y;

				vecMin.Z = (vecMin.Z > posZ) ? posZ : vecMin.Z;

				vecMax.X = (vecMax.X < posX) ? posX : vecMax.X;

				vecMax.Y = (vecMax.Y < posY) ? posY : vecMax.Y;

				vecMax.Z = (vecMax.Z < posZ) ? posZ : vecMax.Z;
			}
		}

		const float lscale = bounds_scale;
		FVector lScaleVec(lscale, lscale, lscale);

		vecMidPt = (vecMax + vecMin) * 0.5f;
		vecMax = (vecMax - vecMidPt) * lScaleVec + vecMidPt;
		vecMin = (vecMin - vecMidPt) * lScaleVec + vecMidPt;

		if ((vecMin.X <= -bounds_max_scalar) || (vecMin.Y <= -bounds_max_scalar) || (vecMin.Z <= -bounds_max_scalar)
			|| (vecMin.X >= bounds_max_scalar) || (vecMin.Y >= bounds_max_scalar) || (vecMin.Z >= bounds_max_scalar) ||
			(vecMax.X <= -bounds_max_scalar) || (vecMax.Y <= -bounds_max_scalar) || (vecMax.Z <= -bounds_max_scalar)
			|| (vecMax.X >= bounds_max_scalar) || (vecMax.Y >= bounds_max_scalar) || (vecMax.Z >= bounds_max_scalar))
		{
			vecMin.Set(-bounds_max_scalar, -bounds_max_scalar, -bounds_max_scalar);
			vecMax.Set(bounds_max_scalar, bounds_max_scalar, bounds_max_scalar);
		}

		calc_local_vec_min = vecMin;
		calc_local_vec_max = vecMax;

		debugSphere = FBoxSphereBounds(FBox(calc_local_vec_min, calc_local_vec_max)).GetSphere();
	}
}

FBoxSphereBounds UCustomPackProceduralMeshComponent::CalcBounds(const FTransform & LocalToWorld) const
{
	FBoxSphereBounds ret_bounds = FBoxSphereBounds(FBox(calc_local_vec_min, calc_local_vec_max));

	if (ret_bounds.ContainsNaN())
	{
		ret_bounds = FBoxSphereBounds(FBox(FVector(-100, -100, -100),
			FVector(100, 100, 100)));
	}

	// transform the bounds by the given transform
	ret_bounds = ret_bounds.TransformBy(LocalToWorld);

	return ret_bounds;
}

void UCustomPackProceduralMeshComponent::SetBoundsScale(float value_in)
{
	bounds_scale = value_in;
}

void UCustomPackProceduralMeshComponent::SetBoundsOffset(const FVector& offset_in)
{
	bounds_offset = offset_in;
}

FSphere 
UCustomPackProceduralMeshComponent::GetDebugBoundsSphere() const
{
	return debugSphere.TransformBy(GetComponentTransform());
}

/*
bool UCustomPackProceduralMeshComponent::GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
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

bool UCustomPackProceduralMeshComponent::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
{
	return (ProceduralMeshTris.Num() > 0);
}
*/

void UCustomPackProceduralMeshComponent::UpdateBodySetup()
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

void UCustomPackProceduralMeshComponent::UpdateCollision()
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

UBodySetup* UCustomPackProceduralMeshComponent::GetBodySetup()
{
	UpdateBodySetup();
	return ModelBodySetup;
}

void UCustomPackProceduralMeshComponent::InitializeComponent()
{
	UMeshComponent::InitializeComponent();
	render_proxy_ready = false;
	MarkRenderStateDirty();
}
