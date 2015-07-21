#pragma once

#include "GameFramework/Actor.h"
#include "CreatureActor.h"
#include "CreatureModule.h"
#include <map>
#include <unordered_map>

#include "CustomProceduralMeshComponent.h"
#include "CreatureSwitchItemActor.generated.h"

/**
 * 
 */

struct ACreatureSwitchData
{
	ACreatureSwitchData()
	{
		x = 0;
		y = 0;
		width = 0;
		height = 0;
		canvas_width = 0;
		canvas_height = 0;
	}

	ACreatureSwitchData(int32 x_in, int32 y_in, int32 width_in, int32 height_in,
		int32 canvas_width_in, int32 canvas_height_in)
	{
		x = x_in;
		y = y_in;
		width = width_in;
		height = height_in;
		canvas_width = canvas_width_in;
		canvas_height = canvas_height_in;

		computeUVData();
	}

	void computeUVData()
	{
		u = (float)x / (float)canvas_width;
		v = (float)y / (float)canvas_height;

		u_width = (float)width / (float)canvas_width;
		v_height = (float)height / (float)canvas_height;
	}

	int32 x, y, width, height;
	int32 canvas_width, canvas_height;
	float u, v, u_width, v_height;
};


UCLASS(Blueprintable)
class ACreatureSwitchItemActor : public AActor
{
	GENERATED_UCLASS_BODY()

protected:
	TArray<FProceduralMeshTriangle> draw_triangles;
	std::unordered_map<std::string, ACreatureSwitchData> switch_table;
	meshRenderRegion * real_switch_region;
	bool switch_init_done;
	FVector switch_min_uv, switch_max_uv;
	float switch_uv_width, switch_uv_height;
	FString switch_to_name;

	void InitSwitchRenderData();

	void InitRealSwitchRegion();

	void TransformTextureSpace(ACreatureSwitchData& switch_data, float u_in, float v_in, float& u_out, float& v_out);

	void UpdateSwitchRender();

public:
	ACreatureSwitchItemActor();

	// Allow viewing/changing the Material ot the procedural Mesh in editor (if placed in a level at construction)
	UPROPERTY(VisibleAnywhere, Category=Materials)
	UCustomProceduralMeshComponent* switch_mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	ACreatureActor *creature_actor;

	// The region that you want to switch to occur
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
	FString creature_switch_region;

	// Blueprints function that adds a new switch item with a name identifier e.g "Cloak1", "Cloak2" etc. Remember to set the Canvas Width and Canvas Height to the size of your entire switch texture containing the switch items
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void AddBluePrintSwitchData(FString name_in, int32 x_in, int32 y_in, int32 width_in, int32 height_in, int32 canvas_width_in, int32 canvas_height_in);

	// Blueprints function to set the item to switch to given the name of the item
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SwitchBluePrintData(FString name_in);

	virtual void OnConstruction(const FTransform & Transform);	

	// Update callback
	virtual void Tick(float DeltaTime) override;

	// Called on startup
	virtual void BeginPlay();

	void GenerateTriangle(TArray<FProceduralMeshTriangle>& OutTriangles);
};