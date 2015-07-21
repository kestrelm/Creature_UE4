// UE4 Procedural Mesh Generation from the Epic Wiki (https://wiki.unrealengine.com/Procedural_Mesh_Generation)
//

#include "CustomProceduralMesh.h"
#include "CreatureActor.h"

static std::map<std::string, std::shared_ptr<CreatureModule::CreatureAnimation> > global_animations;
static std::map<std::string, std::shared_ptr<CreatureModule::CreatureLoadDataPacket> > global_load_data_packets;

static std::string GetAnimationToken(const std::string& filename_in, const std::string& name_in)
{
	return filename_in + std::string("_") + name_in;
}

static std::string ConvertToString(FString str)
{
	std::string t = TCHAR_TO_UTF8(*str);
	return t;
}

ACreatureActor::ACreatureActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UE_LOG(LogTemp, Warning, TEXT("ACreatureActor::Constructor Called()"));


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
	should_play = true;
	creature_bounds_scale = 15.0f;
	creature_debug_draw = false;
	creature_bounds_offset = FVector(0, 0, 0);
	region_overlap_z_delta = 0.01f;
	is_looping = true;
	play_start_done = false;
	play_end_done = false;


	creature_mesh = CreateDefaultSubobject<UCustomProceduralMeshComponent>(TEXT("CreatureActor"));
	RootComponent = creature_mesh;

	// Generate a single dummy triangle
	TArray<FProceduralMeshTriangle> triangles;
	GenerateTriangle(triangles);
	creature_mesh->SetProceduralMeshTriangles(triangles);

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


void ACreatureActor::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);

	bool retval = InitCreatureRender();
	region_alpha_map.Empty();

	if (retval)
	{
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

bool ACreatureActor::InitCreatureRender()
{
	/*
	if (!creature_mesh)
	{
		return true;
	}
	*/

	FString cur_creature_filename = creature_filename;
	bool does_exist = FPlatformFileManager::Get().GetPlatformFile().FileExists(*cur_creature_filename);
	if (!does_exist)
	{
		// see if it is in the content directory
		cur_creature_filename = FPaths::GameContentDir() + FString(TEXT("/")) + cur_creature_filename;
		does_exist = FPlatformFileManager::Get().GetPlatformFile().FileExists(*cur_creature_filename);
	}

	if (does_exist)
	{
		absolute_creature_filename = cur_creature_filename;
		auto load_filename = ConvertToString(cur_creature_filename);
		// try to load creature
		ACreatureActor::LoadDataPacket(load_filename);
		LoadCreature(load_filename);

		// try to load all animations
		auto all_animation_names = creature_manager->GetCreature()->GetAnimationNames();
		auto first_animation_name = all_animation_names[0];
		for (auto& cur_name : all_animation_names)
		{
			ACreatureActor::LoadAnimation(load_filename, cur_name);
			AddLoadedAnimation(load_filename, cur_name);
		}

		auto cur_str = ConvertToString(start_animation_name);
		for (auto& cur_name : all_animation_names)
		{
			if (cur_name == cur_str)
			{
				first_animation_name = cur_name;
				break;
			}
		}

		SetActiveAnimation(first_animation_name);

		if (smooth_transitions)
		{
			creature_manager->SetAutoBlending(true);
		}

		FillBoneData();

		return true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("ACreatureActor::BeginPlay() - ERROR! Could not load creature file: %s"), *creature_filename);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("ACreatureActor::BeginPlay() - ERROR! Could not load creature file: %s"), *creature_filename));
	}


	return false;
}

