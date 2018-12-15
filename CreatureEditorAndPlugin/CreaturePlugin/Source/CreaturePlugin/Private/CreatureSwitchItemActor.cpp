
#include "CreatureSwitchItemActor.h"
#include "CreaturePluginPCH.h"

static std::string ConvertToString1(FString str)
{
	std::string t = TCHAR_TO_UTF8(*str);
	return t;
}

ACreatureSwitchItemActor::ACreatureSwitchItemActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	creature_actor = nullptr;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	switch_init_done = false;
	real_switch_region = nullptr;

	switch_mesh = CreateDefaultSubobject<UCustomProceduralMeshComponent>(TEXT("CreatureSwitchItemActor"));
	RootComponent = switch_mesh;

	// Generate a single dummy triangle
	/*
	TArray<FProceduralMeshTriangle> triangles;
	GenerateTriangle(triangles);
	switch_mesh->SetProceduralMeshTriangles(triangles);
	*/
}

void
ACreatureSwitchItemActor::AddBluePrintSwitchData(FString name_in,
	int32 x_in,
	int32 y_in,
	int32 width_in,
	int32 height_in,
	int32 canvas_width_in,
	int32 canvas_height_in)
{
	switch_table[ConvertToString1(name_in)] = ACreatureSwitchData(x_in, y_in, width_in, height_in, canvas_width_in, canvas_height_in);
}

void ACreatureSwitchItemActor::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);
}

void ACreatureSwitchItemActor::InitRealSwitchRegion()
{
	/*
	int num_triangles = real_switch_region->getNumIndices() / 3;
	draw_triangles.SetNum(num_triangles, true);
	switch_mesh->SetProceduralMeshTriangles(draw_triangles);

	// find min and max uv box
	switch_min_uv.Z = 0;
	switch_max_uv.Z = 0;

	auto cur_uvs = real_switch_region->getUVs();
	switch_min_uv.X = cur_uvs[0];
	switch_min_uv.Y = cur_uvs[1];
	switch_max_uv = switch_min_uv;

	for (int i = 0; i < real_switch_region->getNumPts(); i++)
	{
		float cur_u = cur_uvs[0];
		float cur_v = cur_uvs[1];

		if (cur_u < switch_min_uv.X)
		{
			switch_min_uv.X = cur_u;
		}
		else if (cur_u > switch_max_uv.X)
		{
			switch_max_uv.X = cur_u;
		}


		if (cur_v < switch_min_uv.Y)
		{
			switch_min_uv.Y = cur_v;
		}
		else if (cur_v > switch_max_uv.Y)
		{
			switch_max_uv.Y = cur_v;
		}

		cur_uvs += 2;
	}

	switch_uv_width = switch_max_uv.X - switch_min_uv.X;
	switch_uv_height = switch_max_uv.Y - switch_min_uv.Y;
	*/
}

void 
ACreatureSwitchItemActor::TransformTextureSpace(ACreatureSwitchData& switch_data, float u_in, float v_in, float& u_out, float& v_out)
{
	float rel_u = (u_in - switch_min_uv.X) / switch_uv_width;
	float rel_v = (v_in - switch_min_uv.Y) / switch_uv_height;

	u_out = rel_u * switch_data.u_width + switch_data.u;
	v_out = rel_v * switch_data.v_height + switch_data.v;
}

void ACreatureSwitchItemActor::SwitchBluePrintData(FString name_in)
{
	switch_to_name = name_in;
}

