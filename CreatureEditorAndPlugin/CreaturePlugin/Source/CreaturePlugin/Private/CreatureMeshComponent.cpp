
#include "CreaturePluginPCH.h"
#include "CreatureMeshComponent.h"
#include "CreatureAnimStateMachine.h"
//////////////////////////////////////////////////////////////////////////
//Changed by god of pen
//////////////////////////////////////////////////////////////////////////
#include "CreatureAnimationClipsStore.h"
#include "CreatureAnimStateMachineInstance.h"

#include <math.h>

#ifdef _WIN32
// Put this in if you get: decorated name length exceeded, name was truncated 
// for the Visual Studio Compiler
#pragma warning(disable : 4503)
#pragma warning(disable : 4668)
#endif

#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
#endif


DECLARE_CYCLE_STAT(TEXT("CreatureMesh_Tick"), STAT_CreatureMesh_Tick, STATGROUP_Creature);
DECLARE_CYCLE_STAT(TEXT("CreatureMesh_AsyncTick"), STAT_CreatureMesh_Tick_Async, STATGROUP_Creature);
DECLARE_CYCLE_STAT(TEXT("CreatureMesh_UpdateCoreValues"), STAT_CreatureMesh_UpdateCoreValues, STATGROUP_Creature);
DECLARE_CYCLE_STAT(TEXT("CreatureMesh_MeshUpdate"), STAT_CreatureMesh_MeshUpdate, STATGROUP_Creature);
DECLARE_CYCLE_STAT(TEXT("CreatureMesh_ProcessCreatureCoreResults"), STAT_CreatureMesh_ProcessCreatureCoreResults, STATGROUP_Creature);

// UCreatureMeshComponent
UCreatureMeshComponent::UCreatureMeshComponent(const FObjectInitializer& ObjectInitializer)
	: UCustomProceduralMeshComponent(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;

	EndPhysicsTickFunction.TickGroup = TG_PostPhysics;
	EndPhysicsTickFunction.bCanEverTick = true;
	EndPhysicsTickFunction.bStartWithTickEnabled = true;
	EndPhysicsTickFunction.bTickEvenWhenPaused = false;

	InitStandardValues();
}

void UCreatureMeshComponent::SetBluePrintAlwaysTick(bool flag_in)
{
	PrimaryComponentTick.bTickEvenWhenPaused = flag_in;
	EndPhysicsTickFunction.bTickEvenWhenPaused = flag_in;
}

void UCreatureMeshComponent::SetBluePrintActiveAnimation(FString name_in)
{
	creature_core.SetBluePrintActiveAnimation(FName(*name_in));
	ResetFrameCallbacks();
}

void UCreatureMeshComponent::SetBluePrintActiveAnimation_Name(FName name_in)
{
	creature_core.SetBluePrintActiveAnimation(name_in);
	ResetFrameCallbacks();
}

void UCreatureMeshComponent::SetBluePrintBlendActiveAnimation(FString name_in, float factor)
{
	creature_core.SetBluePrintBlendActiveAnimation(FName(*name_in), factor);
	ResetFrameCallbacks();
}

void UCreatureMeshComponent::SetBluePrintBlendActiveAnimation_Name(FName name_in, float factor)
{
	creature_core.SetBluePrintBlendActiveAnimation(name_in, factor);
	ResetFrameCallbacks();
}

void UCreatureMeshComponent::SetBluePrintAnimationCustomTimeRange(FString name_in, int32 start_time, int32 end_time)
{
	creature_core.SetBluePrintAnimationCustomTimeRange(FName(*name_in), start_time, end_time);
	ResetFrameCallbacks();
}

void UCreatureMeshComponent::SetBluePrintAnimationCustomTimeRange_Name(FName name_in, int32 start_time, int32 end_time)
{
	creature_core.SetBluePrintAnimationCustomTimeRange(name_in, start_time, end_time);
	ResetFrameCallbacks();
}

void UCreatureMeshComponent::MakeBluePrintPointCache(FString name_in, int32 approximation_level)
{
	creature_core.MakeBluePrintPointCache(FName(*name_in), approximation_level);
}

void UCreatureMeshComponent::MakeBluePrintPointCache_Name(FName name_in, int32 approximation_level)
{
	creature_core.MakeBluePrintPointCache(name_in, approximation_level);
}

void UCreatureMeshComponent::ClearBluePrintPointCache(FString name_in, int32 approximation_level)
{
	creature_core.ClearBluePrintPointCache(FName(*name_in), approximation_level);
}

void UCreatureMeshComponent::ClearBluePrintPointCache_Name(FName name_in, int32 approximation_level)
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

FTransform UCreatureMeshComponent::GetBluePrintBoneXform(FString name_in, bool world_transform, float position_slide_factor)
{
	return creature_core.GetBluePrintBoneXform(FName(*name_in), world_transform, position_slide_factor, GetComponentToWorld());
}

FTransform UCreatureMeshComponent::GetBluePrintBoneXform_Name(FName name_in, bool world_transform, float position_slide_factor) const
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

	ResetFrameCallbacks();
}

float 
UCreatureMeshComponent::GetBluePrintAnimationFrame()
{
	return creature_core.GetBluePrintAnimationFrame();
}

void 
UCreatureMeshComponent::SetBluePrintAnimationFrame(float time_in)
{
	creature_core.SetBluePrintAnimationFrame(time_in);
}