void ACreatureActor::FillBoneData()
{
	auto  render_composition = creature_manager->GetCreature()->GetRenderComposition();
	auto& bones_map = render_composition->getBonesMap();

	if (bone_data.Num() == 0)
	{
		bone_data.SetNum(bones_map.size());
	}

	int i = 0;
	for (auto& cur_data : bones_map)
	{
		bone_data[i].name = FString(cur_data.first.c_str());

		auto pt1 = cur_data.second->getWorldStartPt();
		auto pt2 = cur_data.second->getWorldEndPt();

		/* Id References
		const int x_id = 0;
		const int y_id = 2;
		const int z_id = 1;
		*/

		bone_data[i].point1 = FVector(pt1.x, pt1.y, pt1.z);
		bone_data[i].point2 = FVector(pt2.x, pt2.y, pt2.z);

		// figure out bone transform
		auto cur_bone = cur_data.second;
		auto bone_start_pt = pt1;
		auto bone_end_pt = pt2;

		auto bone_vec = bone_end_pt - bone_start_pt;
		auto bone_length = glm::length(bone_vec);
		auto bone_unit_vec = bone_vec / bone_length;

		// quick rotation by 90 degrees
		auto bone_unit_normal_vec = bone_unit_vec;
		bone_unit_normal_vec.x = -bone_unit_vec.y;
		bone_unit_normal_vec.y = bone_unit_vec.x;

		FVector bone_midpt = (bone_data[i].point1 + bone_data[i].point2) * 0.5f;
		FVector bone_axis_x(bone_unit_vec.x, bone_unit_vec.y, 0);
		FVector bone_axis_y(bone_unit_normal_vec.x, bone_unit_normal_vec.y, 0);
		FVector bone_axis_z(0, 0, 1);

		FTransform scaleXform(FVector(0, 0, 0));
		scaleXform.SetScale3D(FVector(bone_length * bone_data_length_factor, bone_data_size, bone_data_size));


		//std::swap(bone_midpt.Y, bone_midpt.Z);

		FTransform fixXform;
		fixXform.SetRotation(FQuat::MakeFromEuler(FVector(-90, 0, 0)));

		FTransform rotXform(bone_axis_x, bone_axis_y, bone_axis_z, FVector(0, 0, 0));

		FTransform posXform, posStartXform, posEndXform;
		posXform.SetTranslation(bone_midpt);
		posStartXform.SetTranslation(bone_data[i].point1);
		posEndXform.SetTranslation(bone_data[i].point2);

//		bone_data[i].xform = scaleXform * FTransform(bone_axis_x, bone_axis_y, bone_axis_z, bone_midpt);
		bone_data[i].xform = scaleXform  * rotXform  * posXform * fixXform;

		bone_data[i].startXform = scaleXform  * rotXform  * posStartXform * fixXform;
		bone_data[i].endXform = scaleXform  * rotXform  * posEndXform * fixXform;

		i++;
	}
}


CreatureModule::CreatureManager * 
ACreatureActor::GetCreatureManager()
{
	return creature_manager.get();
}

void ACreatureActor::BeginPlay()
{
	InitCreatureRender();
	region_alpha_map.Empty();
	Super::BeginPlay();
}

void ACreatureActor::LoadDataPacket(const std::string& filename_in)
{
	if (global_load_data_packets.count(filename_in) > 0)
	{
		// file already loaded, just return
		return;
	}

	std::shared_ptr<CreatureModule::CreatureLoadDataPacket> new_packet =
		std::make_shared<CreatureModule::CreatureLoadDataPacket>();

	bool is_zip = false;
	if (filename_in.substr(filename_in.find_last_of(".") + 1) == "zip") {
		is_zip = true;
	}

	if (is_zip)
	{
		// load zip archive
		CreatureModule::LoadCreatureZipJSONData(filename_in, *new_packet);
	}
	else {
		// load regular JSON
		CreatureModule::LoadCreatureJSONData(filename_in, *new_packet);
	}

	global_load_data_packets[filename_in] = new_packet;
}

void ACreatureActor::LoadAnimation(const std::string& filename_in, const std::string& name_in)
{
	auto cur_token = GetAnimationToken(filename_in, name_in);
	if (global_animations.count(cur_token) > 0)
	{
		// animation already exists, just return
		return;
	}

	auto load_data = global_load_data_packets[filename_in];

	std::shared_ptr<CreatureModule::CreatureAnimation> new_animation =
		std::make_shared<CreatureModule::CreatureAnimation>(*load_data, name_in);

	global_animations[cur_token] = new_animation;
}

