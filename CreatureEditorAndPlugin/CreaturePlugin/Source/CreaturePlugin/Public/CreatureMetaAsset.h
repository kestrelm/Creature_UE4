#pragma  once
#include "CoreMinimal.h"
#include "glm/fwd.hpp"
#include <vector>
#include <algorithm>
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/BoxComponent.h"
#include "CreatureMetaAsset.generated.h"

class meshBone;
class meshRenderBoneComposition;

namespace CreatureModule {
	class CreatureManager;
	class Creature;
}

class CreatureMetaData {
public:

	void clear()
	{
		mesh_map.Empty();
		anim_order_map.Empty();
		skin_swaps.Empty();
		morph_data = MorphData();
		vertex_attachments.Empty();
	}

	void buildSkinSwapIndices(
		const FString& swap_name,
		meshRenderBoneComposition * bone_composition,
		TArray<int32>& skin_swap_indices,
		TSet<int32>& skin_swap_region_ids
	);

	bool hasRegionOrder(const FString& anim_name, int time_in)
	{
		return (sampleOrder(anim_name, time_in) != nullptr);
	}

	int updateIndicesAndPoints(
		glm::uint32 * dst_indices,
		glm::uint32 * src_indices, 
		glm::float32 * dst_pts,
		float delta_z,
		int num_indices,
		int num_pts,
		const FString& anim_name,
		bool skin_swap_active,
		const TSet<int32>& skin_swap_region_ids,
		int time_in)
	{
		bool has_data = false;
		auto cur_order = sampleOrder(anim_name, time_in);
		if(cur_order)
		{
			has_data = (cur_order->Num() > 0);
		}

		int total_num_write_indices = 0;
		if (has_data)
		{
			float cur_z = 0;
			// Copy new ordering to destination
			glm::uint32 * write_ptr = dst_indices;
			for (auto region_id : (*cur_order))
			{
				if (mesh_map.Contains(region_id) == false)
				{
					// region not found, just copy and return
					std::memcpy(dst_indices, src_indices, num_indices * sizeof(glm::uint32));
					return num_indices;
				}

				// Write indices
				auto& mesh_data = mesh_map[region_id];
				auto num_write_indices = mesh_data.Get<1>() - mesh_data.Get<0>() + 1;
				auto region_src_ptr = src_indices + mesh_data.Get<0>();
				bool valid_region = true;
				if (skin_swap_active)
				{
					valid_region = skin_swap_region_ids.Contains(region_id);
				}

				if (valid_region) {
					total_num_write_indices += num_write_indices;
					if (total_num_write_indices > num_indices)
					{
						// overwriting boundaries of array, regions do not match so copy and return
						std::memcpy(dst_indices, src_indices, num_indices * sizeof(glm::uint32));
						return num_indices;
					}

					std::memcpy(write_ptr, region_src_ptr, num_write_indices * sizeof(glm::uint32));
					write_ptr += num_write_indices;
				}

				// Write points
				{
					int start_idx = mesh_data.Get<0>();
					int end_idx = mesh_data.Get<1>();
					
					if ((int)src_indices[end_idx] < num_pts)
					{
						for (int i = start_idx; i <= end_idx; i++)
						{
							auto cur_pt_idx = src_indices[i] * 3;
							dst_pts[cur_pt_idx + 2] = cur_z;
						}
					}

					cur_z += delta_z;
				}
			}
		}
		else {
			// Nothing changded, just copy from source
			total_num_write_indices = num_indices;
			std::memcpy(dst_indices, src_indices, num_indices * sizeof(glm::uint32));
			return num_indices;
		}

		return total_num_write_indices;
	}

	TArray<int32> * sampleOrder(const FString& anim_name, int32 time_in)
	{
		if (anim_order_map.Contains(anim_name))
		{
			auto& order_table = anim_order_map[anim_name];
			if (order_table.Num() == 0)
			{
				return nullptr;
			}

			int32 sample_time = 0;
			for(auto& order_data : order_table)
			{
				if (time_in >= order_data.Key)
				{
					sample_time = order_data.Key;
				}
			}

			if (!order_table.Contains(sample_time))
			{
				return nullptr;
			}

			return &order_table[sample_time];
		}

		return nullptr;
	}

