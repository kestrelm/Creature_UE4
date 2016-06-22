#pragma  once
#include "Engine.h"
#include <vector>
#include <map>
#include <string>

#include "CreatureMetaAsset.generated.h"


class CreatureMetaData {
public:

	void clear()
	{
		mesh_map.clear();
		anim_order_map.clear();
	}

	void updateIndicesAndPoints(
		glm::uint32 * dst_indices,
		glm::uint32 * src_indices, 
		glm::float32 * dst_pts,
		float delta_z,
		int num_indices,
		int num_pts,
		const std::string& anim_name,
		int time_in)
	{
		bool has_data = false;
		auto cur_order = sampleOrder(anim_name, time_in);
		if(cur_order)
		{
			has_data = (cur_order->size() > 0);
		}

		if (has_data)
		{
			float cur_z = 0;
			// Copy new ordering to destination
			glm::uint32 * write_ptr = dst_indices;
			int total_num_write_indices = 0;
			for (auto region_id : (*cur_order))
			{
				if (mesh_map.count(region_id) == 0)
				{
					// region not found, just copy and return
					std::memcpy(dst_indices, src_indices, num_indices * sizeof(glm::uint32));
					return;
				}

				// Write indices
				auto& mesh_data = mesh_map[region_id];
				auto num_write_indices = mesh_data.second - mesh_data.first + 1;
				auto region_src_ptr = src_indices + mesh_data.first;
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
					int start_idx = mesh_data.first;
					int end_idx = mesh_data.second;
					
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

	std::vector<int> * sampleOrder(const std::string& anim_name, int time_in)
	{
		if (anim_order_map.count(anim_name) > 0)
		{
			auto& order_table = anim_order_map.at(anim_name);
			if (order_table.empty())
			{
				return nullptr;
			}

			int sample_time = order_table.begin()->first;

			for(auto& order_data : order_table)
			{
				if (time_in >= order_data.first)
				{
					sample_time = order_data.first;
				}
			}

			return &order_table.at(sample_time);
		}

		return nullptr;
	}

	std::map<int, std::pair<int, int>> mesh_map;
	std::map<std::string, std::map<int, std::vector<int> >> anim_order_map;
};

UCLASS()
class CREATUREPLUGIN_API UCreatureMetaAsset :public UObject{
	GENERATED_BODY()
public:

	// JSON String Data
	UPROPERTY()
	FString jsonString;

	FString& GetJsonString();

	void BuildMetaData();

	CreatureMetaData * GetMetaData();
	
	virtual void Serialize(FArchive& Ar) override;
	
protected:
	CreatureMetaData meta_data;
};
