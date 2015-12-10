
#include "CreaturePluginPCH.h"
#include "CreatureCollectionActor.h"

static std::string ConvertToString(FString str)
{
	std::string t = TCHAR_TO_UTF8(*str);
	return t;
}

static void GenerateTriangle(TArray<FProceduralMeshTriangle>& OutTriangles)
{
	FProceduralMeshTriangle triangle;
	triangle.Vertex0.Position.Set(0.f, -1.f, 0.f);
	triangle.Vertex1.Position.Set(0.f, -0.9, 0.f);
	triangle.Vertex2.Position.Set(0.1f, 0.f, 0.f);
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


ACreatureCollectionActor::ACreatureCollectionActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	is_looping = true;
	animation_speed = 2.0f;
	should_play = true;
	hide_all_actors = false;

	default_mesh = CreateDefaultSubobject<UCustomProceduralMeshComponent>(TEXT("CreatureCollectionActor"));
	RootComponent = default_mesh;

	// Generate a single dummy triangle
	/*
	TArray<FProceduralMeshTriangle> triangles;
	GenerateTriangle(triangles);
	default_mesh->SetProceduralMeshTriangles(triangles);
	*/
}

void ACreatureCollectionActor::AddBluePrintCollectionClipData(FString clipName, ACreatureActor * creatureActor, FString creatureActorClipName)
{
	if (creatureActor == NULL)
	{
		return;
	}

	std::string new_clip_name = ConvertToString(clipName);
	std::string real_actor_clip_name = ConvertToString(creatureActorClipName);

	if (collection_clips.count(new_clip_name) < 0)
	{
		collection_clips[new_clip_name] = ACreatureCollectionClip();
	}

	creatureActor->SetIsDisabled(true);
	creatureActor->SetDriven(false);
	creatureActor->SetBluePrintAnimationLoop(false);
	ACreatureCollectionClip& cur_collection_clip = collection_clips[new_clip_name];
	cur_collection_clip.actor_sequence.push_back(std::make_pair(creatureActor, real_actor_clip_name));
}

void ACreatureCollectionActor::SetBluePrintShouldPlay(bool flag_in)
{
	should_play = flag_in;
}

void ACreatureCollectionActor::SetBluePrintIsLooping(bool flag_in)
{
	is_looping = flag_in;
}

void 
ACreatureCollectionActor::SetBluePrintHideAllActors(bool flag_in)
{
	hide_all_actors = flag_in;
}

void ACreatureCollectionActor::SetBluePrintActiveClip(FString clipName)
{
	if (AreAllActorsReady() == false)
	{
		delay_set_clip_name = ConvertToString(clipName);
		return;
	}

	active_clip_name = ConvertToString(clipName);

	if (collection_clips.count(active_clip_name))
	{
		auto& cur_collection = collection_clips[active_clip_name];
		cur_collection.ref_index = 0;
		UpdateActorAnimationToStart(cur_collection);
	}
}

FTransform 
ACreatureCollectionActor::GetBluePrintBoneXform(FString name_in, bool world_transform, float position_slide_factor)
{
	if (collection_clips.count(active_clip_name))
	{
		auto& cur_collection = collection_clips[active_clip_name];
		int& ref_index = cur_collection.ref_index;
		auto& cur_data = cur_collection.actor_sequence[ref_index];

		auto cur_actor = cur_data.first;
		return cur_actor->GetBluePrintBoneXform(name_in, world_transform, position_slide_factor);
	}

	return FTransform();
}

void ACreatureCollectionActor::UpdateActorAnimationToStart(ACreatureCollectionClip& collection_data)
{
	int& ref_index = collection_data.ref_index;
	auto& cur_data = collection_data.actor_sequence[ref_index];
	auto cur_actor = cur_data.first;

	cur_actor = cur_data.first;
	cur_actor->SetBluePrintActiveAnimation(FString(cur_data.second.c_str()));
	cur_actor->SetBluePrintAnimationResetToStart();
}