	bool addSkinSwap(const FString& swap_name, const TSet<FString>& set_in)
	{
		if (skin_swaps.Contains(swap_name))
		{
			return false;
		}

		skin_swaps.Add(swap_name, set_in);
		return true;
	}

	void computeMorphWeights(const FVector2D& img_pt)
	{
		morph_data.play_img_pt = img_pt;
		auto sampleFilterPt = [](
				float q11, // (x1, y1)
				float q12, // (x1, y2)
				float q21, // (x2, y1)
				float q22, // (x2, y2)
				float x1,
				float y1,
				float x2,
				float y2,
				float x,
				float y)
		{
			float x2x1, y2y1, x2x, y2y, yy1, xx1;
			x2x1 = x2 - x1;
			y2y1 = y2 - y1;
			x2x = x2 - x;
			y2y = y2 - y;
			yy1 = y - y1;
			xx1 = x - x1;

			float denom = (x2x1 * y2y1);
			float numerator = (
				q11 * x2x * y2y +
				q21 * xx1 * y2y +
				q12 * x2x * yy1 +
				q22 * xx1 * yy1
				);

			return (denom == 0) ? q11 : (1.0f / denom * numerator);
		};

		auto lookupVal = [this](int x_in, int y_in, int idx)
		{
			auto& cur_space = morph_data.morph_spaces[idx];
			return static_cast<float>(cur_space[y_in *  morph_data.morph_res + x_in]) / 255.0f;
		};

		float x1 = std::floor(img_pt.X);
		float y1 = std::floor(img_pt.Y);
		float x2 = std::ceil(img_pt.X);
		float y2 = std::ceil(img_pt.Y);

		for (int32 i = 0; i < morph_data.morph_spaces.Num(); i++)
		{
			float q11 = (float)lookupVal(x1, y1, i); // (x1, y1)
			float q12 = (float)lookupVal(x1, y2, i); // (x1, y2)
			float q21 = (float)lookupVal(x2, y1, i); // (x2, y1)
			float q22 = (float)lookupVal(x2, y2, i); // (x2, y2)

			float sample_val = sampleFilterPt(
				q11, q12, q21, q22, x1, y1, x2, y2, img_pt.X, img_pt.Y);
			morph_data.weights[i] = sample_val;
		}
	}

	void computeMorphWeightsNormalised(const FVector2D& normal_pt)
	{
		auto img_pt = normal_pt * FVector2D(morph_data.morph_res - 1, morph_data.morph_res - 1);
		img_pt.X = std::max(std::min((float)morph_data.morph_res - 1.0f, img_pt.X), 0.0f);
		img_pt.Y = std::max(std::min((float)morph_data.morph_res - 1.0f, img_pt.Y), 0.0f);
		computeMorphWeights(img_pt);
	}

	void computeMorphWeightsWorld(const FVector2D& world_pt, const FVector2D& base_pt, float radius)
	{
		auto rel_pt = world_pt - base_pt;
		auto cur_length = FVector2D::Distance(world_pt, base_pt);
		if (cur_length > radius)
		{
			rel_pt.Normalize();
			rel_pt *= radius;
		}

		FVector2D normal_pt = (rel_pt + FVector2D(radius, radius)) / (radius * 2.0f);
		computeMorphWeightsNormalised(normal_pt);
	}

	void updateMorphStep(CreatureModule::CreatureManager * manager_in, float delta_step);

	TMap<int, TTuple<int32, int32>> mesh_map;
	TMap<FString, TMap<int32, TArray<int32> >> anim_order_map;
	TMap<FString, TMap<int32, FString> > anim_events_map;
	TMap<FString, TSet<FString>> skin_swaps;
	TMap<FString, int> vertex_attachments;

	struct MorphData
	{
		TArray<TArray<uint8_t>> morph_spaces;
		FString center_clip;
		TArray<TTuple<FString, FVector2D>> morph_clips;
		TArray<float> weights;
		FVector2D bounds_min, bounds_max;
		int morph_res;
		TArray<TTuple<FName, float>> play_anims_data;
		TTuple<FName, float> play_center_anim_data;
		TArray<glm::float32> play_pts;
		FVector2D play_img_pt;