void ACreatureActor::LoadCreature(const std::string& filename_in)
{
	if (!creature_mesh)
	{
		return;
	}


	auto load_data = global_load_data_packets[filename_in];

	std::shared_ptr<CreatureModule::Creature> new_creature =
		std::make_shared<CreatureModule::Creature>(*load_data);

	creature_manager = std::make_shared<CreatureModule::CreatureManager>(new_creature);

	draw_triangles.SetNum(creature_manager->GetCreature()->GetTotalNumIndices() / 3, true);

	creature_mesh->SetProceduralMeshTriangles(draw_triangles);
}

bool ACreatureActor::AddLoadedAnimation(const std::string& filename_in, const std::string& name_in)
{
	auto cur_token = GetAnimationToken(filename_in, name_in);
	if (global_animations.count(cur_token) > 0)
	{
		creature_manager->AddAnimation(global_animations[cur_token]);
		creature_manager->SetIsPlaying(true);
		creature_manager->SetShouldLoop(is_looping);
		return true;
	}
	else {
		std::cout << "ERROR! ACreatureActor::AddLoadedAnimation() Animation with filename: " << filename_in << " and name: " << name_in << " not loaded!" << std::endl;
	}

	return false;
}

void ACreatureActor::SetBluePrintActiveAnimation(FString name_in)
{
	auto cur_str = ConvertToString(name_in);
	SetActiveAnimation(cur_str);
}

void 
ACreatureActor::SetBluePrintAnimationLoop(bool flag_in)
{
	is_looping = flag_in;
	if (creature_manager) {
		creature_manager->SetShouldLoop(is_looping);
	}
}

void 
ACreatureActor::SetBluePrintAnimationPlay(bool flag_in)
{
	should_play = flag_in;
	play_start_done = false;
	play_end_done = false;
}

void 
ACreatureActor::SetBluePrintAnimationResetToStart()
{
	if (creature_manager) {
		creature_manager->ResetToStartTimes();
		float cur_runtime = (creature_manager->getActualRunTime());
		animation_frame = cur_runtime;

		creature_manager->Update(0.001f);
	}

	play_start_done = false;
	play_end_done = false;
}

void 
ACreatureActor::SetBluePrintAnimationPlayFromStart()
{
	SetBluePrintAnimationResetToStart();
	SetBluePrintAnimationPlay(true);
}

void 
ACreatureActor::SetBluePrintAnimationCustomTimeRange(FString name_in, int32 start_time, int32 end_time)
{
	auto cur_str = ConvertToString(name_in);
	auto all_animations = creature_manager->GetAllAnimations();
	if (all_animations.count(cur_str) > 0)
	{
		all_animations[cur_str]->setStartTime(start_time);
		all_animations[cur_str]->setEndTime(end_time);
	}
}

void ACreatureActor::SetActiveAnimation(const std::string& name_in)
{
	creature_manager->SetActiveAnimationName(name_in);
}

void ACreatureActor::SetBluePrintBlendActiveAnimation(FString name_in, float factor)
{
	auto cur_str = ConvertToString(name_in);
	SetAutoBlendActiveAnimation(cur_str, factor);
}

void 
ACreatureActor::SetBluePrintRegionAlpha(FString region_name_in, uint8 alpha_in)
{
	if (region_name_in.IsEmpty())
	{
		return;
	}

	region_alpha_map.Add(region_name_in, alpha_in);
}

void 
ACreatureActor::SetAutoBlendActiveAnimation(const std::string& name_in, float factor)
{
	auto all_animations = creature_manager->GetAllAnimations();

	if (all_animations.count(name_in) <= 0)
	{
		return;
	}

	if (factor < 0.001f)
	{
		factor = 0.001f;
	}
	else if (factor > 1.0f)
	{
		factor = 1.0f;
	}

	if (smooth_transitions == false)
	{
		smooth_transitions = true;
		creature_manager->SetAutoBlending(true);
	}

	creature_manager->AutoBlendTo(name_in, factor);
}

