
#include "CreaturePluginPCH.h"
#include "CreatureMeshComponent.h"
#include "CreatureAnimStateMachine.h"
//////////////////////////////////////////////////////////////////////////
//Changed by god of pen
//////////////////////////////////////////////////////////////////////////
#include "CreatureAnimationClipsStore.h"
#include "CreatureAnimStateMachineInstance.h"

DECLARE_CYCLE_STAT(TEXT("CreatureMesh_Tick"), STAT_CreatureMesh_Tick, STATGROUP_Creature);
DECLARE_CYCLE_STAT(TEXT("CreatureMesh_UpdateCoreValues"), STAT_CreatureMesh_UpdateCoreValues, STATGROUP_Creature);
DECLARE_CYCLE_STAT(TEXT("CreatureMesh_MeshUpdate"), STAT_CreatureMesh_MeshUpdate, STATGROUP_Creature);

static void GenerateTriangle(TArray<FProceduralMeshTriangle>& OutTriangles)
{
	FProceduralMeshTriangle triangle;
	triangle.Vertex0.Position.Set(0.f, -10.f, 0.f);
	triangle.Vertex1.Position.Set(0.f, 10.f, 0.f);
	triangle.Vertex2.Position.Set(10.f, 0.f, 0.f);
	static const FColor Blue(51, 51, 255);
	triangle.Vertex0.Color = Blue;
	triangle.Vertex1.Color = Blue;
	triangle.Vertex2.Color = Blue;
	triangle.Vertex0.U = 0.0f;
	triangle.Vertex0.V = 0.0f;
	triangle.Vertex1.U = 1.0f;
	triangle.Vertex1.V = 0.0f;
	triangle.Vertex2.U = 0.5f;
	triangle.Vertex2.V = 0.75f;
	OutTriangles.Add(triangle);
}

static std::string ConvertToString(FString str)
{
	std::string t = TCHAR_TO_UTF8(*str);
	return t;
}

// UCreatureMeshComponent
UCreatureMeshComponent::UCreatureMeshComponent(const FObjectInitializer& ObjectInitializer)
	: UCustomProceduralMeshComponent(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	InitStandardValues();
}

void UCreatureMeshComponent::SetBluePrintActiveAnimation(FString name_in)
{
	creature_core.SetBluePrintActiveAnimation(name_in);
}

void UCreatureMeshComponent::SetBluePrintBlendActiveAnimation(FString name_in, float factor)
{
	creature_core.SetBluePrintBlendActiveAnimation(name_in, factor);
}

void UCreatureMeshComponent::SetBluePrintAnimationCustomTimeRange(FString name_in, int32 start_time, int32 end_time)
{
	creature_core.SetBluePrintAnimationCustomTimeRange(name_in, start_time, end_time);
}

void UCreatureMeshComponent::MakeBluePrintPointCache(FString name_in, int32 approximation_level)
{
	creature_core.MakeBluePrintPointCache(name_in, approximation_level);
}

void UCreatureMeshComponent::ClearBluePrintPointCache(FString name_in, int32 approximation_level)
{
	creature_core.ClearBluePrintPointCache(name_in, approximation_level);
}

void UCreatureMeshComponent::SetBluePrintUsePointCache(bool flag_in)
{
	creature_core.SetGlobalEnablePointCache(flag_in);
}

bool UCreatureMeshComponent::GetBluePrintUsePointCache()
{
	return creature_core.GetGlobalEnablePointCache();
}

FTransform 
UCreatureMeshComponent::GetBluePrintBoneXform(FString name_in, bool world_transform, float position_slide_factor)
{
	return creature_core.GetBluePrintBoneXform(name_in, world_transform, position_slide_factor, GetComponentToWorld());
}

void UCreatureMeshComponent::SetBluePrintAnimationLoop(bool flag_in)
{
	creature_core.SetBluePrintAnimationLoop(flag_in);

	if (enable_collection_playback && active_collection_clip)
	{
		active_collection_loop = flag_in;
	}
}

bool 
UCreatureMeshComponent::GetBluePrintAnimationLoop() const
{
	return creature_core.is_looping;
}

void 
UCreatureMeshComponent::SetBluePrintAnimationPlay(bool flag_in)
{
	creature_core.SetBluePrintAnimationPlay(flag_in);

	if (enable_collection_playback && active_collection_clip)
	{
		active_collection_play = flag_in;
	}
}