void 
ACreatureSwitchItemActor::UpdateSwitchRender()
{
	/*
	if (switch_table.count(ConvertToString(switch_to_name)) <= 0)
	{
		return;
	}

	auto creature_manager = creature_actor->GetCreatureManager();
	auto cur_creature = creature_manager->GetCreature();
	std::vector<meshRenderRegion *>& cur_regions =
		cur_creature->GetRenderComposition()->getRegions();
	glm::float32 * cur_pts = cur_creature->GetRenderPts();
	glm::float32 * cur_uvs = cur_creature->GetGlobalUvs();

	float region_z = 0.0f, delta_z = creature_actor->region_overlap_z_delta;
	auto& switch_data = switch_table.at(ConvertToString(switch_to_name));

	// compute real region z
	for (auto& cur_region : cur_regions)
	{
		if (cur_region == real_switch_region)
		{
			break;
		}

		region_z += delta_z;
	}

	// form render triangles
	TArray<FProceduralMeshTriangle>& write_triangles = switch_mesh->GetProceduralTriangles();
	int num_triangles = real_switch_region->getNumIndices() / 3;

	static const FColor White(255, 255, 255, 255);
	int cur_pt_idx = 0, cur_uv_idx = 0;
	const int x_id = 0;
	const int y_id = 2;
	const int z_id = 1;

	glm::uint32 * region_indices = real_switch_region->getIndices();

	for (int i = 0; i < num_triangles; i++)
	{
		int real_idx_1 = region_indices[0];
		int real_idx_2 = region_indices[1];
		int real_idx_3 = region_indices[2];

		float read_u, read_v, out_u, out_v;

		FProceduralMeshTriangle triangle;

		// vertex 1
		cur_pt_idx = real_idx_1 * 3;
		cur_uv_idx = real_idx_1 * 2;
		triangle.Vertex0.Position.Set(cur_pts[cur_pt_idx + x_id], cur_pts[cur_pt_idx + y_id], cur_pts[cur_pt_idx + z_id]);
		triangle.Vertex0.Color = White;

		read_u = cur_uvs[cur_uv_idx];
		read_v = cur_uvs[cur_uv_idx + 1];
		TransformTextureSpace(switch_data, read_u, read_v, out_u, out_v);

		triangle.Vertex0.U = out_u;
		triangle.Vertex0.V = out_v;

		// vertex 2
		cur_pt_idx = real_idx_2 * 3;
		cur_uv_idx = real_idx_2 * 2;
		triangle.Vertex1.Position.Set(cur_pts[cur_pt_idx + x_id], cur_pts[cur_pt_idx + y_id], cur_pts[cur_pt_idx + z_id]);
		triangle.Vertex1.Color = White;

		read_u = cur_uvs[cur_uv_idx];
		read_v = cur_uvs[cur_uv_idx + 1];
		TransformTextureSpace(switch_data, read_u, read_v, out_u, out_v);

		triangle.Vertex1.U = out_u;
		triangle.Vertex1.V = out_v;

		// vertex 3
		cur_pt_idx = real_idx_3 * 3;
		cur_uv_idx = real_idx_3 * 2;
		triangle.Vertex2.Position.Set(cur_pts[cur_pt_idx + x_id], cur_pts[cur_pt_idx + y_id], cur_pts[cur_pt_idx + z_id]);
		triangle.Vertex2.Color = White;

		read_u = cur_uvs[cur_uv_idx];
		read_v = cur_uvs[cur_uv_idx + 1];
		TransformTextureSpace(switch_data, read_u, read_v, out_u, out_v);

		triangle.Vertex2.U = out_u;
		triangle.Vertex2.V = out_v;

		write_triangles[i] = triangle;

		region_indices += 3;
	}

	switch_mesh->SetBoundsScale(creature_actor->creature_bounds_scale);
	switch_mesh->SetBoundsOffset(creature_actor->creature_bounds_offset);
	switch_mesh->SetExtraXForm(creature_actor->GetTransform());
	switch_mesh->ForceAnUpdate();
	*/
}

void
ACreatureSwitchItemActor:: InitSwitchRenderData()
{
	if (creature_actor)
	{
		auto creature_manager = creature_actor->GetCreatureManager();
		if (creature_manager)
		{
			auto cur_creature = creature_manager->GetCreature();
			auto& regions_map = cur_creature->GetRenderComposition()->getRegionsMap();
			auto find_name = creature_switch_region;

			for (auto& cur_region_pair : regions_map)
			{
				auto cur_region = cur_region_pair.Value;
				if (cur_region->getName() == FName(*find_name))
				{
					// init render mesh
					real_switch_region = cur_region;
					InitRealSwitchRegion();

					switch_init_done = true;
					break;
				}
			}

		}
	}
}

// Update callback
void ACreatureSwitchItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // Call parent class tick function  

	if (creature_actor)
	{
		// sync up with target creature actor
		FTransform target_transform = creature_actor->GetTransform();
		SetActorTransform(target_transform);

		// init if required
		if (!switch_init_done)
		{
			InitSwitchRenderData();
		}

		UpdateSwitchRender();
	}
}

// Called on startup
void ACreatureSwitchItemActor::BeginPlay()
{
	switch_init_done = false;
	real_switch_region = nullptr;

	Super::BeginPlay();
}

// Generate a single horizontal triangle counterclockwise to point up (one face, visible only from the top, not from the bottom)
void ACreatureSwitchItemActor::GenerateTriangle(TArray<FProceduralMeshTriangle>& OutTriangles)
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