FTransform
ACreatureActor::GetBluePrintBoneXform(FString name_in, bool world_transform, float position_slide_factor)
{

	FTransform ret_xform;
	for (size_t i = 0; i < bone_data.Num(); i++)
	{
		if (bone_data[i].name == name_in)
		{
			ret_xform = bone_data[i].xform;
			float diff_slide_factor = fabs(position_slide_factor);
			const float diff_cutoff = 0.01f;
			if (diff_slide_factor > diff_cutoff)
			{
				// interpolate between start and end
				ret_xform.Blend(bone_data[i].startXform, bone_data[i].endXform, position_slide_factor + 0.5f);
			}


			if (world_transform)
			{
				FTransform xform = GetTransform();
				/*
				FVector world_location = xform.GetTranslation();
				ret_data.point1 = xform.TransformPosition(ret_data.point1);
				ret_data.point2 = xform.TransformPosition(ret_data.point2);
				*/
				//FMatrix no_scale = xform.ToMatrixNoScale();
				
				ret_xform = ret_xform * xform;
			}

			break;
		}
	}

	return ret_xform;
}

bool
ACreatureActor::IsBluePrintBonesCollide(FVector test_point, float bone_size)
{
	if (bone_size <= 0)
	{
		bone_size = 1.0f;
	}

	FTransform xform = GetTransform();
	FVector local_test_point = xform.InverseTransformPosition(test_point);
	auto  render_composition = creature_manager->GetCreature()->GetRenderComposition();
	auto& bones_map = render_composition->getBonesMap();
	
	glm::vec4 real_test_pt(local_test_point.X, local_test_point.Y, local_test_point.Z, 1.0f);
	for (auto cur_data : bones_map)
	{
		auto cur_bone = cur_data.second;
		auto bone_start_pt = cur_bone->getWorldStartPt();
		auto bone_end_pt = cur_bone->getWorldEndPt();

		auto bone_vec = bone_end_pt - bone_start_pt;
		auto bone_length = glm::length(bone_vec);
		auto bone_unit_vec = bone_vec / bone_length;

		auto rel_vec = real_test_pt - bone_start_pt;
		float proj_length_u = glm::dot(rel_vec, bone_unit_vec);
		if (proj_length_u >= 0 && proj_length_u <= bone_length)
		{
			// quick rotation by 90 degrees
			auto bone_unit_normal_vec = bone_unit_vec;
			bone_unit_normal_vec.x = -bone_unit_vec.y;
			bone_unit_normal_vec.y = bone_unit_vec.x;

			float proj_length_v = fabs(glm::dot(rel_vec, bone_unit_normal_vec));
			if (proj_length_v <= bone_size)
			{
				return true;
			}
		}
		
	}

	return false;
}

void ACreatureActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // Call parent class tick function  

	if (creature_manager)
	{
		ParseEvents(DeltaTime);

		if (should_play) {
			creature_manager->Update(DeltaTime * animation_speed);
		}

		UpdateCreatureRender();

		FillBoneData();

	}
}

float 
ACreatureActor::GetBluePrintAnimationFrame()
{
	return animation_frame;
}