void 
UCreatureMeshComponent::SetBluePrintAnimationPlayFromStart()
{
	creature_core.SetBluePrintAnimationPlayFromStart();

	if (enable_collection_playback && active_collection_clip)
	{
		SetBluePrintAnimationResetToStart();
		active_collection_play = true;
	}
}

void UCreatureMeshComponent::SetBluePrintAnimationResetToStart()
{
	creature_core.SetBluePrintAnimationResetToStart();

	if (enable_collection_playback && active_collection_clip)
	{
		SwitchToCollectionClip(active_collection_clip);
		auto cur_data = GetCollectionDataFromClip(active_collection_clip);
		cur_data->creature_core.SetBluePrintAnimationResetToStart();
	}
}

float 
UCreatureMeshComponent::GetBluePrintAnimationFrame()
{
	return creature_core.GetBluePrintAnimationFrame();
}

void UCreatureMeshComponent::SetBluePrintRegionAlpha(FString region_name_in, uint8 alpha_in)
{
	creature_core.SetBluePrintRegionAlpha(region_name_in, alpha_in);
}

void UCreatureMeshComponent::RemoveBluePrintRegionAlpha(FString region_name_in)
{
	creature_core.RemoveBluePrintRegionAlpha(region_name_in);
}

void UCreatureMeshComponent::SetBluePrintRegionCustomOrder(TArray<FString> order_in)
{
	creature_core.SetBluePrintRegionCustomOrder(order_in);
}

void UCreatureMeshComponent::ClearBluePrintRegionCustomOrder()
{
	creature_core.ClearBluePrintRegionCustomOrder();
}

void UCreatureMeshComponent::SetIsDisabled(bool flag_in)
{
	creature_core.SetIsDisabled(flag_in);
}

void UCreatureMeshComponent::SetBluePrintRegionItemSwap(FString region_name_in, int32 tag)
{
	creature_core.SetBluePrintRegionItemSwap(region_name_in, tag);
}

void UCreatureMeshComponent::SetBluePrintUseAnchorPoints(bool flag_in)
{
	creature_core.SetUseAnchorPoints(flag_in);
}

bool UCreatureMeshComponent::GetBluePrintUseAnchorPoints() const
{
	return creature_core.GetUseAnchorPoints();
}

void UCreatureMeshComponent::RemoveBluePrintRegionItemSwap(FString region_name_in)
{
	creature_core.RemoveBluePrintRegionItemSwap(region_name_in);
}

CreatureCore& UCreatureMeshComponent::GetCore()
{
	return creature_core;
}

void UCreatureMeshComponent::SwitchToCollectionClip(FCreatureMeshCollectionClip * clip_in)
{
	active_collection_clip = clip_in;
	clip_in->active_index = 0;
	
	SetActiveCollectionAnimation(clip_in);
}

void UCreatureMeshComponent::SetActiveCollectionAnimation(FCreatureMeshCollectionClip * clip_in)
{
	auto& cur_seq = clip_in->sequence_clips[clip_in->active_index];
	auto data_idx = cur_seq.collection_data_index;
	if (data_idx < 0 || data_idx >= collectionData.Num())
	{
		return;
	}

	auto& cur_data = collectionData[data_idx];
	auto& cur_anim_name = cur_seq.animation_name;
	
	cur_data.creature_core.SetBluePrintActiveAnimation(cur_anim_name);
	cur_data.creature_core.SetBluePrintAnimationResetToStart();
	cur_data.creature_core.SetBluePrintAnimationLoop(false);
	cur_data.creature_core.SetBluePrintAnimationPlay(true);

	FCProceduralMeshSceneProxy *localRenderProxy = GetLocalRenderProxy();
	if (localRenderProxy) {
		localRenderProxy->SetActiveRenderPacketIdx(data_idx);
	}
	else {
		RecreateRenderProxy(true);
	}

	SetMaterial(0, cur_data.collection_material);
	if (localRenderProxy)
	{
		localRenderProxy->SetNeedsMaterialUpdate(true);
	}

	ForceAnUpdate(data_idx);
}

