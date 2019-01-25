#include "CreatureParticlesAsset.h"
#include "CreaturePluginPCH.h"

// UCreatureParticlesAsset

void UCreatureParticlesAsset::computeDataInfo(const std::vector<mpMiniLib::msg_mini_generic_data>& pack_objs)
{
	TMap<int, int> max_lparticles_num;
	animation_clipnames.Empty();
	m_ClipDataOffsets.Empty();

	auto num_clips = getAnimClipsNum(pack_objs);
	int offset = 2;
	for (int32 i = 0; i < num_clips; i++)
	{
		clipOffsetsData new_offsets_data;

		// name
		offset++;
		new_offsets_data.base_idx = offset;
		new_offsets_data.name = FString(pack_objs[offset].string_val.c_str());
		animation_clipnames.Add(new_offsets_data.name);

		// num frames
		offset++;
		int32 num_frames = pack_objs[offset].int_val;
		new_offsets_data.num_frames = num_frames;

		for (int32 j = 0; j < num_frames; j++)
		{
			// num layers
			offset++;
			int32 num_layers = pack_objs[offset].int_val;
			TMap<int, particlesOffsetsData> particles_layers_map;

			for (int32 k = 0; k < num_layers; k++)
			{
				// layer idx
				offset++;
				int32 layer_idx = pack_objs[offset].int_val;
				if (!max_lparticles_num.Contains(layer_idx))
				{
					max_lparticles_num.Add(layer_idx, 0);
				}

				// num particles
				offset++;
				int32 num_particles = pack_objs[offset].int_val;
				max_lparticles_num[layer_idx] = std::max(max_lparticles_num[layer_idx], num_particles);

				particles_layers_map.Add(layer_idx, particlesOffsetsData(offset + 1, num_particles));
				for (int32 m = 0; m < num_particles; m++)
				{
					// particles
					offset += 5;
				}				
			}

			new_offsets_data.particles_lookup.Add(j, particles_layers_map);
		}

		m_ClipDataOffsets.Add(new_offsets_data.name, new_offsets_data);
	}

	m_maxParticlesNum = 0;
	for (const auto& cur_data : max_lparticles_num)
	{
		m_maxParticlesNum = std::max(m_maxParticlesNum, cur_data.Value);
	}

	m_maxIndicesNum = 3 * 2 * m_maxParticlesNum;
}

int32 UCreatureParticlesAsset::getVersion(const std::vector<mpMiniLib::msg_mini_generic_data>& pack_objs) const
{
	return pack_objs[0].int_val;
}

int32 UCreatureParticlesAsset::getGapStep(const std::vector<mpMiniLib::msg_mini_generic_data>& pack_objs) const
{
	return pack_objs[1].int_val;
}

int32 UCreatureParticlesAsset::getAnimClipsNum(const std::vector<mpMiniLib::msg_mini_generic_data>& pack_objs) const
{
	return pack_objs[2].int_val;;
}

TArray<FString> UCreatureParticlesAsset::getAnimClipNames(const std::vector<mpMiniLib::msg_mini_generic_data>& pack_objs) const
{
	TArray<FString> ret_names;
	for (const auto& cur_data : m_ClipDataOffsets)
	{
		ret_names.Add(cur_data.Key);
	}

	return TArray<FString>();
}

void UCreatureParticlesAsset::setData(const TArray<uint8>& data_in)
{
	m_ParticlesBytes = data_in;
	auto pack_data = getPackData();
	if (pack_data == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCreatureParticlesAsset::setData() - Could not load particle asset!"));
		return;
	}
}

mpMiniLib::msg_mini * UCreatureParticlesAsset::getPackData()
{
	if ((m_PackData.Get() == nullptr) && (m_ParticlesBytes.Num() > 0))
	{
		std::vector<uint8_t> tmp_buf(m_ParticlesBytes.Num());
		for (int32 i = 0; i < m_ParticlesBytes.Num(); i++)
		{
			tmp_buf[i] = m_ParticlesBytes[i];
		}

		m_PackData = MakeUnique<mpMiniLib::msg_mini>(tmp_buf);
		const auto& pack_objs = m_PackData->msg_mini_get_generic_objects();
		computeDataInfo(pack_objs);
	}

	return m_PackData.Get();
}

