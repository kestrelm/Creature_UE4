// UE4 Procedural Mesh Generation from the Epic Wiki (https://wiki.unrealengine.com/Procedural_Mesh_Generation)

#pragma once

#include <memory>
#include "GameFramework/Actor.h"
#include "CreatureModule.h"
#include <map>

#include "CustomProceduralMeshComponent.h"
#include "CreatureCore.h"
#include "CreatureActor.generated.h"

/**
 * 
 */

// Blueprint event delegates event declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreatureAnimationStartEvent, float, frame);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreatureAnimationEndEvent, float, frame);

UCLASS(Blueprintable)
class ACreatureActor : public AActor
{
	GENERATED_UCLASS_BODY()

protected:
	CreatureCore creature_core;

	void InitStandardValues();

	void UpdateCoreValues();

	void PrepareRenderData();

public:
	ACreatureActor();

	// Allow viewing/changing the Material ot the procedural Mesh in editor (if placed in a level at construction)
	UPROPERTY(VisibleAnywhere, Category=Materials)
	UCustomProceduralMeshComponent* creature_mesh;

	/* Removed, put this in if you need a default capsule root collider
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Collision)
	UCapsuleComponent * rootCollider;
	*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	FString creature_filename;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	float animation_speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	float bone_data_size;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	float creature_bounds_scale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	FVector creature_bounds_offset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	bool creature_debug_draw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	float bone_data_length_factor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	float region_overlap_z_delta;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	bool smooth_transitions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	FString start_animation_name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Creature")
	float animation_frame;

	UPROPERTY(BlueprintAssignable, Category = "Components|Creature")
	FCreatureAnimationStartEvent CreatureAnimationStartEvent;

	UPROPERTY(BlueprintAssignable, Category = "Components|Creature")
	FCreatureAnimationEndEvent CreatureAnimationEndEvent;

#if WITH_EDITOR
	//virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent);
#endif

	virtual void OnConstruction(const FTransform & Transform);

	// Returns the CreatureManager associated with this actor
	CreatureModule::CreatureManager * GetCreatureManager();

	// Blueprint version of setting the active animation name
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintActiveAnimation(FString name_in);

	// Blueprint version of setting the blended active animation name
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintBlendActiveAnimation(FString name_in, float factor);

	// Blueprint version of setting a custom time range for a given animation
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintAnimationCustomTimeRange(FString name_in, int32 start_time, int32 end_time);

	// Blueprint function to create a point cache for the creature character. This speeds up the playback performance.
	// A small amount of time will be spent precomputing the point cache. You can reduce this time by increasing the approximation level.
	// name_in is the name of the animation to cache, approximation_level is the approximation level. The higher the approximation level
	// the faster the cache generation but lower the quality. 1 means no approximation, with 10 being the maximum value allowed.
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void MakeBluePrintPointCache(FString name_in, int32 approximation_level);

	// Blueprint function to clear the point cache of a given animation
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void ClearBluePrintPointCache(FString name_in, int32 approximation_level);

	// Blueprint function that returns the transform given a bone name, position_slide_factor
	// determines how far left or right the transform is placed. The default value of 0 places it
	// in the center of the bone, positve values places it to the right, negative to the left
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	FTransform GetBluePrintBoneXform(FString name_in, bool world_transform, float position_slide_factor);

	// BLueprint function that returns whether a given input point is colliding with any of the bones
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	bool IsBluePrintBonesCollide(FVector test_point, float bone_size);

	// Blueprint function that decides whether the animation will loop or not
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintAnimationLoop(bool flag_in);

	// Blueprint function that returns whether the animation is looping or not
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	bool GetBluePrintAnimationLoop() const;

	// Blueprint function that decides whether to play the animation or not
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintAnimationPlay(bool flag_in);

	// Blueprint function that plays the animation from the start
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintAnimationPlayFromStart();

	// Blueprint function that resets the animation to the start time
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintAnimationResetToStart();

	// Blueprint function that returns the current animation frame
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")	
	float GetBluePrintAnimationFrame();

	// Blueprint function that sets the alpha(opacity value) of a region
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintRegionAlpha(FString region_name_in, uint8 alpha_in);

	// Blueprint function that sets up a custom z order for the various regions
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintRegionCustomOrder(TArray<FString> order_in);

	// Blueprint function that clears the custom z order for the various regions
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void ClearBluePrintRegionCustomOrder();

	// Blueprint function that turns on/turns off internal updates of this object
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetIsDisabled(bool flag_in);

	void SetDriven(bool flag_in);

	CreatureCore& GetCore();

	// Update callback
	virtual void Tick(float DeltaTime) override;

	// Called on startup
	virtual void BeginPlay();

	UFUNCTION(BlueprintCallable, Category = "Rendering", meta = (DisplayName = "Set Actor Hidden In Game", Keywords = "Visible Hidden Show Hide"))
	virtual void SetActorHiddenInGame(bool bNewHidden) override;

	void GenerateTriangle(TArray<FProceduralMeshTriangle>& OutTriangles);
};