void UCreatureMeshComponent::SetBluePrintActiveCollectionClip(FString name_in)
{
	if (!enable_collection_playback)
	{
		return;
	}

	if (active_collection_clip_name == name_in)
	{
		return;
	}

	active_collection_clip_name = name_in;
	active_collection_clip = nullptr;

	for (auto& cur_collection_clip : collectionClips)
	{
		if (cur_collection_clip.collection_name == name_in)
		{
			SwitchToCollectionClip(&cur_collection_clip);
			break;
		}
	}
}

void UCreatureMeshComponent::InitStandardValues()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bWantsInitializeComponent = true;

	animation_speed = 2.0f;
	smooth_transitions = false;
	bone_data_size = 0.01f;
	bone_data_length_factor = 0.02f;
	creature_bounds_scale = 1.0f;
	creature_debug_draw = false;
	creature_bounds_offset = FVector(0, 0, 0);
	region_overlap_z_delta = 0.01f;
	enable_collection_playback = false;
	active_collection_clip = nullptr;
	active_collection_loop = true;
	active_collection_play = true;
	creature_animation_asset = nullptr;
	can_use_point_cache = false;

	// Generate a single dummy triangle
	/*
	TArray<FProceduralMeshTriangle> triangles;
	GenerateTriangle(triangles);
	SetProceduralMeshTriangles(triangles);
	*/
}

void UCreatureMeshComponent::UpdateCoreValues()
{
	SCOPE_CYCLE_COUNTER(STAT_CreatureMesh_UpdateCoreValues);

	creature_core.creature_filename = creature_filename;

	if (creature_animation_asset) {
		creature_core.pJsonData = &creature_animation_asset->GetJsonString();
		creature_core.creature_asset_filename = creature_animation_asset->GetCreatureFilename();

		creature_animation_asset->LoadPointCacheForAllClips(&creature_core);
	}

	creature_core.bone_data_size = bone_data_size;
	creature_core.bone_data_length_factor = bone_data_length_factor;
	creature_core.region_overlap_z_delta = region_overlap_z_delta;
}

void UCreatureMeshComponent::PrepareRenderData()
{
	RecreateRenderProxy(true);
	SetProceduralMeshTriData(creature_core.GetProcMeshData());
}

void UCreatureMeshComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (enable_collection_playback)
	{
		CollectionInit();
	}
	else {
		StandardInit();
	}
}

void UCreatureMeshComponent::RunTick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_CreatureMesh_Tick);

	UpdateCoreValues();

	if (bHiddenInGame)
	{
		return;
	}

	bool can_tick = creature_core.RunTick(DeltaTime * animation_speed);

	if (can_tick) {
		// Events
		bool announce_start = creature_core.GetAndClearShouldAnimStart();
		bool announce_end = creature_core.GetAndClearShouldAnimEnd();

		float cur_runtime = (creature_core.GetCreatureManager()->getActualRunTime());
		animation_frame = cur_runtime;

		if (announce_start)
		{
			CreatureAnimationStartEvent.Broadcast(cur_runtime);
		}

		if (announce_end)
		{
			CreatureAnimationEndEvent.Broadcast(cur_runtime);
		}

		DoCreatureMeshUpdate();
	}

}