void ACreatureActor::ParseEvents(float deltaTime)
{
	float cur_runtime = (creature_manager->getActualRunTime());
	animation_frame = cur_runtime;

	auto load_filename = ConvertToString(absolute_creature_filename);

	auto cur_animation_name = creature_manager->GetActiveAnimationName();

	auto cur_token = GetAnimationToken(load_filename, cur_animation_name);
	CreatureModule::CreatureAnimation * cur_animation = NULL;
	if (global_animations.count(cur_token) > 0)
	{
		cur_animation = global_animations[cur_token].get();
	}


	if (cur_animation)
	{
		int cur_start_time = cur_animation->getStartTime();
		int cur_end_time = cur_animation->getEndTime();

		float diff_val_start = fabs(cur_runtime - cur_start_time);
		const float cutoff = 0.01f;

		if ((diff_val_start <= cutoff)
			&& !is_looping 
			&& !play_start_done
			&& should_play)
		{
			play_start_done = true;
			this->BlueprintAnimationStart(cur_runtime);
			CreatureAnimationStartEvent.Broadcast(cur_runtime);
		}

		if ((cur_runtime + 1.0f >= cur_end_time)
			&& !is_looping 
			&& !play_end_done
			&& should_play)
		{
			play_end_done = true;
			should_play = false;
			this->BlueprintAnimationEnd(cur_runtime);
			CreatureAnimationEndEvent.Broadcast(cur_runtime);
		}
	}
}

void ACreatureActor::SetBluePrintRegionCustomOrder(TArray<FString> order_in)
{
	region_custom_order = order_in;
}

void ACreatureActor::ClearBluePrintRegionCustomOrder()
{
	region_custom_order.Empty();
}

