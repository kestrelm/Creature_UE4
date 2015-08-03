#include "CustomProceduralMesh.h"
#include "CreatureCollectionActor.h"

static std::string ConvertToString(FString str)
{
	std::string t = TCHAR_TO_UTF8(*str);
	return t;
}

ACreatureCollectionActor::ACreatureCollectionActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	is_looping = true;
	animation_speed = 2.0f;
	should_play = true;
	delay_set_animation = false;

	default_mesh = CreateDefaultSubobject<UCustomProceduralMeshComponent>(TEXT("CreatureCollectionActor"));
	RootComponent = default_mesh;
}

void ACreatureCollectionActor::AddBluePrintCollectionClipData(FString clipName, ACreatureActor * creatureActor, FString creatureActorClipName)
{
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

void ACreatureCollectionActor::SetBluePrintActiveClip(FString clipName)
{
	active_clip_name = ConvertToString(clipName);

	if (collection_clips.count(active_clip_name))
	{
		auto& cur_collection = collection_clips[active_clip_name];
		cur_collection.ref_index = 0;
		for (auto& cur_data : cur_collection.actor_sequence)
		{
			auto cur_actor = cur_data.first;
			auto cur_actor_clip = cur_data.second;

			if (cur_actor->GetCreatureManager())
			{
				cur_actor->SetActiveAnimation(cur_actor_clip);
				cur_actor->SetBluePrintAnimationResetToStart();
				delay_set_animation = false;
			}
			else {
				delay_set_animation = true;
			}
		}
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
	cur_actor->SetActiveAnimation(cur_data.second);
	cur_actor->SetBluePrintAnimationResetToStart();
}

void ACreatureCollectionActor::UpdateActorsVisibility(ACreatureCollectionClip& collection_data)
{
	int i = 0;
	for (auto& cur_data : collection_data.actor_sequence)
	{
		auto cur_actor = cur_data.first;
		if (i != collection_data.ref_index)
		{
			cur_actor->SetDriven(false);
			cur_actor->SetActorHiddenInGame(true);
		}
		else {
			cur_actor->SetDriven(true);
			cur_actor->SetActorHiddenInGame(false);
		}

		i++;
	}
}

void ACreatureCollectionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // Call parent class tick function  

	if (!should_play)
	{
		return;
	}

	if (delay_set_animation)
	{
		SetBluePrintActiveClip(FString(active_clip_name.c_str()));
	}

	float true_delta_time = DeltaTime * animation_speed;

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
