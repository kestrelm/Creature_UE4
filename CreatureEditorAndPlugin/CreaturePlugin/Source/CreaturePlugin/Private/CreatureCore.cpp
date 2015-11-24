#include "CustomProceduralMesh.h"
#include "CreatureCore.h"

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

typedef std::chrono::high_resolution_clock Time;
static auto profileTimeStart = Time::now();
static auto profileTimeEnd = Time::now();

static void StartProfileTimer()
{
	typedef std::chrono::milliseconds ms;
	typedef std::chrono::duration<float> fsec;

	profileTimeStart = Time::now();
}

static float StopProfileTimer()
{
	typedef std::chrono::milliseconds ms;
	typedef std::chrono::duration<float> fsec;

	profileTimeEnd = Time::now();

	fsec fs = profileTimeEnd - profileTimeStart;
	ms d = std::chrono::duration_cast<ms>(fs);
	auto time_passed_fs = fs.count();
	return time_passed_fs * 1000.0f;
}

CreatureCore::CreatureCore()
{
	pJsonData = nullptr;
	smooth_transitions = false;
	bone_data_size = 0.01f;
	bone_data_length_factor = 0.02f;
	should_play = true;
	region_overlap_z_delta = 0.01f;
	is_looping = true;
	play_start_done = false;
	play_end_done = false;
	is_disabled = false;
	is_driven = false;
	is_ready_play = false;
	do_file_warning = true;
	should_process_animation_start = false;
	should_process_animation_end = false;
	update_lock = new std::mutex();
}

bool 
CreatureCore::GetAndClearShouldAnimStart()
{
	bool retval = should_process_animation_start;
	should_process_animation_start = false;

	return retval;
}

bool 
CreatureCore::GetAndClearShouldAnimEnd()
{
	bool retval = should_process_animation_end;
	should_process_animation_end = false;

	return retval;
}

FProceduralMeshTriData 
CreatureCore::GetProcMeshData()
{
	if (!creature_manager)
	{
		FProceduralMeshTriData ret_data(nullptr,
			nullptr, nullptr,
			0, 0,
			&region_alphas,
			update_lock);

		return ret_data;
	}

	auto cur_creature = creature_manager->GetCreature();
	int32 num_points = cur_creature->GetTotalNumPoints();
	int32 num_indices = cur_creature->GetTotalNumIndices();
	glm::uint32 * cur_idx = cur_creature->GetGlobalIndices();
	glm::float32 * cur_pts = cur_creature->GetRenderPts();
	glm::float32 * cur_uvs = cur_creature->GetGlobalUvs();

	if (region_alphas.Num() != num_points)
	{
		region_alphas.SetNum(num_points);
	}

	FProceduralMeshTriData ret_data(cur_idx,
		cur_pts, cur_uvs,
		num_points, num_indices,
		&region_alphas,
		update_lock);

	return ret_data;
}

void CreatureCore::UpdateCreatureRender()
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
	/*
	TArray<FProceduralMeshTriangle>& write_triangles = draw_tris;

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
	*/

	// process the render regions
	ProcessRenderRegions();
}

bool CreatureCore::InitCreatureRender()
{
	FString cur_creature_filename = creature_filename;
	bool init_success = false;
	std::string load_filename;

	//////////////////////////////////////////////////////////////////////////
	//Changed by God of Pen
	//////////////////////////////////////////////////////////////////////////
	if (pJsonData != nullptr)
	{
		if (cur_creature_filename.IsEmpty())
		{
			cur_creature_filename = creature_asset_filename;
		}

		absolute_creature_filename = cur_creature_filename;
		load_filename = ConvertToString(cur_creature_filename);

		// try to load creature
		init_success = CreatureCore::LoadDataPacket(load_filename, pJsonData);;
	}
	else{
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
			load_filename = ConvertToString(cur_creature_filename);

			// try to load creature
			CreatureCore::LoadDataPacket(load_filename);
			init_success = true;
		}
		else {

			if (do_file_warning && (!load_filename.empty())) {
				UE_LOG(LogTemp, Warning, TEXT("ACreatureActor::BeginPlay() - ERROR! Could not load creature file: %s"), *creature_filename);
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("ACreatureActor::BeginPlay() - ERROR! Could not load creature file: %s"), *creature_filename));
			}
		}
	}
	
	if (init_success)
	{
		LoadCreature(load_filename);

		// try to load all animations
		auto all_animation_names = creature_manager->GetCreature()->GetAnimationNames();
		auto first_animation_name = all_animation_names[0];
		for (auto& cur_name : all_animation_names)
		{
			CreatureCore::LoadAnimation(load_filename, cur_name);
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
	}


	return init_success;
}