void UCreatureMeshComponent::SetBluePrintRegionAlpha(FString region_name_in, uint8 alpha_in)
{
	creature_core.SetBluePrintRegionAlpha(FName(*region_name_in), alpha_in);
}

void UCreatureMeshComponent::SetBluePrintRegionAlpha_Name(FName region_name_in, uint8 alpha_in)
{
	creature_core.SetBluePrintRegionAlpha(region_name_in, alpha_in);
}

void UCreatureMeshComponent::RemoveBluePrintRegionAlpha(FString region_name_in)
{
	creature_core.RemoveBluePrintRegionAlpha(FName(*region_name_in));
}

void UCreatureMeshComponent::RemoveBluePrintRegionAlpha_Name(FName region_name_in)
{
	creature_core.RemoveBluePrintRegionAlpha(region_name_in);
}

void UCreatureMeshComponent::SetBluePrintRegionCustomOrder(TArray<FString> order_in)
{
	TArray<FName> order_name;
	for (const FString &str : order_in)
	{
		order_name.Add(FName(*str));
	}
	creature_core.SetBluePrintRegionCustomOrder(order_name);
}

void UCreatureMeshComponent::SetBluePrintRegionCustomOrder_Name(TArray<FName> order_in)
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
	creature_core.SetBluePrintRegionItemSwap(FName(*region_name_in), tag);
}

void UCreatureMeshComponent::SetBluePrintRegionItemSwap_Name(FName region_name_in, int32 tag)
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

void UCreatureMeshComponent::SetBluePrintBonesOverride(const TArray<FCreatureBoneOverride>& bones_list_in)
{
	bones_override_list = bones_list_in;
}

void UCreatureMeshComponent::ClearBluePrintBonesOverride()
{
	bones_override_list.Empty();
}

void UCreatureMeshComponent::RemoveBluePrintRegionItemSwap(FString region_name_in)
{
	creature_core.RemoveBluePrintRegionItemSwap(FName(*region_name_in));
}

void UCreatureMeshComponent::RemoveBluePrintRegionItemSwap_Name(FName region_name_in)
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

	if (cur_data.creature_core.GetCreatureManager() == nullptr)
	{
		return;
	}
	
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

	if (GetMaterial(0) != cur_data.collection_material)
	{
		SetMaterial(0, cur_data.collection_material);
		if (localRenderProxy)
		{
			localRenderProxy->SetNeedsMaterialUpdate(true);
		}
	}

	ForceAnUpdate(data_idx);
}

void UCreatureMeshComponent::SetBluePrintActiveCollectionClip(FString name_in)
{
	SetBluePrintActiveCollectionClip_Name(FName(*name_in));
}

void UCreatureMeshComponent::SetBluePrintActiveCollectionClip_Name(FName name_in)
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
	creature_bones_draw = false;
	creature_bounds_offset = FVector(0, 0, 0);
	region_overlap_z_delta = 0.01f;
	enable_collection_playback = false;
	active_collection_clip = nullptr;
	active_collection_loop = true;
	active_collection_play = true;
	creature_animation_asset = nullptr;
	creature_meta_asset = nullptr;
	can_use_point_cache = false;
	bones_override_blend_factor = 1.0f;
	completely_disable = false;
	fixed_timestep = 0.0f;
	run_multicore = true;

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

	FScopeLock scope_lock(creature_core.update_lock.Get());

	creature_core.creature_filename = creature_filename;

	if (creature_animation_asset && creature_core.creature_asset_filename != creature_animation_asset->GetCreatureFilename())
	{
		creature_core.pJsonData = &creature_animation_asset->GetJsonString();
		creature_core.creature_asset_filename = creature_animation_asset->GetCreatureFilename();

		creature_animation_asset->LoadPointCacheForAllClips(&creature_core);
	}

	creature_core.bone_data_size = bone_data_size;
	creature_core.bone_data_length_factor = bone_data_length_factor;
	creature_core.region_overlap_z_delta = region_overlap_z_delta;
}

void UCreatureMeshComponent::PrepareRenderData(CreatureCore &forCore)
{
	RecreateRenderProxy(true);
	SetProceduralMeshTriData(forCore.GetProcMeshData(GetWorld()->WorldType));
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

	// Frame Callback events, if any
	if ((GetWorld()->WorldType != EWorldType::Type::Editor) &&
		(GetWorld()->WorldType != EWorldType::Type::EditorPreview))
	{
		if (CreatureFrameCallbackEvent.IsBound() || CreatureRepeatFrameCallbackEvent.IsBound()) {
			const auto& cur_animation_name = creature_core.GetCreatureManager()->GetActiveAnimationName();
			auto cur_animation = creature_core.GetCreatureManager()->GetAnimation(cur_animation_name);
			auto diff_runtime = fabs(creature_core.GetCreatureManager()->getActualRunTime() - cur_animation->getStartTime());
			const auto small_num = 0.0001f;
			if (diff_runtime <= small_num)
			{
				ResetFrameCallbacks();
			}

			ProcessFrameCallbacks();
		}
	}

	// Run the animation
	if (run_multicore) {
		creatureTickResult = Async<bool>(EAsyncExecution::TaskGraph, [this, DeltaTime]()
		{
			SCOPE_CYCLE_COUNTER(STAT_CreatureMesh_Tick_Async);
			return RunTickProcessing(DeltaTime, false);
		});
	}
	else {
		auto can_tick = RunTickProcessing(DeltaTime, true);
		if (can_tick) {
			// fire events
			FireStartEndEvents();
		}
	}
}