void 
UCreatureMeshComponent::RunCollectionTick(float DeltaTime)
{
	UpdateCoreValues();

	if (bHiddenInGame)
	{
		return;
	}

	if (active_collection_clip == nullptr)
	{
		return;
	}

	float true_delta_time = DeltaTime * animation_speed;
	auto cur_data = GetCollectionDataFromClip(active_collection_clip);
	if (cur_data == nullptr)
	{
		return;
	}

	if (cur_data->animation_speed > 0)
	{
		true_delta_time = DeltaTime * cur_data->animation_speed;
	}

	if (active_collection_play == false)
	{
		true_delta_time = 0;
	}

	TArray<FProceduralMeshTriangle>& write_triangles = cur_data->ProceduralMeshTris;
	auto& cur_core = cur_data->creature_core;
	if (cur_core.GetCreatureManager() == nullptr)
	{
		return;
	}

	cur_core.should_play = true;
	bool can_tick = cur_core.RunTick(true_delta_time);

	if (can_tick) {
		// Events
		bool announce_start = cur_core.GetAndClearShouldAnimStart();
		bool announce_end = cur_core.GetAndClearShouldAnimEnd();

		float cur_runtime = (cur_core.GetCreatureManager()->getActualRunTime());

		bool is_collection_start = (active_collection_clip->active_index == 0) && announce_start;
		bool is_collection_end = (active_collection_clip->active_index == active_collection_clip->sequence_clips.Num() - 1) && announce_end;

		if (is_collection_start)
		{
			CreatureAnimationStartEvent.Broadcast(cur_runtime);
		}

		if (is_collection_end)
		{
			CreatureAnimationEndEvent.Broadcast(cur_runtime);
		}

		// Process Collection Clip
		auto cur_manager = cur_core.GetCreatureManager();
		auto& all_animations = cur_manager->GetAllAnimations();
		auto& cur_token = active_collection_clip->sequence_clips[active_collection_clip->active_index];
		std::string anim_string = ConvertToString(cur_token.animation_name);
		if (all_animations.count(anim_string) == 0)
		{
			return;
		}

		auto clip_animation = all_animations.at(anim_string).get();
		float next_time = cur_runtime + true_delta_time;

		if (next_time >= clip_animation->getEndTime())
		{
			// At the end of current clip, see if we should switch to the next clip
			int next_active_index = active_collection_clip->active_index + 1;
			int seq_num = active_collection_clip->sequence_clips.Num();
			bool do_switch = false;
			if (next_active_index >= seq_num)
			{
				if (active_collection_loop)
				{
					active_collection_clip->active_index = 0;
					do_switch = true;
				}
			}
			else {
				active_collection_clip->active_index++;
				do_switch = true;
			}

			if (do_switch)
			{
				SetActiveCollectionAnimation(active_collection_clip);
			}
		}

		DoCreatureMeshUpdate(GetCollectionDataIndexFromClip(active_collection_clip));
	}
}

void UCreatureMeshComponent::DoCreatureMeshUpdate(int render_packet_idx)
{
	SCOPE_CYCLE_COUNTER(STAT_CreatureMesh_MeshUpdate);

	FCProceduralMeshSceneProxy *localRenderProxy = GetLocalRenderProxy();
	if (localRenderProxy)
	{
		localRenderProxy->SetNeedsIndexUpdate(creature_core.should_update_render_indices);
	}

	// Update Mesh
	SetBoundsScale(creature_bounds_scale);
	SetBoundsOffset(creature_bounds_offset);
	SetExtraXForm(GetComponentToWorld());

	ForceAnUpdate(render_packet_idx);

	// Debug

	if (creature_debug_draw) {
		FSphere debugSphere = GetDebugBoundsSphere();
		DrawDebugSphere(
			GetWorld(),
			debugSphere.Center,
			debugSphere.W,
			32,
			FColor(255, 0, 0)
			);

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Sphere pos is: (%f, %f, %f)"), debugSphere.Center.X, debugSphere.Center.Y, debugSphere.Center.Z));
		FTransform wTransform = GetComponentToWorld();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Walk pos is: (%f, %f, %f)"), wTransform.GetLocation().X,
			wTransform.GetLocation().Y,
			wTransform.GetLocation().Z));
	}
}

int 
UCreatureMeshComponent::GetCollectionDataIndexFromClip(FCreatureMeshCollectionClip * clip_in)
{
	int seq_idx = active_collection_clip->active_index;

	if (seq_idx < 0 || seq_idx >= active_collection_clip->sequence_clips.Num())
	{
		return -1;
	}

	auto& cur_seq = active_collection_clip->sequence_clips[seq_idx];
	auto data_index = cur_seq.collection_data_index;

	return data_index;
}

FCreatureMeshCollection *
UCreatureMeshComponent::GetCollectionDataFromClip(FCreatureMeshCollectionClip * clip_in)
{
	auto data_index = GetCollectionDataIndexFromClip(clip_in);
	if (data_index < 0 || data_index >= collectionData.Num())
	{
		return nullptr;
	}

	FCreatureMeshCollection * cur_data = &collectionData[data_index];

	return cur_data;
}

void UCreatureMeshComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//////////////////////////////////////////////////////////////////////////
	//ChangedByGod of Pen
	//////////////////////////////////////////////////////////////////////////
	if (StateMachineInstance !=nullptr)
	{
		StateMachineInstance->GetCurrentState()->CheckCondition(StateMachineInstance);
	}
	
	if (enable_collection_playback)
	{
		RunCollectionTick(DeltaTime);
	}
	else {
		if (creature_core.GetCreatureManager()) {
			RunTick(DeltaTime);
		}
	}
}

