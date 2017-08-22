#pragma  once
#include "Engine.h"
#include "glm/fwd.hpp"
#include <vector>
#include "CreatureMetaAsset.generated.h"

class meshBone;
class meshRenderBoneComposition;

class CreatureMetaData {
public:

	void clear()
	{
		mesh_map.Empty();
		anim_order_map.Empty();
		skin_swaps.Empty();
	}

	void buildSkinSwapIndices(
		const FString& swap_name, 
		meshRenderBoneComposition * bone_composition,
		TArray<int32>& skin_swap_indices
	);

	void updateIndicesAndPoints(
		glm::uint32 * dst_indices,
		glm::uint32 * src_indices, 
		glm::float32 * dst_pts,
		float delta_z,
		int num_indices,
		int num_pts,
		const FString& anim_name,
		int time_in)
	{
		bool has_data = false;
		auto cur_order = sampleOrder(anim_name, time_in);
		if(cur_order)
		{
			has_data = (cur_order->Num() > 0);
		}

		if (has_data)
		{
			float cur_z = 0;
			// Copy new ordering to destination
			glm::uint32 * write_ptr = dst_indices;
			int total_num_write_indices = 0;
			for (auto region_id : (*cur_order))
			{
				if (mesh_map.Contains(region_id) == false)
				{
					// region not found, just copy and return
					std::memcpy(dst_indices, src_indices, num_indices * sizeof(glm::uint32));
					return;
				}

				// Write indices
				auto& mesh_data = mesh_map[region_id];
				auto num_write_indices = mesh_data.Get<1>() - mesh_data.Get<0>() + 1;
				auto region_src_ptr = src_indices + mesh_data.Get<0>();
				total_num_write_indices += num_write_indices;

				if (total_num_write_indices > num_indices)
				{
					// overwriting boundaries of array, regions do not match so copy and return
					std::memcpy(dst_indices, src_indices, num_indices * sizeof(glm::uint32));
					return;
				}

				std::memcpy(write_ptr, region_src_ptr, num_write_indices * sizeof(glm::uint32));

				write_ptr += num_write_indices;

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
			std::memcpy(dst_indices, src_indices, num_indices * sizeof(glm::uint32));
		}
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

	TMap<int, TTuple<int32, int32>> mesh_map;
	TMap<FString, TMap<int32, TArray<int32> >> anim_order_map;
	TMap<FString, TMap<int32, FString> > anim_events_map;
	TMap<FString, TSet<FString>> skin_swaps;
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
};