bool UCreatureMeshComponent::RunTickProcessing(float DeltaTime, bool markDirty)
{
	// Run the animation
	bool can_tick = creature_core.RunTick(DeltaTime);

	if (can_tick)
	{
		FScopeLock cur_lock(&local_lock);

		animation_frame = creature_core.GetCreatureManager()->getActualRunTime();
		DoCreatureMeshUpdate(INDEX_NONE, markDirty);
	}

	return can_tick;
}

void UCreatureMeshComponent::ProcessCreatureCoreResult(FCreatureCoreResultTickFunction& ThisTickFunction)
{
	if (ShouldSkipTick() || (!run_multicore))
	{
		return;
	}

	SCOPE_CYCLE_COUNTER(STAT_CreatureMesh_ProcessCreatureCoreResults);
	
	bool can_tick = creatureTickResult.IsValid() && creatureTickResult.Get();

	if (can_tick)
	{
		FScopeLock cur_lock(&local_lock);

		// mark the ActorComponent dirty flags based on the result of the creature update
		// Need to recreate scene proxy to send it over
		if (recreate_render_proxy)
		{
			MarkRenderStateDirty();
			recreate_render_proxy = false;
		}
		else
		{
			FCProceduralMeshSceneProxy *localRenderProxy = GetLocalRenderProxy();
			if (render_proxy_ready && localRenderProxy)
			{
				MarkRenderTransformDirty();

				check(localRenderProxy->GetDoesActiveRenderPacketHaveVertices());
				MarkRenderDynamicDataDirty();
			}
		}

		// fire events
		FireStartEndEvents();
	}
}

void 
UCreatureMeshComponent::FireStartEndEvents()
{
	// fire events
	bool announce_start = creature_core.GetAndClearShouldAnimStart();
	bool announce_end = creature_core.GetAndClearShouldAnimEnd();

	if (announce_start)
	{
		CreatureAnimationStartEvent.Broadcast(animation_frame);
	}

	if (announce_end)
	{
		CreatureAnimationEndEvent.Broadcast(animation_frame);
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

	true_delta_time *= cur_data->animation_speed;

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
		auto anim_name = cur_token.animation_name;
		if (all_animations.Contains(anim_name) == false)
		{
			return;
		}

		auto clip_animation = all_animations[anim_name].Get();
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
		else if (next_time <= clip_animation->getStartTime())
		{
			// if we're going backwards, see if we should switch to the previous clip
			// At the end of current clip, see if we should switch to the next clip
			int next_active_index = active_collection_clip->active_index - 1;
			int seq_num = active_collection_clip->sequence_clips.Num();
			bool do_switch = false;
			if (next_active_index < 0)
			{
				if (active_collection_loop)
				{
					active_collection_clip->active_index = seq_num - 1;
					do_switch = true;
				}
			}
			else {
				active_collection_clip->active_index--;
				do_switch = true;
			}

			if (do_switch)
			{
				SetActiveCollectionAnimation(active_collection_clip);

				auto& cur_seq = active_collection_clip->sequence_clips[active_collection_clip->active_index];
				auto data_idx = cur_seq.collection_data_index;
				auto& collect_data = collectionData[data_idx];
				collect_data.creature_core.SetBluePrintAnimationResetToEnd();
			}
		}

		DoCreatureMeshUpdate(GetCollectionDataIndexFromClip(active_collection_clip));
	}
}

void UCreatureMeshComponent::DoCreatureMeshUpdate(int render_packet_idx, bool markDirty /*= true*/)
{
	SCOPE_CYCLE_COUNTER(STAT_CreatureMesh_MeshUpdate);

	FScopeLock cur_lock(&local_lock);

	FCProceduralMeshSceneProxy *localRenderProxy = GetLocalRenderProxy();
	if (localRenderProxy)
	{
		localRenderProxy->SetNeedsIndexUpdate(creature_core.should_update_render_indices);
	}

	// Update Mesh
	SetBoundsScale(creature_bounds_scale);
	SetBoundsOffset(creature_bounds_offset);

	ForceAnUpdate(render_packet_idx, markDirty);

#if !UE_BUILD_SHIPPING
	// Debug

	if (creature_debug_draw) {
		FSphere tmpDebugSphere = GetDebugBoundsSphere();
		DrawDebugSphere(
			GetWorld(),
			tmpDebugSphere.Center,
			tmpDebugSphere.W,
			32,
			FColor(255, 0, 0)
			);

		/*
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Sphere pos is: (%f, %f, %f)"), debugSphere.Center.X, debugSphere.Center.Y, debugSphere.Center.Z));
		FTransform wTransform = GetComponentToWorld();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Walk pos is: (%f, %f, %f)"), wTransform.GetLocation().X,
			wTransform.GetLocation().Y,
			wTransform.GetLocation().Z));
		*/
	}

	if (creature_bones_draw)
	{
		auto base_xform = GetComponentToWorld();
		for (auto& bone_data : creature_core.creature_manager->GetCreature()->GetRenderComposition()->getBonesMap())
		{
			auto cur_bone = bone_data.Value;
			auto cur_start_pos = cur_bone->getWorldStartPt();
			auto cur_end_pos = cur_bone->getWorldEndPt();

			FVector local_pt1(cur_start_pos.x, -3.0f, cur_start_pos.y);
			FVector local_pt2(cur_end_pos.x, -3.0f, cur_end_pos.y);
			FVector world_pt1 = base_xform.TransformPosition(local_pt1);
			FVector world_pt2 = base_xform.TransformPosition(local_pt2);

			DrawDebugLine(GetWorld(), world_pt1, world_pt2, FColor::Red);
			DrawDebugString(GetWorld(), (world_pt1 + world_pt2) * 0.5f, cur_bone->getKey().ToString());
		}
	}
#endif
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

bool UCreatureMeshComponent::ShouldSkipTick() const
{
	return (GetOwner() && GetOwner()->bHidden) || bHiddenInGame || completely_disable || !bVisible;
}

void UCreatureMeshComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	bool shouldSkipTick = ShouldSkipTick();
	
	RegisterCoreResultsTickFunction(!shouldSkipTick && !enable_collection_playback);

	if (shouldSkipTick)
	{
		return;
	}

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
			auto real_delta_time = DeltaTime * animation_speed;
			
			if (fixed_timestep > 0.0f)
			{
				real_delta_time = fixed_timestep;
			}
			
			RunTick(real_delta_time);
		}
	}
}