void ACreatureActor::UpdateCreatureRender()
{
	auto cur_creature = creature_manager->GetCreature();
	int num_triangles = cur_creature->GetTotalNumIndices() / 3;
	glm::uint32 * cur_idx = cur_creature->GetGlobalIndices();
	glm::float32 * cur_pts = cur_creature->GetRenderPts();
	glm::float32 * cur_uvs = cur_creature->GetGlobalUvs();

	// Update depth per region
	std::vector<meshRenderRegion *>& cur_regions =
		cur_creature->GetRenderComposition()->getRegions();
	float region_z = 0.0f, delta_z = region_overlap_z_delta;

	if (region_custom_order.Num() != cur_regions.size())
	{
		// Normal update in default order
		for (auto& single_region : cur_regions)
		{
			glm::float32 * region_pts = cur_pts + (single_region->getStartPtIndex() * 3);
			for (size_t i = 0; i < single_region->getNumPts(); i++)
			{
				region_pts[2] = region_z;
				region_pts += 3;
			}

			region_z += delta_z;
		}
	}
	else {
		// Custom order update
		auto& regions_map = cur_creature->GetRenderComposition()->getRegionsMap();
		for (auto& custom_region_name : region_custom_order)
		{
			auto real_name = ConvertToString(custom_region_name);
			if (regions_map.count(real_name) > 0)
			{
				auto single_region = regions_map[real_name];
				glm::float32 * region_pts = cur_pts + (single_region->getStartPtIndex() * 3);
				for (size_t i = 0; i < single_region->getNumPts(); i++)
				{
					region_pts[2] = region_z;
					region_pts += 3;
				}

				region_z += delta_z;
			}
		}
	}

	// Build render triangles
	TArray<FProceduralMeshTriangle>& write_triangles = creature_mesh->GetProceduralTriangles();

	static const FColor White(255, 255, 255, 255);
	int cur_pt_idx = 0, cur_uv_idx = 0;
	const int x_id = 0;
	const int y_id = 2;
	const int z_id = 1;

	for (int i = 0; i < num_triangles; i++)
	{
		int real_idx_1 = cur_idx[0];
		int real_idx_2 = cur_idx[1];
		int real_idx_3 = cur_idx[2];

		FProceduralMeshTriangle triangle;

		cur_pt_idx = real_idx_1 * 3;
		cur_uv_idx = real_idx_1 * 2;
		triangle.Vertex0.Position.Set(cur_pts[cur_pt_idx + x_id], cur_pts[cur_pt_idx + y_id], cur_pts[cur_pt_idx + z_id]);
		triangle.Vertex0.Color = White;
		triangle.Vertex0.U = cur_uvs[cur_uv_idx];
		triangle.Vertex0.V = cur_uvs[cur_uv_idx + 1];

		cur_pt_idx = real_idx_2 * 3;
		cur_uv_idx = real_idx_2 * 2;
		triangle.Vertex1.Position.Set(cur_pts[cur_pt_idx + x_id], cur_pts[cur_pt_idx + y_id], cur_pts[cur_pt_idx + z_id]);
		triangle.Vertex1.Color = White;
		triangle.Vertex1.U = cur_uvs[cur_uv_idx];
		triangle.Vertex1.V = cur_uvs[cur_uv_idx + 1];

		cur_pt_idx = real_idx_3 * 3;
		cur_uv_idx = real_idx_3 * 2;
		triangle.Vertex2.Position.Set(cur_pts[cur_pt_idx + x_id], cur_pts[cur_pt_idx + y_id], cur_pts[cur_pt_idx + z_id]);
		triangle.Vertex2.Color = White;
		triangle.Vertex2.U = cur_uvs[cur_uv_idx];
		triangle.Vertex2.V = cur_uvs[cur_uv_idx + 1];

		write_triangles[i] = triangle;

		cur_idx += 3;
	}

	// process the render regions
	ProcessRenderRegions(write_triangles);

	//mesh->SetProceduralMeshTriangles(draw_triangles);
	creature_mesh->SetBoundsScale(creature_bounds_scale);
	creature_mesh->SetBoundsOffset(creature_bounds_offset);
	creature_mesh->SetExtraXForm(GetTransform());
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

void ACreatureActor::ProcessRenderRegions(TArray<FProceduralMeshTriangle>& draw_tris)
{
	auto cur_creature = creature_manager->GetCreature();
	auto& regions_map = cur_creature->GetRenderComposition()->getRegionsMap();
	int num_triangles = cur_creature->GetTotalNumIndices() / 3;

	// process alphas
	if (region_alphas.Num() <= 0)
	{
		region_alphas.Init(255, cur_creature->GetTotalNumPoints());
	}

	// fill up animation alphas
	for (auto& cur_region_pair : regions_map)
	{
		auto cur_region = cur_region_pair.second;
		auto start_pt_index = cur_region->getStartPtIndex();
		auto end_pt_index = cur_region->getEndPtIndex();
		auto cur_alpha = FMath::Clamp(cur_region->getOpacity() / 100.0f, 0.0f, 1.0f) * 255.0f;


		for (auto i = start_pt_index; i <= end_pt_index; i++)
		{
			region_alphas[i] = (uint8)cur_alpha;
		}
	}

	// user overwrite alphas
	if (region_alpha_map.Num() > 0)
	{
		// fill up the alphas for specific regions with alpha overwrites
		for (auto cur_iter : region_alpha_map)
		{
			auto cur_name = ConvertToString(cur_iter.Key);
			auto cur_alpha = cur_iter.Value;

			if (regions_map.count(cur_name) > 0)
			{
				meshRenderRegion * cur_region = regions_map[cur_name];
				auto start_pt_index = cur_region->getStartPtIndex();
				auto end_pt_index = cur_region->getEndPtIndex();

				for (auto i = start_pt_index; i <= end_pt_index; i++)
				{
					region_alphas[i] = cur_alpha;
 				}
			}
		}
	}

	// now write out alphas into render triangles
	glm::uint32 * cur_idx = cur_creature->GetGlobalIndices();
	for (int i = 0; i < num_triangles; i++)
	{
		int real_idx_1 = cur_idx[0];
		int real_idx_2 = cur_idx[1];
		int real_idx_3 = cur_idx[2];

		auto& cur_tri = draw_tris[i];
		auto set_alpha_1 = region_alphas[real_idx_1];
		auto set_alpha_2 = region_alphas[real_idx_2];
		auto set_alpha_3 = region_alphas[real_idx_3];

		cur_tri.Vertex0.Color = FColor(set_alpha_1, set_alpha_1, set_alpha_1, set_alpha_1);
		cur_tri.Vertex1.Color = FColor(set_alpha_2, set_alpha_2, set_alpha_1, set_alpha_2);
		cur_tri.Vertex2.Color = FColor(set_alpha_3, set_alpha_3, set_alpha_1, set_alpha_3);

		cur_idx += 3;
	}
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
