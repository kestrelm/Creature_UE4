#include "CustomProceduralMesh.h"
#include "CreatureMeshComponent.h"

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

FTransform 
UCreatureMeshComponent::GetBluePrintBoneXform(FString name_in, bool world_transform, float position_slide_factor)
{
	return creature_core.GetBluePrintBoneXform(name_in, world_transform, position_slide_factor, GetComponentToWorld());
}

void UCreatureMeshComponent::SetBluePrintAnimationLoop(bool flag_in)
{
	creature_core.SetBluePrintAnimationLoop(flag_in);
}

void 
UCreatureMeshComponent::SetBluePrintAnimationPlay(bool flag_in)
{
	creature_core.SetBluePrintAnimationPlay(flag_in);
}

void 
UCreatureMeshComponent::SetBluePrintAnimationPlayFromStart()
{
	creature_core.SetBluePrintAnimationPlayFromStart();
}

void UCreatureMeshComponent::SetBluePrintAnimationResetToStart()
{
	creature_core.SetBluePrintAnimationResetToStart();
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

CreatureCore& UCreatureMeshComponent::GetCore()
{
	return creature_core;
}

void UCreatureMeshComponent::InitStandardValues()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	animation_speed = 2.0f;
	smooth_transitions = false;
	bone_data_size = 0.01f;
	bone_data_length_factor = 0.02f;
	creature_bounds_scale = 15.0f;
	creature_debug_draw = false;
	creature_bounds_offset = FVector(0, 0, 0);
	region_overlap_z_delta = 0.01f;

	// Generate a single dummy triangle
	TArray<FProceduralMeshTriangle> triangles;
	GenerateTriangle(triangles);
	SetProceduralMeshTriangles(triangles);

}

void UCreatureMeshComponent::UpdateCoreValues()
{
	creature_core.creature_filename = creature_filename;
	creature_core.bone_data_size = bone_data_size;
	creature_core.bone_data_length_factor = bone_data_length_factor;
	creature_core.region_overlap_z_delta = region_overlap_z_delta;
}

void UCreatureMeshComponent::PrepareRenderData()
{
	RecreateRenderProxy(true);
	auto& load_triangles = creature_core.draw_triangles;
	SetProceduralMeshTriangles(load_triangles);
}

void UCreatureMeshComponent::RunTick(float DeltaTime)
{
	UpdateCoreValues();

	if (bHiddenInGame)
	{
		return;
	}

	TArray<FProceduralMeshTriangle>& write_triangles = GetProceduralTriangles();
	bool can_tick = creature_core.RunTick(DeltaTime * animation_speed, write_triangles);

	if (can_tick) {
		// Events
		bool announce_start = creature_core.GetAndClearShouldAnimStart();
		bool announce_end = creature_core.GetAndClearShouldAnimEnd();

		float cur_runtime = (creature_core.GetCreatureManager()->getActualRunTime());

		if (announce_start)
		{
			CreatureAnimationStartEvent.Broadcast(cur_runtime);
		}

		if (announce_end)
		{
			CreatureAnimationEndEvent.Broadcast(cur_runtime);
		}

		// Update Mesh
		SetBoundsScale(creature_bounds_scale);
		SetBoundsOffset(creature_bounds_offset);
		SetExtraXForm(GetComponentToWorld());

		ForceAnUpdate();

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

}

void UCreatureMeshComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RunTick(DeltaTime);
}

void UCreatureMeshComponent::OnRegister()
{
	Super::OnRegister();

	UpdateCoreValues();

	bool retval = creature_core.InitCreatureRender();
	creature_core.region_alpha_map.Empty();

	if (retval)
	{
		PrepareRenderData();
		RunTick(0.1f);
	}

}