void FCreatureCoreResultTickFunction::ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	QUICK_SCOPE_CYCLE_COUNTER(FCreatureCoreResultTickFunction_ExecuteTick);
	FActorComponentTickFunction::ExecuteTickHelper(Target, /*bTickInEditor=*/ false, DeltaTime, TickType, [this](float DilatedTime)
	{
		Target->ProcessCreatureCoreResult(*this);
	});
}

FString FCreatureCoreResultTickFunction::DiagnosticMessage()
{
	return TEXT("FCreatureMeshComponentEndPhysicsTickFunction");
}

void UCreatureMeshComponent::OnRegister()
{
	Super::OnRegister();

	LoadAnimationFromStore();
	if (enable_collection_playback)
	{
		CollectionInit();
	}
	else {
		StandardInit();
	}
}

void UCreatureMeshComponent::RegisterComponentTickFunctions(bool bRegister)
{
	Super::RegisterComponentTickFunctions(bRegister);

	RegisterCoreResultsTickFunction(bRegister && !enable_collection_playback);
}

void UCreatureMeshComponent::RegisterCoreResultsTickFunction(bool bRegister)
{
	if (bRegister != EndPhysicsTickFunction.IsTickFunctionRegistered())
	{
		if (bRegister)
		{
			if (SetupActorComponentTickFunction(&EndPhysicsTickFunction))
			{
				EndPhysicsTickFunction.Target = this;
			}
		}
		else
		{
			EndPhysicsTickFunction.UnRegisterTickFunction();
		}
	}
}

void UCreatureMeshComponent::StandardInit()
{
	creature_core.ClearMemory();
	creature_core = CreatureCore();

	UpdateCoreValues();
	creature_core.do_file_warning = !enable_collection_playback;
	bool retval = creature_core.InitCreatureRender();
	creature_core.InitValues();

	if (creature_meta_asset)
	{
		creature_meta_asset->BuildMetaData();
		creature_core.meta_data = creature_meta_asset->GetMetaData();
	}

	if (retval)
	{
		if (!start_animation_name.IsNone())
		{
			SetBluePrintActiveAnimation_Name(start_animation_name);
		}

		creature_core.SetBluePrintAnimationResetToStart();
		PrepareRenderData(creature_core);
		
		// Register bone override callback
		bones_override_list.Empty();
		final_bones_override_list.Empty();
		internal_ik_map.Empty();
		std::function<void(TMap<FName, meshBone *>&) > cur_callback =
			std::bind(&UCreatureMeshComponent::CoreBonesOverride, this, std::placeholders::_1);
		creature_core.creature_manager->SetBonesOverrideCallback(cur_callback);
	}
	else {
		static FProceduralMeshTriData empty_data;
		SetProceduralMeshTriData(empty_data);
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

			// needed to ensure point arrays have proper data
			cur_core.SetBluePrintAnimationResetToStart();
		}
	}

	if (!start_animation_name.IsNone())
	{
		active_collection_clip = nullptr;
		active_collection_clip_name = NAME_None;
		SetBluePrintActiveCollectionClip_Name(start_animation_name);

		if (active_collection_clip)
		{
			auto& cur_seq = active_collection_clip->sequence_clips[active_collection_clip->active_index];
			auto data_idx = cur_seq.collection_data_index;

			if (collectionData.IsValidIndex(data_idx))
			{
				auto& cur_data = collectionData[data_idx];

				// prepare default render data to match code path of StandardInit
				PrepareRenderData(collectionData[data_idx].creature_core);
			}
		}
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


	FScopeLock cur_lock(&local_lock);

	auto not_editor_mode = ((GetWorld()->WorldType != EWorldType::Type::Editor) &&
		(GetWorld()->WorldType != EWorldType::Type::EditorPreview));
	FColor start_color = not_editor_mode ? FColor(0, 0, 0, 0) : FColor::White;
	FCProceduralMeshSceneProxy* Proxy = NULL;
	// Only if have enough triangles
	Proxy = new FCProceduralMeshSceneProxy(this, nullptr, start_color);

	// Loop through and add in the collectionData
	for (auto& cur_data : collectionData)
	{
		auto proc_mesh_data = cur_data.creature_core.GetProcMeshData(GetWorld()->WorldType);
		if (proc_mesh_data.point_num > 0) {

			Proxy->AddRenderPacket(&proc_mesh_data, start_color);
		}
	}

	render_proxy_ready = true;
	ProcessCalcBounds(Proxy);

	if (IsInRenderingThread())
	{
		Proxy->SetDynamicData_RenderThread();
	}
	else if(GetWorld()->bPostTickComponentUpdate)
	{
		SendRenderDynamicData_Concurrent();
	}
	else
	{
		MarkRenderDynamicDataDirty();
	}

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

	// previous pointers are now invalid
	active_collection_clip = nullptr;
	active_collection_clip_name = NAME_None;
	FCProceduralMeshSceneProxy *localRenderProxy = GetLocalRenderProxy();
	if (localRenderProxy)
	{
		localRenderProxy->ResetAllRenderPackets();

		// Loop through and add in the collectionData
		for (auto& cur_data : collectionData)
		{
			auto proc_mesh_data = cur_data.creature_core.GetProcMeshData(GetWorld()->WorldType);
			if (proc_mesh_data.point_num > 0)
			{
				auto not_editor_mode = ((GetWorld()->WorldType != EWorldType::Type::Editor) &&
					(GetWorld()->WorldType != EWorldType::Type::EditorPreview));
				FColor start_color = not_editor_mode ? FColor(0, 0, 0, 0) : FColor::White;
				localRenderProxy->AddRenderPacket(&proc_mesh_data, start_color);
			}
		}

		render_proxy_ready = true;
		ProcessCalcBounds(localRenderProxy);
	}
}

