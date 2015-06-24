// UE4 Procedural Mesh Generation from the Epic Wiki (https://wiki.unrealengine.com/Procedural_Mesh_Generation)

#pragma once

#include "GameFramework/Actor.h"

#include "CreatureModule.h"
#include <map>

#include "CustomProceduralMeshComponent.h"
#include "CreatureActor.generated.h"

/**
 * 
 */

struct FCreatureBoneData
{
	FVector point1;
	FVector point2;
	FTransform xform;
	FString name;
};

UCLASS(Blueprintable)
class ACreatureActor : public AActor
{
	GENERATED_UCLASS_BODY()

protected:
	TArray<FProceduralMeshTriangle> draw_triangles;

	std::shared_ptr<CreatureModule::CreatureManager> creature_manager;

	TArray<FCreatureBoneData> bone_data;

	FString absolute_creature_filename;

	bool should_play;

	void UpdateCreatureRender();

	bool InitCreatureRender();

	void FillBoneData();

	void ParseEvents(float deltaTime);

	void InitStandardValues();

public:
	ACreatureActor();

	// Allow viewing/changing the Material ot the procedural Mesh in editor (if placed in a level at construction)
	UPROPERTY(VisibleAnywhere, Category=Materials)
	UCustomProceduralMeshComponent* creature_mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Collision)
	UCapsuleComponent * rootCollider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	FString creature_filename;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	float animation_speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	float bone_data_size;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	float bone_data_length_factor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	bool smooth_transitions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	FString start_animation_name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Creature")
	float animation_frame;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent);
#endif

	virtual void OnConstruction(const FTransform & Transform);

	// Loads a data packet from a file
	static void LoadDataPacket(const std::string& filename_in);

	// Loads an animation from a file
	static void LoadAnimation(const std::string& filename_in, const std::string& name_in);

	// Loads the creature character from a file
	void LoadCreature(const std::string& filename_in);

	// Adds a loaded animation onto the creature character
	bool AddLoadedAnimation(const std::string& filename_in, const std::string& name_in);

	// Blueprint version of setting the active animation name
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintActiveAnimation(FString name_in);

	// Blueprint version of setting the blended active animation name
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintBlendActiveAnimation(FString name_in, float factor);

	// Blueprint version of setting a custom time range for a given animation
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintAnimationCustomTimeRange(FString name_in, int32 start_time, int32 end_time);

	// Blueprint function that returns the transform given a bone name
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	FTransform GetBluePrintBoneXform(FString name_in, bool world_transform);

	// BLueprint function that returns whether a given input point is colliding with any of the bones
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	bool IsBluePrintBonesCollide(FVector test_point, float bone_size);

	// Blueprint function that decides whether the animation will loop or not
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintAnimationLoop(bool flag_in);

	// Blueprint function that decides whether to play the animation or not
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintAnimationPlay(bool flag_in);

	UFUNCTION(BlueprintImplementableEvent, Category = "Components|Creature", meta = (DisplayName = "Calback when animation has started"))
	virtual void BlueprintAnimationStart(float frame_in);

	UFUNCTION(BlueprintImplementableEvent, Category = "Components|Creature", meta = (DisplayName = "Calback when animation has ended"))
	virtual void BlueprintAnimationEnd(float frame_in);


	// Sets the an active animation by name
	void SetActiveAnimation(const std::string& name_in);

	// Sets the active animation by smoothly blending, factor is a range of ( 0 < factor < 1 )
	void SetAutoBlendActiveAnimation(const std::string& name_in, float factor);

	

	// Update callback
	virtual void Tick(float DeltaTime) override;

	// Called on startup
	virtual void BeginPlay();

	void GenerateTriangle(TArray<FProceduralMeshTriangle>& OutTriangles);
};