void ACreatureCollectionActor::HideAllActors(ACreatureCollectionClip& collection_data, ACreatureActor * exceptActor)
{
	for (auto& cur_data : collection_data.actor_sequence)
	{
		auto cur_actor = cur_data.first;

		if (exceptActor != cur_actor) {
			cur_actor->SetDriven(false);
			cur_actor->SetActorHiddenInGame(true);
		}
	}
}

void ACreatureCollectionActor::UpdateActorsVisibility(ACreatureCollectionClip& collection_data)
{
	int i = 0;
	ACreatureActor * active_actor = GetActiveActor();

	for (auto& cur_data : collection_data.actor_sequence)
	{
		auto cur_actor = cur_data.first;
		if ((i != collection_data.ref_index)
			&& (cur_actor != active_actor))
		{
			cur_actor->SetDriven(false);
			cur_actor->SetActorHiddenInGame(true);
		}

		i++;
	}

	if (active_actor) {
		active_actor->SetDriven(true);
		active_actor->SetActorHiddenInGame(false);
	}
}

bool 
ACreatureCollectionActor::AreAllActorsReady() const
{
	for (auto& collection_data : collection_clips)
	{
		
		for (auto& cur_data : collection_data.second.actor_sequence)
		{
			auto cur_actor = cur_data.first;
			if (cur_actor->GetCore().GetIsReadyPlay() == false)
			{
				return false;
			}
		}
	}

	return true;
}

ACreatureActor * 
ACreatureCollectionActor::GetActiveActor()
{
	if (collection_clips.count(active_clip_name))
	{
		auto& cur_collection = collection_clips[active_clip_name];
		int& ref_index = cur_collection.ref_index;
		auto& cur_data = cur_collection.actor_sequence[ref_index];

		auto cur_actor = cur_data.first;
		return cur_actor;
	}

	return NULL;
}

void ACreatureCollectionActor::BeginPlay()
{
	Super::BeginPlay();
}


void ACreatureCollectionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // Call parent class tick function  

	if (!should_play)
	{
		return;
	}

	if(AreAllActorsReady() == false)
	{
		return;
	}
	else {
		if (delay_set_clip_name.empty() == false)
		{
			SetBluePrintActiveClip(FString(delay_set_clip_name.c_str()));
			delay_set_clip_name = std::string("");
			return;
		}
	}

	float true_delta_time = DeltaTime * animation_speed;

	// hide other actors in other clips
	for (auto& cur_collection_data : collection_clips)
	{
		if (cur_collection_data.first != active_clip_name) {
			HideAllActors(cur_collection_data.second, GetActiveActor());
		}
	}

	// see if we need to hide everybody and return
	if (hide_all_actors)
	{
		for (auto& cur_collection_data : collection_clips)
		{
			HideAllActors(cur_collection_data.second, nullptr);
		}

		return;
	}

	// process active clip
	if (collection_clips.count(active_clip_name))
	{
		auto& cur_collection = collection_clips[active_clip_name];
		int& ref_index = cur_collection.ref_index;
		auto& cur_data = cur_collection.actor_sequence[ref_index];

		auto cur_actor = cur_data.first;
		auto cur_manager = cur_actor->GetCreatureManager();
		cur_manager->Update(true_delta_time);

		// update visiblity
		UpdateActorsVisibility(cur_collection);

		// check to see if we are at the end of the current actor animation
		auto& all_actor_animations = cur_manager->GetAllAnimations();
		bool is_actor_animation_done = false;
		bool is_at_sequence_end = false;
		if (all_actor_animations.count(cur_data.second) > 0)
		{
			auto& actor_animation = all_actor_animations.at(cur_data.second);
			float cur_runtime = (cur_manager->getActualRunTime());
			if (cur_runtime + true_delta_time >= actor_animation->getEndTime())
			{
				is_actor_animation_done = true;

				if (ref_index + 1 < cur_collection.actor_sequence.size())
				{
					// switch to new actor animation
					ref_index++;
					UpdateActorAnimationToStart(cur_collection);
				}
				else {
					is_at_sequence_end = true;
					CreatureAnimationEndEvent.Broadcast(cur_runtime);
				}
			}
		}

		if (is_at_sequence_end && is_looping)
		{
			ref_index = 0;
			UpdateActorAnimationToStart(cur_collection);
		}
	}
}