void 
UCreatureMeshComponent::CoreBonesOverride(TMap<FName, meshBone *>& bones_map)
{
	if ((internal_ik_map.Num() == 0) && (bones_override_list.Num() == 0))
	{
		return;
	}

	final_bones_override_list.Empty(final_bones_override_list.Num());

	// First apply the IK constraints
	for (auto& cur_ik : internal_ik_map)
	{
		ComputeBonesIK(
			cur_ik.Value.first_bone_name,
			cur_ik.Value.second_bone_name,
			final_bones_override_list);
	}

	// Add in the current override list
	final_bones_override_list.Append(bones_override_list);

	if (final_bones_override_list.Num() == 0)
	{
		return;
	}

	auto base_xform = GetComponentToWorld();
	auto inv_base_xform = base_xform.Inverse();

	auto projectLocalLamda = [](const FTransform& inv_xform, const FVector& pos_in)
	{
		FVector ret_pos(0, 0, 0);
		ret_pos = inv_xform.TransformPosition(pos_in);

		return ret_pos;
	};

	auto scalarBlendLamda = [](float val1, float val2, float factor)
	{
		return ((1.0f - factor) * val1) + (factor * val2);
	};

	for (auto& cur_data : final_bones_override_list)
	{
		auto cur_bone_name = cur_data.bone_name;
		auto local_start_pos = projectLocalLamda(inv_base_xform, cur_data.start_pos);
		auto local_end_pos = projectLocalLamda(inv_base_xform, cur_data.end_pos);

		// Set to new positions based on bone name
		if (bones_map.Contains(cur_bone_name)) {
			auto set_bone = bones_map[cur_bone_name];
			auto set_start_pos = set_bone->getWorldStartPt();
			auto set_end_pos = set_bone->getWorldEndPt();

			set_start_pos.x = scalarBlendLamda(set_start_pos.x, local_start_pos.X, bones_override_blend_factor);
			set_start_pos.y = scalarBlendLamda(set_start_pos.y, local_start_pos.Z, bones_override_blend_factor);

			set_end_pos.x = scalarBlendLamda(set_end_pos.x, local_end_pos.X, bones_override_blend_factor);
			set_end_pos.y = scalarBlendLamda(set_end_pos.y, local_end_pos.Z, bones_override_blend_factor);

			set_bone->setWorldStartPt(set_start_pos);
			set_bone->setWorldEndPt(set_end_pos);
		}
	}
}

void 
UCreatureMeshComponent::SetBluePrintBonesIKConstraint(FCreatureBoneIK ik_data_in)
{
	auto cur_key = GetIkKey(ik_data_in.first_bone_name, ik_data_in.second_bone_name);
	if (cur_key.IsNone())
	{
		return;
	}

	if (internal_ik_map.Contains(cur_key) == false) {
		internal_ik_map.Add(cur_key, ik_data_in);
	}
	else {
		auto& cur_ik_data = internal_ik_map[cur_key];
		cur_ik_data.positive_angle = ik_data_in.positive_angle;
		cur_ik_data.target_pos = ik_data_in.target_pos;
	}
}

void
UCreatureMeshComponent::RemoveBluePrintBonesIKConstraint(FCreatureBoneIK ik_data_in)
{
	auto cur_key = GetIkKey(ik_data_in.first_bone_name, ik_data_in.second_bone_name);
	if (internal_ik_map.Contains(cur_key)) {
		internal_ik_map.Remove(cur_key);
	}
}

void UCreatureMeshComponent::SetBluePrintFrameCallbacks(const TArray<FCreatureFrameCallback>& callbacks_in)
{
	frame_callbacks = callbacks_in;
}

void UCreatureMeshComponent::ClearBluePrintFrameCallbacks()
{
	frame_callbacks.Empty();
}

