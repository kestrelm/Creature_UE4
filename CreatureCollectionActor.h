#pragma once

#include "GameFramework/Actor.h"
#include "CreatureActor.h"
#include "CreatureModule.h"
#include <unordered_map>
#include <vector>
#include <string>

#include "CustomProceduralMeshComponent.h"
#include "CreatureCollectionActor.generated.h"

struct ACreatureCollectionClip
{
	ACreatureCollectionClip()
	{
		ref_index = 0;
	}

	std::vector<std::pair<ACreatureActor *, std::string> > actor_sequence;
	int32 ref_index;
};

// Blueprint event delegates event declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreatureAnimationCollectionEndEvent, float, frame);

UCLASS(Blueprintable)
class ACreatureCollectionActor : public AActor
{
	GENERATED_UCLASS_BODY()

protected:
	std::unordered_map <std::string, ACreatureCollectionClip> collection_clips;
	bool is_looping;
	std::string active_clip_name;
	bool should_play;
	std::string delay_set_clip_name;

	void UpdateActorAnimationToStart(ACreatureCollectionClip& collection_data);

	void UpdateActorsVisibility(ACreatureCollectionClip& collection_data);

	bool AreAllActorsReady() const;

public:
	ACreatureCollectionActor();

	// Allow viewing/changing the Material ot the procedural Mesh in editor (if placed in a level at construction)
	UPROPERTY(VisibleAnywhere, Category = Materials)
	UCustomProceduralMeshComponent* default_mesh;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	float animation_speed;

	// Blueprints function to add a new CreatureActor with its associated animation clip into your new combined clip called clipName
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void AddBluePrintCollectionClipData(FString clipName, ACreatureActor * creatureActor, FString creatureActorClipName);

	// Blueprints function setting whether this plays or not
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintShouldPlay(bool flag_in);

	// Blueprints function setting whether the playback loops or not
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintIsLooping(bool flag_in);

	// Blueprints function that sets the active collection clip 
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintActiveClip(FString clipName);

	// Blueprint function that returns the transform given a bone name, position_slide_factor
	// determines how far left or right the transform is placed. The default value of 0 places it
	// in the center of the bone, positve values places it to the right, negative to the left
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	FTransform GetBluePrintBoneXform(FString name_in, bool world_transform, float position_slide_factor);

	UPROPERTY(BlueprintAssignable, Category = "Components|Creature")
	FCreatureAnimationCollectionEndEvent CreatureAnimationEndEvent;

	// Called on startup
	virtual void BeginPlay();

	// Update callback
	virtual void Tick(float DeltaTime) override;

};