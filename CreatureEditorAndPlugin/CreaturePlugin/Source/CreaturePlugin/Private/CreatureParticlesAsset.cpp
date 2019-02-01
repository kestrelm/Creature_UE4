#include "CreatureParticlesAsset.h"
#include "CreatureCore.h"
#include "CreatureMetaAsset.h"
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
		new_offsets_data.name = FName(pack_objs[offset].string_val.c_str());
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

				int32 local_max_particles = 0;
				for (int32 m = 0; m < num_particles; m++)
				{
					local_max_particles += (int32)(pack_objs[offset + (5 * m) + 1].float_array_val.size() / 2);
				}
				max_lparticles_num[layer_idx] = std::max(max_lparticles_num[layer_idx], local_max_particles);

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

TArray<FName> UCreatureParticlesAsset::getAnimClipNames(const std::vector<mpMiniLib::msg_mini_generic_data>& pack_objs) const
{
	TArray<FName> ret_names;
	for (const auto& cur_data : m_ClipDataOffsets)
	{
		ret_names.Add(cur_data.Key);
	}

	return TArray<FName>();
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

void UCreatureParticlesAsset::setupMeshModifier(CreatureCore & core_in)
{
	getPackData();
	auto cur_char = core_in.GetCreatureManager()->GetCreature();

	int32 max_indices_num = m_maxIndicesNum + cur_char->GetTotalNumIndices();
	core_in.mesh_modifier = MakeShareable(new CreatureMeshDataModifier(
		max_indices_num,
		(m_maxParticlesNum * 4) + cur_char->GetTotalNumPoints()));
	core_in.mesh_modifier->m_maxIndice = max_indices_num;

	auto& mesh_modifier = *core_in.mesh_modifier;

	mesh_modifier.m_initCB = [this](CreatureMeshDataModifier& modifier_in, CreatureCore& core_in)
	{
		// Copy over Character data first
		auto cur_char = core_in.GetCreatureManager()->GetCreature();
		int32 char_num_pts = cur_char->GetTotalNumPoints();
		glm::float32 * char_pts = cur_char->GetGlobalPts();
		FMemory::Memcpy(modifier_in.m_pts.GetData(), char_pts, sizeof(glm::float32) * char_num_pts * 3);

		glm::float32 * char_uvs = cur_char->GetGlobalUvs();
		FMemory::Memcpy(modifier_in.m_uvs.GetData(), char_uvs, sizeof(glm::float32) * char_num_pts * 2);

		glm::uint32 * char_indices = core_in.GetIndicesCopy(cur_char->GetTotalNumIndices());
		FMemory::Memcpy(modifier_in.m_indices.GetData(), char_indices, sizeof(glm::uint32) * cur_char->GetTotalNumIndices());

		for (int32 i = 0; i < modifier_in.m_colors.Num(); i++)
		{
			modifier_in.m_colors[i] = FColor(255, 255, 255, 255);
		}

		modifier_in.m_isValid = true;
		modifier_in.m_numIndices = cur_char->GetTotalNumIndices();
	};

	mesh_modifier.m_updateCB = [this](CreatureMeshDataModifier& modifier_in, CreatureCore& core_in)
	{
		modifier_in.m_isValid = false;
		if (core_in.meta_data == nullptr)
		{
			// Meta Data not loaded
			return;
		}

		const auto& cur_anim_name = core_in.GetCreatureManager()->GetActiveAnimationName();
		if (!m_ClipDataOffsets.Contains(cur_anim_name))
		{
			// No animation
			return;
		}

		// Copy over Character data first
		auto cur_char = core_in.GetCreatureManager()->GetCreature();
		int32 char_num_pts = cur_char->GetTotalNumPoints();
		glm::float32 * char_pts = cur_char->GetRenderPts();
		FMemory::Memcpy(modifier_in.m_pts.GetData(), char_pts, sizeof(glm::float32) * char_num_pts * 3);

		glm::float32 * char_uvs = cur_char->GetGlobalUvs();
		FMemory::Memcpy(modifier_in.m_uvs.GetData(), char_uvs, sizeof(glm::float32) * char_num_pts * 2);
		FMemory::Memcpy(modifier_in.m_colors.GetData(), core_in.region_colors.GetData(), sizeof(FColor) * core_in.region_colors.Num());

		auto setVertData = [&modifier_in](
			const glm::vec2& pt_in,
			float pt_z,
			const glm::vec2& uv_in,
			const glm::vec4& color_in,
			int32& pt_offset,
			int32& uv_offset,
			int32& colors_offset) -> void
		{
			auto& set_pts = modifier_in.m_pts;
			auto& set_uvs = modifier_in.m_uvs;
			auto& set_colors = modifier_in.m_colors;

			// set points
			set_pts[pt_offset] = pt_in.x;
			set_pts[pt_offset + 1] = pt_in.y;
			set_pts[pt_offset + 2] = pt_z;
			pt_offset += 3;

			// set uvs
			set_uvs[uv_offset] = uv_in.x;
			set_uvs[uv_offset + 1] = uv_in.y;
			uv_offset += 2;

			// set color
			set_colors[colors_offset] = FColor((uint8)(color_in.r * 255.0f), (uint8)(color_in.g * 255.0f), (uint8)(color_in.b * 255.0f), (uint8)(color_in.a * 255.0f));
			colors_offset += 1;
		};

		auto setIndiceData = [&modifier_in](int& offset, int val) -> void
		{
			modifier_in.m_indices[offset] = val;
			offset++;
		};

		// Generate Particle data
		const auto& pack_objs = m_PackData->msg_mini_get_generic_objects();
		int32 char_real_indices_num = core_in.GetRealTotalIndicesNum();
		const auto& clip_data = m_ClipDataOffsets[cur_anim_name];
		auto cur_anim = core_in.GetCreatureManager()->GetAnimation(cur_anim_name);
		auto delta_frames = static_cast<int32>(std::roundf(core_in.GetCreatureManager()->getActualRunTime() - cur_anim->getStartTime()));
		int32 clip_runtime = std::min(std::max(0, delta_frames), clip_data.num_frames - 1);

		const auto& frame_data = clip_data.particles_lookup[clip_runtime];

		auto rotVec2D = [](const glm::vec2& vec_in, float angle)
		{
			auto ret_vec = vec_in;
			ret_vec.x = vec_in.x * cosf(angle) - vec_in.y * sinf(angle);
			ret_vec.y = vec_in.x * sinf(angle) + vec_in.y * cosf(angle);

			return ret_vec;
		};

		auto rotVec2D_90 = [](const glm::vec2& vec_in)
		{
			auto ret_vec = vec_in;
			ret_vec.x = -vec_in.y;
			ret_vec.y = vec_in.x;

			return ret_vec;
		};

		int32 pt_offset = char_num_pts * 3;
		int32 uv_offset = char_num_pts * 2;
		int32 colors_offset = char_num_pts;
		int32 indices_offset = 0;
		modifier_in.m_numIndices = 0;

		TArray<meshRenderRegion *>& all_regions = cur_char->GetRenderComposition()->getRegions();
		// Go through each mesh region and check to see if we need to process it considering SkinSwaps as well
		float region_z = 0.0f;
		for (const auto cur_region : all_regions)
		{
			bool is_valid = true;
			if (core_in.shouldSkinSwap())
			{
				is_valid = (core_in.meta_data->skin_swaps[core_in.skin_swap_name].Contains(cur_region->getName().ToString()));
			}

			if (is_valid)
			{
				// Copy over base region indices
				FMemory::Memcpy(
					modifier_in.m_indices.GetData() + indices_offset,
					cur_region->getIndices(),
					sizeof(glm::uint32) * cur_region->getNumIndices());
				indices_offset += cur_region->getNumIndices();

				// Set base region z
				for (int32 i = 0; i < cur_region->getNumIndices(); i++)
				{
					modifier_in.m_pts[i * 3 + 2] = region_z;
				}

				int32 layer_idx = cur_region->getTagId();
				if (frame_data.Contains(cur_region->getTagId())) {
					// Process Particles for layer Start
					const auto& layer_data = frame_data[cur_region->getTagId()];
					for (int32 i = 0; i < layer_data.num_particles; i++)
					{
						const int32 rel_offset = layer_data.offset + (i * 5);
						const auto& p_pos_list = pack_objs[rel_offset].float_array_val;
						float p_angle = pack_objs[rel_offset + 1].float_val;
						int32 p_sprite_idx = -pack_objs[rel_offset + 2].int_val;
						const auto& p_size = pack_objs[rel_offset + 3].float_array_val;
						const auto& p_color = pack_objs[rel_offset + 4].float_array_val;
						const auto& p_sprite_uvs = core_in.meta_data->uvs_data[p_sprite_idx];

						glm::vec2 b_uv0(p_sprite_uvs.uv0.X, p_sprite_uvs.uv0.Y);
						glm::vec2 b_uv1(p_sprite_uvs.uv1.X, p_sprite_uvs.uv1.Y);
						glm::vec4 b_color(p_color[0], p_color[1], p_color[2], p_color[3]);
						region_z += core_in.region_overlap_z_delta * 0.1f;

						if (p_pos_list.size() == 2)
						{
							// No Trail
							int idx_pt_offset = pt_offset / 3;
							glm::vec2 b_pos(p_pos_list[0], p_pos_list[1]);

							setVertData(
								b_pos + rotVec2D(glm::vec2(-p_size[0], -p_size[1]), p_angle),
								region_z,
								glm::vec2(b_uv0.x, b_uv0.y),
								b_color,
								pt_offset,
								uv_offset,
								colors_offset
							);


							setVertData(
								b_pos + rotVec2D(glm::vec2(-p_size[0], p_size[1]), p_angle),
								region_z,
								glm::vec2(b_uv0.x, b_uv1.y),
								b_color,
								pt_offset,
								uv_offset,
								colors_offset
							);

							setVertData(
								b_pos + rotVec2D(glm::vec2(p_size[0], p_size[1]), p_angle),
								region_z,
								glm::vec2(b_uv1.x, b_uv1.y),
								b_color,
								pt_offset,
								uv_offset,
								colors_offset
							);

							setVertData(
								b_pos + rotVec2D(glm::vec2(p_size[0], -p_size[1]), p_angle),
								region_z,
								glm::vec2(b_uv1.x, b_uv0.y),
								b_color,
								pt_offset,
								uv_offset,
								colors_offset
							);

							// Indices
							setIndiceData(indices_offset, idx_pt_offset + 2);
							setIndiceData(indices_offset, idx_pt_offset + 1);
							setIndiceData(indices_offset, idx_pt_offset);
							setIndiceData(indices_offset, idx_pt_offset + 2);
							setIndiceData(indices_offset, idx_pt_offset);
							setIndiceData(indices_offset, idx_pt_offset + 3);													
						}
						else if(p_pos_list.size() > 2) {
							// With Trail
							int idx_pt_offset = pt_offset / 3;
							int trail_num = p_pos_list.size() / 2;
							auto getParticlePos = [&p_pos_list](int idx) -> glm::vec2
							{
								return glm::vec2(p_pos_list[idx * 2], p_pos_list[idx * 2 + 1]);
							};

							glm::vec2 dir_p(0, 0), rot_p(0, 0), diff_p(0, 0);
							glm::vec2 delta_uvs = b_uv1 - b_uv0;
							for (int32 j = 0; j < trail_num; j++)
							{
								glm::vec2 p1 = getParticlePos(j);

								if (j < (trail_num - 1)) {
									diff_p = getParticlePos(j + 1) - getParticlePos(j);
								}
								else {
									diff_p = getParticlePos(trail_num - 1) - getParticlePos(trail_num - 2);
								}

								if (glm::length(diff_p) > 0)
								{
									dir_p = glm::normalize(diff_p);
								}

								rot_p = rotVec2D_90(dir_p) * p_size[1];
								glm::vec2 cur_uv(0, 0);
								float sub_alpha = std::min(1.0f / (float)(trail_num) * (float)(j), 1.0f);

								if (j < (trail_num - 1)) {
									cur_uv.x = delta_uvs.x / (float)(trail_num) * (float)(j)+b_uv0.x;
									cur_uv.y = b_uv0.y;

									setVertData(
										p1 + rot_p,
										region_z,
										cur_uv,
										b_color * sub_alpha,
										pt_offset,
										uv_offset,
										colors_offset
									);

									cur_uv.y = b_uv1.y;
									setVertData(
										p1 - rot_p,
										region_z,
										cur_uv,
										b_color * sub_alpha,
										pt_offset,
										uv_offset,
										colors_offset
									);
								}
								else {
									// Final trail pos
									cur_uv.x = b_uv1.x;
									cur_uv.y = b_uv0.y;

									setVertData(
										p1 + diff_p + rot_p,
										region_z,
										cur_uv,
										b_color * sub_alpha,
										pt_offset,
										uv_offset,
										colors_offset
									);

									cur_uv.y = b_uv1.y;
									setVertData(
										p1 + diff_p + rot_p,
										region_z,
										cur_uv,
										b_color * sub_alpha,
										pt_offset,
										uv_offset,
										colors_offset
									);
								}
							}

							// Indices
							int delta_trail_indices = 0;
							for (int32 j = 0; j < trail_num - 1; j++)
							{
								setIndiceData(indices_offset, idx_pt_offset + 2 + delta_trail_indices);
								setIndiceData(indices_offset, idx_pt_offset + delta_trail_indices);
								setIndiceData(indices_offset, idx_pt_offset + 1 + delta_trail_indices);
								
								setIndiceData(indices_offset, idx_pt_offset + 2 + delta_trail_indices);
								setIndiceData(indices_offset, idx_pt_offset + 1 + delta_trail_indices);
								setIndiceData(indices_offset, idx_pt_offset + 3 + delta_trail_indices);
								
								delta_trail_indices += 2;
							}
						}
						
					}

					// Process Particles for layer End
				}
			}

			region_z += core_in.region_overlap_z_delta;
		}

		modifier_in.m_numIndices = indices_offset;
		modifier_in.m_isValid = true;
	};

}