void 
UCreatureMeshComponent::LoadBlueprintFramCallBacksAsset()
{
	if (creature_meta_asset)
	{
		TArray<FCreatureFrameCallback> set_callbacks;
		for (auto& cur_data : creature_meta_asset->GetMetaData()->anim_events_map)
		{
			FCreatureFrameCallback new_callback;
			new_callback.animClipName = FName(*cur_data.Key);

			auto& cur_value = cur_data.Value;
			for (auto& event_data : cur_value)
			{
				new_callback.name = FName(*event_data.Value);
				new_callback.frame = event_data.Key;

				set_callbacks.Add(new_callback);
			}
		}

		SetBluePrintFrameCallbacks(set_callbacks);
	}
}

void UCreatureMeshComponent::SetBluePrintRepeatFrameCallbacks(const TArray<FCreatureRepeatFrameCallback>& callbacks_in)
{
	repeat_frame_callbacks = callbacks_in;
}

void UCreatureMeshComponent::ClearBluePrintRepeatFrameCallbacks()
{
	repeat_frame_callbacks.Empty();
}

FName
UCreatureMeshComponent::GetIkKey(const FName& start_bone_name, const FName& end_bone_name) const
{
	return FName(*(start_bone_name.ToString() + end_bone_name.ToString()));
}

