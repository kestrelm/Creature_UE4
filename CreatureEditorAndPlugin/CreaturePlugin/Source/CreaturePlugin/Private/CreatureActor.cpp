// UE4 Procedural Mesh Generation from the Epic Wiki (https://wiki.unrealengine.com/Procedural_Mesh_Generation)
//

#include "CreaturePluginPCH.h"
#include "CreatureActor.h"
#include <chrono>

ACreatureActor::ACreatureActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//UE_LOG(LogTemp, Warning, TEXT("ACreatureActor::Constructor Called()"));


	InitStandardValues();

	// Test Creature code
	/*
	std::string creature_json("C:\\Work\\CreatureDataExport2.CreaExport\\character_data.json");
	ACreatureActor::LoadDataPacket(creature_json);
	ACreatureActor::LoadAnimation(creature_json, "default");

	LoadCreature(creature_json);
	AddLoadedAnimation(creature_json, "default");
	SetActiveAnimation("default");
	creature_manager->Update(0.1f);
	UpdateCreatureRender();
	*/
}

void ACreatureActor::InitStandardValues()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	animation_speed = 1.0f;
	smooth_transitions = false;
	bone_data_size = 0.01f;
	bone_data_length_factor = 0.02f;
	creature_bounds_scale = 15.0f;
	creature_debug_draw = false;
	creature_bounds_offset = FVector(0, 0, 0);
	region_overlap_z_delta = 0.01f;

	creature_mesh = CreateDefaultSubobject<UCustomProceduralMeshComponent>(TEXT("CreatureActor"));
	RootComponent = creature_mesh;
	creature_mesh->SetTagString(GetName());

	// Generate a single dummy triangle
	/*
	TArray<FProceduralMeshTriangle> triangles;
	GenerateTriangle(triangles);
	creature_mesh->SetProceduralMeshTriangles(triangles);
	*/

	// Root collider capsule
	/*
	rootCollider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RootCollider"));
	rootCollider->SetRelativeRotation(FQuat(0, 1, 0, FMath::DegreesToRadians(90)));
	rootCollider->AttachParent = RootComponent;
	rootCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	rootCollider->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	*/

	/*
	CapsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	*/
}

void ACreatureActor::UpdateCoreValues()
{
	creature_core.creature_filename = creature_filename;
	creature_core.bone_data_size = bone_data_size;
	creature_core.bone_data_length_factor = bone_data_length_factor;
	creature_core.region_overlap_z_delta = region_overlap_z_delta;
}

void ACreatureActor::PrepareRenderData()
{
	creature_mesh->RecreateRenderProxy(true);
	creature_mesh->SetProceduralMeshTriData(creature_core.GetProcMeshData());
}

void ACreatureActor::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);

	UpdateCoreValues();

	if (!creature_mesh)
	{
		return;
	}

	bool retval = creature_core.InitCreatureRender();
	creature_core.region_alpha_map.Empty();

	if (retval)
	{
		PrepareRenderData();
		Tick(0.1f);
	}
}

#if WITH_EDITOR
/*
void ACreatureActor::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	bool retval = InitCreatureRender();

	if (retval)
	{
		Tick(0.1f);
	}

}
*/
#endif

void ACreatureActor::SetActorHiddenInGame(bool bNewHidden)
{
	creature_mesh->RecreateRenderProxy(true);
	Super::SetActorHiddenInGame(bNewHidden);
}

CreatureModule::CreatureManager * 
ACreatureActor::GetCreatureManager()
{
	return creature_core.GetCreatureManager();
}

void 
ACreatureActor::MakeBluePrintPointCache(FName name_in, int32 approximation_level)
{
	creature_core.MakeBluePrintPointCache(name_in, approximation_level);
}

void 
ACreatureActor::ClearBluePrintPointCache(FName name_in, int32 approximation_level)
{
	creature_core.ClearBluePrintPointCache(name_in, approximation_level);
}

void ACreatureActor::BeginPlay()
{
	UpdateCoreValues();

	creature_core.RunBeginPlay();
	PrepareRenderData();

	Super::BeginPlay();
}

void ACreatureActor::SetBluePrintActiveAnimation(FName name_in)
{
	creature_core.SetBluePrintActiveAnimation(name_in);
}