void CreatureCore::FillBoneData()
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

void CreatureCore::ParseEvents(float deltaTime)
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
			should_process_animation_start = true;
		}

		if ((cur_runtime + 1.0f >= cur_end_time)
			&& !is_looping
			&& !play_end_done
			&& should_play)
		{
			play_end_done = true;
			should_play = false;
			should_process_animation_end = true;
		}
	}

}

void CreatureCore::ProcessRenderRegions()
{
	auto cur_creature = creature_manager->GetCreature();
	auto& regions_map = cur_creature->GetRenderComposition()->getRegionsMap();
	int num_triangles = cur_creature->GetTotalNumIndices() / 3;

	// process alphas
	if (region_alphas.Num() != cur_creature->GetTotalNumPoints())
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
	/*
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
	*/
}

bool 
CreatureCore::LoadDataPacket(const std::string& filename_in)
{
	if (global_load_data_packets.count(filename_in) > 0)
	{
		// file already loaded, just return
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	//Changed!
	//////////////////////////////////////////////////////////////////////////
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
	
	

		return true;
}

bool CreatureCore::LoadDataPacket(const std::string& filename_in, FString* pSourceData)
{
	//////////////////////////////////////////////////////////////////////////
	//直接从Data中载入
	if (pSourceData == nullptr)
	{
		return false;
	}
	if (global_load_data_packets.count(filename_in) > 0)
	{
		// file already loaded, just return
		return true;
	}
	else
	{
		if (pSourceData->Len() == 0)
		{
			return false;
		}

		std::shared_ptr<CreatureModule::CreatureLoadDataPacket> new_packet =
			std::make_shared<CreatureModule::CreatureLoadDataPacket>();

		CreatureModule::LoadCreatureJSONDataFromString(std::string(TCHAR_TO_UTF8(*(*pSourceData))), *new_packet);
		global_load_data_packets[filename_in] = new_packet;
	}

	return true;
}

void 
CreatureCore::LoadAnimation(const std::string& filename_in, const std::string& name_in)
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

TArray<FProceduralMeshTriangle>&
CreatureCore::LoadCreature(const std::string& filename_in)
{
	auto load_data = global_load_data_packets[filename_in];

	std::shared_ptr<CreatureModule::Creature> new_creature =
		std::make_shared<CreatureModule::Creature>(*load_data);

	creature_manager = std::make_shared<CreatureModule::CreatureManager>(new_creature);

	draw_triangles.SetNum(creature_manager->GetCreature()->GetTotalNumIndices() / 3, true);

	return draw_triangles;
}

bool 
CreatureCore::AddLoadedAnimation(const std::string& filename_in, const std::string& name_in)
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

CreatureModule::CreatureManager * 
CreatureCore::GetCreatureManager()
{
	return creature_manager.get();
}

void 
CreatureCore::SetBluePrintActiveAnimation(FString name_in)
{
	auto cur_str = ConvertToString(name_in);
	SetActiveAnimation(cur_str);
}

void 
CreatureCore::SetBluePrintBlendActiveAnimation(FString name_in, float factor)
{
	auto cur_str = ConvertToString(name_in);
	SetAutoBlendActiveAnimation(cur_str, factor);
}

void 
CreatureCore::SetBluePrintAnimationCustomTimeRange(FString name_in, int32 start_time, int32 end_time)
{
	auto cur_str = ConvertToString(name_in);
	auto all_animations = creature_manager->GetAllAnimations();
	if (all_animations.count(cur_str) > 0)
	{
		all_animations[cur_str]->setStartTime(start_time);
		all_animations[cur_str]->setEndTime(end_time);
	}
}

void 
CreatureCore::MakeBluePrintPointCache(FString name_in, int32 approximation_level)
{
	auto cur_creature_manager = GetCreatureManager();
	if (!cur_creature_manager)
	{
		UE_LOG(LogTemp, Warning, TEXT("ACreatureActor::MakeBluePrintPointCache() - ERROR! Could not generate point cache for %s"), *name_in);
		return;
	}

	int32 real_approximation_level = approximation_level;
	if (real_approximation_level <= 0)
	{
		real_approximation_level = 1;
	}
	else if (real_approximation_level > 10)
	{
		real_approximation_level = 10;
	}

	cur_creature_manager->MakePointCache(ConvertToString(name_in), real_approximation_level);
}

void 
CreatureCore::ClearBluePrintPointCache(FString name_in, int32 approximation_level)
{
	auto cur_creature_manager = GetCreatureManager();
	if (!cur_creature_manager)
	{
		UE_LOG(LogTemp, Warning, TEXT("ACreatureActor::MakeBluePrintPointCache() - ERROR! Could not generate point cache for %s"), *name_in);
		return;
	}

	cur_creature_manager->ClearPointCache(ConvertToString(name_in));
}

FTransform 
CreatureCore::GetBluePrintBoneXform(FString name_in, bool world_transform, float position_slide_factor, FTransform base_transform)
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
				FTransform xform = base_transform;
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
CreatureCore::IsBluePrintBonesCollide(FVector test_point, float bone_size, FTransform base_transform)
{
	if (bone_size <= 0)
	{
		bone_size = 1.0f;
	}

	FTransform xform = base_transform;
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

bool 
CreatureCore::RunTick(float delta_time)
{
	std::lock_guard<std::mutex> scope_lock(*update_lock);

	if (is_driven)
	{
		UpdateCreatureRender();
		FillBoneData();

		return true;
	}

	if (is_disabled)
	{
		return false;
	}

	if (creature_manager)
	{
		ParseEvents(delta_time);

		if (should_play) {
			creature_manager->Update(delta_time);
		}

		UpdateCreatureRender();

		FillBoneData();

	}

	return true;
}

void 
CreatureCore::SetBluePrintAnimationLoop(bool flag_in)
{
	is_looping = flag_in;
	if (creature_manager) {
		creature_manager->SetShouldLoop(is_looping);
	}
}

void 
CreatureCore::SetBluePrintAnimationPlay(bool flag_in)
{
	should_play = flag_in;
	play_start_done = false;
	play_end_done = false;
}

void 
CreatureCore::SetBluePrintAnimationPlayFromStart()
{
	SetBluePrintAnimationResetToStart();
	SetBluePrintAnimationPlay(true);
}

void 
CreatureCore::SetBluePrintAnimationResetToStart()
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

float 
CreatureCore::GetBluePrintAnimationFrame()
{
	return animation_frame;
}

void 
CreatureCore::SetBluePrintRegionAlpha(FString region_name_in, uint8 alpha_in)
{
	if (region_name_in.IsEmpty())
	{
		return;
	}

	region_alpha_map.Add(region_name_in, alpha_in);
}

void 
CreatureCore::SetBluePrintRegionCustomOrder(TArray<FString> order_in)
{
	region_custom_order = order_in;
}

void 
CreatureCore::ClearBluePrintRegionCustomOrder()
{
	region_custom_order.Empty();
}

void 
CreatureCore::SetActiveAnimation(const std::string& name_in)
{
	creature_manager->SetActiveAnimationName(name_in);
}

void 
CreatureCore::SetAutoBlendActiveAnimation(const std::string& name_in, float factor)
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

void 
CreatureCore::SetIsDisabled(bool flag_in)
{
	is_disabled = flag_in;
}

void 
CreatureCore::SetDriven(bool flag_in)
{
	is_driven = flag_in;
}

bool 
CreatureCore::GetIsReadyPlay() const
{
	return is_ready_play;
}

void 
CreatureCore::RunBeginPlay()
{
	is_ready_play = false;
	InitCreatureRender();
	is_ready_play = true;

	region_alpha_map.Empty();
}