void
UCreatureMeshComponent::ComputeBonesIK(
	const FName& start_bone_name, 
	const FName& end_bone_name,
	TArray<FCreatureBoneOverride>& mod_list)
{
	FName ik_key = GetIkKey(start_bone_name, end_bone_name);
	if (ik_key.IsNone())
	{
		return;
	}

	if (internal_ik_map.Contains(ik_key) == false)
	{
		return;
	}

	FCreatureBoneIK& ik_data = internal_ik_map[ik_key];

	// Lambda functions
	auto fillBoneArrayLambda = [](const std::vector<meshBone *> bones, TArray<meshBone *>& write_array)
	{
		for (auto cur_bone : bones)
		{
			write_array.Add(cur_bone);
		}
	};

	auto computeLocalBasisLamda = [](
		const FVector& base_start_pt, 
		const FVector& base_end_pt)
	{
		std::pair<FVector, FVector> ret_basis;
		// Assume you are in local 2D space
		auto tangent_vec = base_end_pt - base_start_pt;
		auto normal_vec = FVector(-tangent_vec.Y, tangent_vec.X, tangent_vec.Z);

		tangent_vec.Normalize();
		normal_vec.Normalize();

		ret_basis.first = tangent_vec;
		ret_basis.second = normal_vec;

		return ret_basis;
	};

	auto computeLocalUVLamda = [](
		const std::pair<FVector, FVector>& local_basis,
		const FVector& base_start_pt,
		const FVector& calc_pt)
	{
		// Assume you are in local 2D space
		FVector ret_uv;
		auto rel_pt = calc_pt - base_start_pt;
		ret_uv.X = FVector::DotProduct(rel_pt, local_basis.first);
		ret_uv.Y = FVector::DotProduct(rel_pt, local_basis.second);

		return ret_uv;
	};

	auto projectLocalPtUVLamda = [](
		const FVector& local_uv,
		const std::pair<FVector, FVector>& dest_local_basis,
		const FVector& dst_start_pt
		)
	{
		// Assume you are in local 2D space
		return (local_uv.X * dest_local_basis.first) + (local_uv.Y * dest_local_basis.second) + dst_start_pt;
	};

	auto rotateVec2DLamda = [](const glm::vec4& vec_in, float angle)
	{
		auto ret_vec = vec_in;
		ret_vec.x = vec_in.x * cosf(angle) - vec_in.y * sinf(angle);
		ret_vec.y = vec_in.x * sinf(angle) + vec_in.y * cosf(angle);

		return ret_vec;
	};

	// This function is computed on local space, make sure the input Z is mapped to Y if in UE4 space
	// Calculate IK from origin, so transform points to local space first
	// Base point is at (0, 0)
	auto calc2BoneIKLambda = [](
		float& out_angle1, float& out_angle2,
		bool solve_pos_angle2,
		float length1,
		float length2,
		const FVector& target_pt
		)
	{
		/******************************************************************************
		Based off code from: 2008-2009 Ryan Juckett
		http://www.ryanjuckett.com/

		This software is provided 'as-is', without any express or implied
		warranty. In no event will the authors be held liable for any damages
		arising from the use of this software.

		Permission is granted to anyone to use this software for any purpose,
		including commercial applications, and to alter it and redistribute it
		freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.

		2. Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		distribution.
		******************************************************************************/

		// calculate IK from origin, so transform points to local space first
		// base point is at (0, 0)

		const float epsilon = 0.0001f;
		bool found_valid_solution = true;
		float target_dist_sqr = FVector::DistSquared(FVector(0, 0, 0), target_pt);

		// Compute a new value for angle2 along with its cosine
		float sin_angle2, cos_angle2;
		sin_angle2 = cos_angle2 = 0;

		float cos_angle2_denom = 2.0f * length1 * length2;
		if (cos_angle2_denom > epsilon)
		{
			cos_angle2 = (target_dist_sqr - (length1 * length1) - (length2 * length2))
				/ (cos_angle2_denom);

			// if our result is not in the legal cosine range, we can not find a
			// legal solution for the target
			if ((cos_angle2 < -1.0f) || (cos_angle2 > 1.0f)) {
				found_valid_solution = false;
			}

			// clamp our value into range so we can calculate the best
			// solution when there are no valid ones
			// clamp [-1, 1]
			if (cos_angle2 < -1.0f) {
				cos_angle2 = -1.0f;
			}
			else if (cos_angle2 > 1.0) {
				cos_angle2 = 1.0;
			}

			// compute a new value for angle2
			out_angle2 = acosf(cos_angle2);

			// adjust for the desired bend direction
			if (!solve_pos_angle2) {
				out_angle2 = -out_angle2;
			}

			// compute the sine of our angle
			sin_angle2 = sinf(out_angle2);
		}
		else {
			// At leaset one of the bones had a zero length. This means our
			// solvable domain is a circle around the origin with a radius
			// equal to the sum of our bone lengths.
			float total_len_sqr = (length1 + length2) * (length1 + length2);
			if (target_dist_sqr < (total_len_sqr - epsilon)
				|| (target_dist_sqr >(total_len_sqr + epsilon)))
			{
				found_valid_solution = false;
			}

			// Only the value of angle1 matters at this point. We can just
			// set angle2 to zero.
			out_angle2 = 0;
			cos_angle2 = 1.0f;
			sin_angle2 = 0;
		}

		// Compute the value of angle1 based on the sine and cosine of angle2
		float tri_adjacent = length1 + (length2 * cos_angle2);
		float tri_opposite = length2 * sin_angle2;

		float tan_y = (target_pt.Y * tri_adjacent) - (target_pt.X * tri_opposite);
		float tan_x = (target_pt.X * tri_adjacent) + (target_pt.Y * tri_opposite);

		out_angle1 = atan2f(tan_y, tan_x);

		return found_valid_solution;
	};

	auto getBoneFVectorLamda = [](
		const glm::vec4& vec_in
		)
	{
		return FVector(vec_in.x, vec_in.y, vec_in.z);
	};

	auto getBoneFVectorSwapLamda = [](
		const glm::vec4& vec_in
		)
	{
		return FVector(vec_in.x, vec_in.z, vec_in.y);
	};

	auto posePointFromBasisLambda = [&](
		const FVector& pt_in,
		const FVector& src_base_pt,
		const FVector& dst_base_pt,
		const std::pair<FVector, FVector>& src_basis,
		const std::pair<FVector, FVector>& dst_basis
		)
	{
		auto cur_uv = computeLocalUVLamda(src_basis, src_base_pt, pt_in);
		return projectLocalPtUVLamda(cur_uv, dst_basis, dst_base_pt);
	};

	auto poseBonesLambda = [&](
		TArray<meshBone *>& bones_in,
		const FVector& src_pt1,
		const FVector& src_pt2,
		const FVector& dst_pt1,
		const FVector& dst_pt2
		)
	{
		for (auto cur_bone : bones_in)
		{
			auto src_basis = computeLocalBasisLamda(src_pt1, src_pt2);
			auto dst_basis = computeLocalBasisLamda(dst_pt1, dst_pt2);

			auto new_start_pt = posePointFromBasisLambda(
				getBoneFVectorLamda(cur_bone->getWorldStartPt()),
				src_pt1,
				dst_pt1,
				src_basis,
				dst_basis);

			auto new_end_pt = posePointFromBasisLambda(
				getBoneFVectorLamda(cur_bone->getWorldEndPt()),
				src_pt1,
				dst_pt1,
				src_basis,
				dst_basis);

			internal_ik_bone_pts.Add(cur_bone->getKey(),
				std::make_pair(
					glm::vec4(new_start_pt.X, new_start_pt.Y, 0, 1.0f),
					glm::vec4(new_end_pt.X, new_end_pt.Y, 0, 1.0f)));
		}
	};

	auto overrideDataFromBoneLamda = [&](
		FTransform& world_xform,
		meshBone * bone
		)
	{
		FCreatureBoneOverride ret_data;
		ret_data.bone_name = bone->getKey();

		if (internal_ik_bone_pts.Contains(ret_data.bone_name))
		{
			auto pos1 = internal_ik_bone_pts[bone->getKey()].first;
			auto pos2 = internal_ik_bone_pts[bone->getKey()].second;

			ret_data.start_pos = world_xform.TransformPosition(getBoneFVectorSwapLamda(pos1));
			ret_data.end_pos = world_xform.TransformPosition(getBoneFVectorSwapLamda(pos2));
		}

		return ret_data;
	};

	if (ik_data.children_ready == false)
	{
		auto root_bone = creature_core.creature_manager->GetCreature()->GetRenderComposition()->getRootBone();

		auto first_children = 
			creature_core.getAllChildrenWithIgnore(ik_data.second_bone_name,
				root_bone->getChildByKey(ik_data.first_bone_name));
		auto second_children = 
			creature_core.getAllChildrenWithIgnore(ik_data.first_bone_name,
				root_bone->getChildByKey(ik_data.second_bone_name));

		fillBoneArrayLambda(first_children, ik_data.first_bone_children);
		fillBoneArrayLambda(second_children, ik_data.second_bone_children);

		ik_data.children_ready = true;
	}

	auto bones_map = creature_core.creature_manager->GetCreature()->GetRenderComposition()->getBonesMap();
	auto real_first_bone_name = ik_data.first_bone_name;
	auto real_second_bone_name = ik_data.second_bone_name;

	if ((bones_map.Contains(real_first_bone_name) == false)
		|| (bones_map.Contains(real_second_bone_name) == false))
	{
		return;
	}

	auto base_xform = GetComponentToWorld();
	auto inv_base_xform = base_xform.Inverse();

	// First compute IK solution
	// The function is computed on local space, make sure the input Z is mapped to Y if in UE4 space
	// Calculate IK from origin, so transform points to local space first
	// Base point is at (0, 0)
	auto local_ue4_target_pos = inv_base_xform.TransformPosition(ik_data.target_pos);
	FVector real_local_target_pos(local_ue4_target_pos.X, 
		local_ue4_target_pos.Z,
		0);

	auto start_bone_pt1 = bones_map[real_first_bone_name]->getWorldStartPt();
	auto start_bone_pt2 = bones_map[real_first_bone_name]->getWorldEndPt();

	auto end_bone_pt1 = bones_map[real_second_bone_name]->getWorldStartPt();
	auto end_bone_pt2 = bones_map[real_second_bone_name]->getWorldEndPt();

	auto cur_ik_start_pt = start_bone_pt1;
	auto cur_ik_mid_pt = (start_bone_pt2 + end_bone_pt1) * 0.5f;
	auto cur_ik_end_pt = end_bone_pt2;
	auto ik_length1 = glm::length(cur_ik_mid_pt - cur_ik_start_pt);
	auto ik_length2 = glm::length(cur_ik_end_pt - cur_ik_mid_pt);

	float ik_angle1 = 0, ik_angle2 = 0;
	FVector rel_target_pos(real_local_target_pos.X - cur_ik_start_pt.x, real_local_target_pos.Y - cur_ik_start_pt.y, 0);
	calc2BoneIKLambda(ik_angle1, ik_angle2, ik_data.positive_angle, ik_length1, ik_length2, rel_target_pos);

	auto new_ik_start_pt = getBoneFVectorLamda(cur_ik_start_pt);
	auto tmp_ik_mid_pt = rotateVec2DLamda(glm::vec4(ik_length1, 0, 0, 1), ik_angle1) + cur_ik_start_pt;
	auto tmp_ik_end_pt = rotateVec2DLamda(glm::vec4(ik_length2, 0, 0, 1), ik_angle2) + glm::vec4(ik_length1, 0, 0, 1);
	tmp_ik_end_pt = rotateVec2DLamda(tmp_ik_end_pt, ik_angle1) + cur_ik_start_pt;

	auto new_ik_mid_pt = getBoneFVectorLamda(tmp_ik_mid_pt);
	auto new_ik_end_pt = getBoneFVectorLamda(tmp_ik_end_pt);

	auto orig_start_pt = getBoneFVectorLamda(cur_ik_start_pt);
	auto orig_mid_pt = getBoneFVectorLamda(cur_ik_mid_pt);
	auto orig_end_pt = getBoneFVectorLamda(cur_ik_end_pt);

	// Now pose bones
	poseBonesLambda(ik_data.first_bone_children,
		orig_start_pt,
		orig_mid_pt,
		new_ik_start_pt,
		new_ik_mid_pt);

	poseBonesLambda(ik_data.second_bone_children,
		orig_mid_pt,
		orig_end_pt,
		new_ik_mid_pt,
		new_ik_end_pt);

	// Write out final bone array
	for (auto& cur_bone : ik_data.first_bone_children)
	{
		mod_list.Add(overrideDataFromBoneLamda(base_xform, cur_bone));
	}

	for (auto& cur_bone : ik_data.second_bone_children)
	{
		mod_list.Add(overrideDataFromBoneLamda(base_xform, cur_bone));
	}

}

