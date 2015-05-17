// UE4 Procedural Mesh Generation from the Epic Wiki (https://wiki.unrealengine.com/Procedural_Mesh_Generation)

#pragma once

#include "GameFramework/Actor.h"

#include "CreatureModule.h"
#include <map>

#include "ProceduralMeshComponent.h"
#include "CreatureActor.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class PROCEDURALMESH_API ACreatureActor : public AActor
{
	GENERATED_BODY()

protected:
	TArray<FProceduralMeshTriangle> draw_triangles;
	std::shared_ptr<CreatureModule::CreatureManager> creature_manager;

	void UpdateCreatureRender();

public:
	ACreatureActor();

	// Allow viewing/changing the Material ot the procedural Mesh in editor (if placed in a level at construction)
	UPROPERTY(VisibleAnywhere, Category=Materials)
	UProceduralMeshComponent* mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	FString creature_filename;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	float animation_speed;

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

	// Sets the an active animation by name
	void SetActiveAnimation(const std::string& name_in);

	// Update callback
	virtual void Tick(float DeltaTime) override;

	// Called on startup
	virtual void BeginPlay();

	void GenerateTriangle(TArray<FProceduralMeshTriangle>& OutTriangles);
};