void 
ACreatureActor::SetBluePrintAnimationLoop(bool flag_in)
{
	creature_core.SetBluePrintAnimationLoop(flag_in);
}

bool
ACreatureActor::GetBluePrintAnimationLoop() const
{
	return creature_core.is_looping;
}

void 
ACreatureActor::SetBluePrintAnimationPlay(bool flag_in)
{
	creature_core.SetBluePrintAnimationPlay(flag_in);
}

void 
ACreatureActor::SetBluePrintAnimationResetToStart()
{
	creature_core.SetBluePrintAnimationResetToStart();
}

void 
ACreatureActor::SetBluePrintAnimationPlayFromStart()
{
	creature_core.SetBluePrintAnimationPlayFromStart();
}

void 
ACreatureActor::SetBluePrintAnimationCustomTimeRange(FName name_in, int32 start_time, int32 end_time)
{
	creature_core.SetBluePrintAnimationCustomTimeRange(name_in, start_time, end_time);
}

void ACreatureActor::SetBluePrintBlendActiveAnimation(FName name_in, float factor)
{
	creature_core.SetBluePrintBlendActiveAnimation(name_in, factor);
}

void 
ACreatureActor::SetBluePrintRegionAlpha(FName region_name_in, uint8 alpha_in)
{
	creature_core.SetBluePrintRegionAlpha(region_name_in, alpha_in);
}

FTransform
ACreatureActor::GetBluePrintBoneXform(FName name_in, bool world_transform, float position_slide_factor)
{
	return creature_core.GetBluePrintBoneXform(name_in, world_transform, position_slide_factor, GetTransform());
}

bool
ACreatureActor::IsBluePrintBonesCollide(FVector test_point, float bone_size)
{
	return creature_core.IsBluePrintBonesCollide(test_point, bone_size, GetTransform());
}

void ACreatureActor::SetIsDisabled(bool flag_in)
{
	creature_core.SetIsDisabled(flag_in);
}

void ACreatureActor::SetDriven(bool flag_in)
{
	creature_core.SetDriven(flag_in);
}

CreatureCore& 
ACreatureActor::GetCore()
{
	return creature_core;
}

void ACreatureActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // Call parent class tick function  
	UpdateCoreValues();

	if (bHidden)
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

		// Update Mesh
		creature_mesh->SetBoundsScale(creature_bounds_scale);
		creature_mesh->SetBoundsOffset(creature_bounds_offset);

		creature_mesh->SetTagString(GetName());
		creature_mesh->ForceAnUpdate();

		// Debug

		if (creature_debug_draw) {
			FSphere debugSphere = creature_mesh->GetDebugBoundsSphere();
			DrawDebugSphere(
				GetWorld(),
				debugSphere.Center,
				debugSphere.W,
				32,
				FColor(255, 0, 0)
				);

			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Sphere pos is: (%f, %f, %f)"), debugSphere.Center.X, debugSphere.Center.Y, debugSphere.Center.Z));
			FTransform wTransform = GetTransform();
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Walk pos is: (%f, %f, %f)"), wTransform.GetLocation().X,
				wTransform.GetLocation().Y,
				wTransform.GetLocation().Z));
		}
	}
}

float 
ACreatureActor::GetBluePrintAnimationFrame()
{
	return creature_core.GetBluePrintAnimationFrame();
}

void ACreatureActor::SetBluePrintRegionCustomOrder(TArray<FName> order_in)
{
	creature_core.SetBluePrintRegionCustomOrder(order_in);
}

void ACreatureActor::ClearBluePrintRegionCustomOrder()
{
	creature_core.ClearBluePrintRegionCustomOrder();
}

// Generate a single horizontal triangle counterclockwise to point up (one face, visible only from the top, not from the bottom)
void ACreatureActor::GenerateTriangle(TArray<FProceduralMeshTriangle>& OutTriangles)
{
	FProceduralMeshTriangle triangle;
	triangle.Vertex0.Position.Set(  0.f, -10.f, 0.f);
	triangle.Vertex1.Position.Set(  0.f,  10.f, 0.f);
	triangle.Vertex2.Position.Set(10.f,  0.f,  0.f);
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