void UCreatureMeshComponent::OnRegister()
{
	Super::OnRegister();

	StandardInit();
}

void UCreatureMeshComponent::StandardInit()
{
	LoadAnimationFromStore();
	UpdateCoreValues();

	creature_core.do_file_warning = !enable_collection_playback;
	bool retval = creature_core.InitCreatureRender();
	creature_core.region_alpha_map.Empty();

	if (retval)
	{
		PrepareRenderData();
		RunTick(0.1f);
		
		if (!start_animation_name.IsEmpty())
		{
			SetBluePrintActiveAnimation(start_animation_name);
		}
	}
}

void UCreatureMeshComponent::CollectionInit()
{
	RecreateRenderProxy(true);
	for (int32 collectionDataIndex = 0; collectionDataIndex < collectionData.Num(); collectionDataIndex++)
	{
		FCreatureMeshCollection& cur_data = collectionData[collectionDataIndex];
		auto& cur_core = cur_data.creature_core;

		cur_core.creature_filename = cur_data.creature_filename;
		cur_core.bone_data_size = bone_data_size;
		cur_core.bone_data_length_factor = bone_data_length_factor;
		cur_core.region_overlap_z_delta = region_overlap_z_delta;
		//////////////////////////////////////////////////////////////////////////
		//changed by God of Pen
		//////////////////////////////////////////////////////////////////////////
		if (cur_data.creature_core.pJsonData!=nullptr)
		{
			cur_core.pJsonData = cur_data.creature_core.pJsonData;
		}

		bool retval = cur_core.InitCreatureRender();
		if (retval)
		{
			auto& load_triangles = cur_core.draw_triangles;
			cur_data.ProceduralMeshTris = load_triangles;

			if (cur_data.source_asset)
			{
				for (auto & clip : collectionClips)
				{
					for (auto &token : clip.sequence_clips)
					{
						if (token.collection_data_index == collectionDataIndex)
						{
							// load the point cache into the creature core, if available for this clip
							cur_data.source_asset->LoadPointCacheForClip(token.animation_name, &cur_core);
						}
					}
				}
			}
		}
	}

	if (!start_animation_name.IsEmpty())
	{
		SetBluePrintActiveCollectionClip(start_animation_name);
	}
}

FPrimitiveSceneProxy* UCreatureMeshComponent::CreateSceneProxy()
{
	if (enable_collection_playback == false)
	{
		return UCustomProceduralMeshComponent::CreateSceneProxy();
	}

	for (auto& cur_data : collectionData)
	{
		//////////////////////////////////////////////////////////////////////////
		//Changed by God of PEn
		//////////////////////////////////////////////////////////////////////////
		if (cur_data.ProceduralMeshTris.Num() <= 0)
		{
			CollectionInit();
			break;
		}
	}


	std::lock_guard<std::mutex> cur_lock(local_lock);

	FCProceduralMeshSceneProxy* Proxy = NULL;
	// Only if have enough triangles
	Proxy = new FCProceduralMeshSceneProxy(this, nullptr);

	// Loop through and add in the collectionData
	for (auto& cur_data : collectionData)
	{
		auto proc_mesh_data = cur_data.creature_core.GetProcMeshData();
		if (proc_mesh_data.point_num > 0) {
			Proxy->AddRenderPacket(&proc_mesh_data);
		}
	}

	render_proxy_ready = true;

	return Proxy;
}

void UCreatureMeshComponent::BeginPlay()
{
	Super::BeginPlay();
	if (StateMachineAsset != nullptr)
	{
		// copy the state machine asset to an instance for exclusive use by this component
		// necessary because multiple components may share the same asset, so we need our own data for this instance

		StateMachineInstance = NewObject<UCreatureAnimStateMachineInstance>(this);

		StateMachineInstance->InitInstance(StateMachineAsset);
	}

	creature_core.SetGlobalEnablePointCache(can_use_point_cache);
	for (auto& cur_data : collectionData)
	{
		cur_data.creature_core.SetGlobalEnablePointCache(can_use_point_cache);
	}
}

void UCreatureMeshComponent::LoadAnimationFromStore()
{
	if (ClipStore==nullptr)
	{
		return;
	}
	collectionData.Empty();
	collectionClips.Empty();
	ClipStore->LoadAnimationDataToComponent(this);
	enable_collection_playback = true;
}