		bool isValid() const {
			return (morph_spaces.Num() > 0);
		}
	};
	MorphData morph_data;
};

class CreaturePhysicsData
{
public:
	CreaturePhysicsData()
	{
		run_cnt = 0;
	}

	static FString getConstraintsKey(const FString& bone1, const FString& bone2)
	{
		return bone1 + FString("_") + bone2;
	}

	TArray<UPhysicsConstraintComponent *> * getConstraint(const FString& bone1, const FString& bone2)
	{
		auto test_name = getConstraintsKey(bone1, bone2);
		if (constraints.Contains(test_name))
		{
			return &constraints[test_name];
		}

		test_name = getConstraintsKey(bone2, bone1);
		if (constraints.Contains(test_name))
		{
			return &constraints[test_name];
		}

		return nullptr;
	}

	void createPhysicsChain(
		const FTransform& base_xform,
		USceneComponent * attach_root,
		UObject * parent,
		const TArray<meshBone *>& bones_in,
		float stiffness,
		float damping,
		const FString& anim_clip_name_in);

	void clearPhysicsChain();

	void updateKinematicPos(const FTransform& base_xform, meshBone * bone_in);

	void updateAllKinematicBones(const FTransform& base_xform);

	void updateBonePositions(const FTransform& base_xform);
	
	void drawDebugBones(UWorld * world_in);

	struct boxAndBone
	{
		boxAndBone(
			UBoxComponent * box_in,
			meshBone * bone_in,
			meshBone * next_bone_in,
			const FVector2D& basis_in)
		{
			box = box_in;
			end_box = nullptr;
			bone = bone_in;
			next_bone = next_bone_in;
			basis = basis_in;
		}

		UBoxComponent * box, *end_box;
		meshBone * bone, *next_bone;
		FVector2D basis;
	};

	TMap<FString, boxAndBone> bodies;
	TMap<FString, TArray<UPhysicsConstraintComponent *> > constraints;
	TArray<meshBone *> kinematic_bones;
	FString anim_clip_name;
	int run_cnt;
};

USTRUCT(BlueprintType)
struct FBendPhysicsChain
{
	GENERATED_BODY()

	// Name of this bend physics motor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creature")
	FString motor_name;

	// The animation clip name it is associated with
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creature")
	FString anim_clip_name;

	// The animation clip name it is associated with
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	bool active = false;

	// Number of bones associated with this chain
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creature")
	int32 num_bones = 0;

	// Stiffness of this chain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float stiffness = 10.0f;

	// Damping of this chain
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Creature")
	float damping = 0.1f;

	TArray<FString> bone_names;
};

UCLASS()
class CREATUREPLUGIN_API UCreatureMetaAsset :public UObject{
	GENERATED_BODY()
public:

	// JSON String Data
	UPROPERTY()
	FString jsonString;

	// The available bend physics chains
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Creature")
	TArray<FBendPhysicsChain> bend_physics_chains;

	// The available skin swaps
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creature")
	TArray<FString> skin_swap_names;
	
	// The available events
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creature")
	TArray<FString> event_names;

	// The available morph poses
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creature")
	TArray<FString> morph_poses;

	// The available vertex attachments
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Creature")
	TArray<FString> vertex_attachments;

	FString& GetJsonString();

	void BuildMetaData();

	TSharedPtr<CreaturePhysicsData>
	CreateBendPhysicsChain(
		const FTransform& base_xform,
		USceneComponent * attach_root,
		UObject * parent,
		meshRenderBoneComposition * bone_composition, 
		const FString& anim_clip);

	CreatureMetaData * GetMetaData();

	virtual void PostLoad() override;

	virtual void Serialize(FArchive& Ar) override;

protected:
	CreatureMetaData meta_data;
	
#if WITH_EDITORONLY_DATA
public:
	FString GetSourceFilename() const;
	void SetSourceFilename(const FString &filename);
	void PostInitProperties() override;

protected:
	// Denoting source filename using UE4's asset registry system
	UPROPERTY(VisibleAnywhere, Instanced, Category = ImportSettings)
	class UAssetImportData* AssetImportData;
#endif

};