void UCreatureMeshComponent::FreeBluePrintJSONMemory()
{
	CreatureCore::ClearAllDataPackets();
	UE_LOG(LogTemp, Warning, TEXT("UCreatureMeshComponent::FreeBluePrintJSONMemory() - Freed up JSON Memory Data."));
}

void UCreatureMeshComponent::ResetFrameCallbacks()
{
	for (auto& frame_callback : frame_callbacks)
	{
		frame_callback.resetCallback();
	}

	for (auto& frame_callback : repeat_frame_callbacks)
	{
		frame_callback.resetCallback(creature_core.creature_manager->getRunTime());
	}
}

void UCreatureMeshComponent::ProcessFrameCallbacks()
{
	auto cur_runtime = creature_core.creature_manager->getActualRunTime();
	for (auto& frame_callback : frame_callbacks)
	{
		if (frame_callback.animClipName == creature_core.creature_manager->GetActiveAnimationName()) {
			auto should_trigger = frame_callback.tryTrigger(cur_runtime);
			if (should_trigger && CreatureFrameCallbackEvent.IsBound())
			{
				CreatureFrameCallbackEvent.Broadcast(frame_callback.name);
			}
		}
	}

	for (auto& frame_callback : repeat_frame_callbacks)
	{
		if (frame_callback.animClipName == creature_core.creature_manager->GetActiveAnimationName()) {
			auto should_trigger = frame_callback.tryTrigger(cur_runtime);
			if (should_trigger && CreatureRepeatFrameCallbackEvent.IsBound())
			{
				CreatureRepeatFrameCallbackEvent.Broadcast(frame_callback.name);
			}
		}
	}
}

#if __clang__
#pragma clang diagnostic pop
#endif
