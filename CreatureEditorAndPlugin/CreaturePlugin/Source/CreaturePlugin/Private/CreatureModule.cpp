/******************************************************************************
 * Creature Runtimes License
 *
 * Copyright (c) 2015, Kestrel Moon Studios
 * All rights reserved.
 *
 * Preamble: This Agreement governs the relationship between Licensee and Kestrel Moon Studios(Hereinafter: Licensor).
 * This Agreement sets the terms, rights, restrictions and obligations on using [Creature Runtimes] (hereinafter: The Software) created and owned by Licensor,
 * as detailed herein:
 * License Grant: Licensor hereby grants Licensee a Sublicensable, Non-assignable & non-transferable, Commercial, Royalty free,
 * Including the rights to create but not distribute derivative works, Non-exclusive license, all with accordance with the terms set forth and
 * other legal restrictions set forth in 3rd party software used while running Software.
 * Limited: Licensee may use Software for the purpose of:
 * Running Software on Licensee’s Website[s] and Server[s];
 * Allowing 3rd Parties to run Software on Licensee’s Website[s] and Server[s];
 * Publishing Software’s output to Licensee and 3rd Parties;
 * Distribute verbatim copies of Software’s output (including compiled binaries);
 * Modify Software to suit Licensee’s needs and specifications.
 * Binary Restricted: Licensee may sublicense Software as a part of a larger work containing more than Software,
 * distributed solely in Object or Binary form under a personal, non-sublicensable, limited license. Such redistribution shall be limited to unlimited codebases.
 * Non Assignable & Non-Transferable: Licensee may not assign or transfer his rights and duties under this license.
 * Commercial, Royalty Free: Licensee may use Software for any purpose, including paid-services, without any royalties
 * Including the Right to Create Derivative Works: Licensee may create derivative works based on Software,
 * including amending Software’s source code, modifying it, integrating it into a larger work or removing portions of Software,
 * as long as no distribution of the derivative works is made
 *
 * THE RUNTIMES IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE RUNTIMES OR THE USE OR OTHER DEALINGS IN THE
 * RUNTIMES.
 *****************************************************************************/
#include "CreaturePluginPCH.h"
#include "CreatureModule.h"
#include <Runtime/Core/Public/Async/ParallelFor.h>

DECLARE_CYCLE_STAT(TEXT("CreatureManager_Update"), STAT_CreatureManager_Update, STATGROUP_Creature);
DECLARE_CYCLE_STAT(TEXT("CreatureManager_IncreRunTime"), STAT_CreatureManager_IncreRunTime, STATGROUP_Creature);
DECLARE_CYCLE_STAT(TEXT("CreatureManager_PoseJustBones"), STAT_CreatureManager_PoseJustBones, STATGROUP_Creature);
DECLARE_CYCLE_STAT(TEXT("CreatureManager_PoseCreature"), STAT_CreatureManager_PoseCreature, STATGROUP_Creature);
DECLARE_CYCLE_STAT(TEXT("CreatureManager_RunUVItemSwap"), STAT_CreatureManager_RunUVItemSwap, STATGROUP_Creature);
DECLARE_CYCLE_STAT(TEXT("CreatureManager_AlterBonesByAnchor"), STAT_CreatureManager_AlterBonesByAnchor, STATGROUP_Creature);
DECLARE_CYCLE_STAT(TEXT("CreatureManager_JustRunUVWarps"), STAT_CreatureManager_JustRunUVWarps, STATGROUP_Creature);

template <typename T>
static T clipNum(const T& n, const T& lower, const T& upper) {
	return FMath::Clamp(n, lower, upper);
}

static JsonNode * GetJSONLevelNodeFromKey(JsonNode& json_obj,
                                          const FName& key)
{
    JsonNode * ret_node = NULL;
    if(FName(json_obj.key) == key) {
        ret_node = &json_obj;
    }
    else {
        JsonNode * cur_node = &json_obj;
        while(true) {
            if(FName(cur_node->key) == key) {
                ret_node = cur_node;
                break;
            }
            
            JsonNode * next_node = cur_node->next;
            if(next_node == NULL) {
                break;
            }
            
            cur_node = next_node;
        }
    }
    
    return ret_node;
}

static JsonNode * GetJSONNodeFromKey(JsonNode& json_obj,
                                     const FName& key)
{
    JsonNode * ret_node = NULL;
    
    
    for(JsonIterator it = JsonBegin(json_obj.value);
        it != JsonEnd(json_obj.value); ++it)
    {
        if( FName((*it)->key) == key) {
            ret_node = *it;
            break;
        }
    }
    
    return ret_node;
}

static TArray<FName> GetJSONKeysFromNode(JsonNode& json_obj)
{
    TArray<FName> ret_keys;
    for(JsonIterator it = JsonBegin(json_obj.value);
        it != JsonEnd(json_obj.value); ++it)
    {
        ret_keys.Add(FName((*it)->key));
    }
    
    return ret_keys;
}

static TArray<float> GetJSONNodeFloatArray(JsonNode& json_obj)
{
    TArray<float> ret_vals;
    for(JsonIterator it = JsonBegin(json_obj.value);
        it != JsonEnd(json_obj.value); ++it)
    {
        JsonNode * cur_node = *it;
        ret_vals.Add((float)cur_node->value.toNumber());
    }
    
    return ret_vals;
}

static TArray<int32> GetJSONNodeIntArray(JsonNode& json_obj)
{
    TArray<int32> ret_vals;
    for(JsonIterator it = JsonBegin(json_obj.value);
        it != JsonEnd(json_obj.value); ++it)
    {
        JsonNode * cur_node = *it;
        ret_vals.Add((int32)cur_node->value.toNumber());
    }
    
    return ret_vals;
}

static glm::float32 * ReadJSONPoints3D(JsonNode& json_obj,
                                       const FName& key,
                                       int32& num_pts)
{
    TArray<float> pts_array = GetJSONNodeFloatArray(*GetJSONNodeFromKey(json_obj, key));
    num_pts = (int32)pts_array.Num() / 2; // Since we are storing them as x,y pairs
    
    glm::float32 * ret_pts = new glm::float32[num_pts * 3];
    
    int32 cur_index = 0;
    for(auto i = 0; i < pts_array.Num(); i+=2)
    {
        ret_pts[cur_index] = pts_array[i];
        ret_pts[cur_index + 1] = pts_array[i + 1];
        ret_pts[cur_index + 2] = 0;
        
        cur_index += 3;
    }
    
    
    return ret_pts;
}

static glm::float32 * ReadJSONPoints2D(JsonNode& json_obj,
                                       const FName& key,
                                       int32& num_pts)
{
    TArray<float> pts_array = GetJSONNodeFloatArray(*GetJSONNodeFromKey(json_obj, key));
    glm::float32 * ret_pts = new glm::float32[pts_array.Num()];
    for(auto i = 0; i < pts_array.Num(); i++)
    {
        ret_pts[i] = pts_array[i];
    }
    
    num_pts = (int32)pts_array.Num() / 2; // Since we are storing them as x,y pairs
    
    return ret_pts;
}

static TArray<glm::vec2> ReadJSONPoints2DVector(JsonNode& json_obj,
                                                     const FName& key)
{
    TArray<float> pts_array = GetJSONNodeFloatArray(*GetJSONNodeFromKey(json_obj, key));
	TArray<glm::vec2> ret_pts;
	ret_pts.SetNumZeroed(pts_array.Num() / 2);
    
    int32 pt_index = 0;
    for(auto i = 0; i < pts_array.Num(); i+=2)
    {
        ret_pts[pt_index].x = pts_array[i];
        ret_pts[pt_index].y = pts_array[i + 1];
        
        pt_index++;
    }
    
    return ret_pts;
}

static glm::uint32 * ReadJSONUints(JsonNode& json_obj,
                                   const FName& key,
                                   int32& num_ints)
{
    TArray<int32> ints_array = GetJSONNodeIntArray(*GetJSONNodeFromKey(json_obj, key));
    glm::uint32 * ret_pts = new glm::uint32[ints_array.Num()];
    for(auto i = 0; i < ints_array.Num(); i++)
    {
        ret_pts[i] = (glm::uint32)ints_array[i];
    }
    
    num_ints = (int32)ints_array.Num();
    
    return ret_pts;
}

static glm::vec2 ReadJSONVec2(JsonNode& json_obj,
                                const FName& key)
{
    TArray<float> read_array = GetJSONNodeFloatArray(*GetJSONNodeFromKey(json_obj, key));
    return glm::vec2(read_array[0], read_array[1]);
}

static glm::vec4 ReadJSONVec4_2(JsonNode& json_obj,
                                const FName& key)
{
    TArray<float> read_array = GetJSONNodeFloatArray(*GetJSONNodeFromKey(json_obj, key));
    return glm::vec4(read_array[0], read_array[1], 0, 1.0f);
}

static glm::mat4 ReadJSONMat4(JsonNode& json_obj,
                              const FName& key)
{
    TArray<float> read_array = GetJSONNodeFloatArray(*GetJSONNodeFromKey(json_obj, key));
    float mat_vals[16];
    for(int32 i = 0; i < 16; i++) {
        mat_vals[i] = read_array[i];
    }
    
    return glm::make_mat4(mat_vals);
}

static TArray<int32> ReadIntArray(JsonNode& json_obj,
                                     const FName& key)
{
    TArray<int32> read_array = GetJSONNodeIntArray(*GetJSONNodeFromKey(json_obj, key));;
    TArray<int32> ret_array;
    ret_array.SetNumZeroed(read_array.Num());
    for(auto i = 0; i < read_array.Num(); i++)
    {
        ret_array[i] = read_array[i];
    }
    
    return ret_array;
}

static TArray<float> ReadFloatArray(JsonNode& json_obj,
                                     const FName& key)
{
    TArray<float> read_array = GetJSONNodeFloatArray(*GetJSONNodeFromKey(json_obj, key));
    TArray<float> ret_array;
    ret_array.SetNumZeroed(read_array.Num());
    for(auto i = 0; i < read_array.Num(); i++)
    {
        ret_array[i] = read_array[i];
    }
    
    return ret_array;
}

static meshBone * CreateBones(JsonNode& json_obj,
                              const FName& key)
{
    meshBone * root_bone = NULL;
    JsonNode * base_obj =  GetJSONLevelNodeFromKey(json_obj, key);
    TMap<int32, std::pair<meshBone *, TArray<int32> > > bone_data;
    TSet<int32> child_set;

    // layout bones
    for (JsonIterator it = JsonBegin(base_obj->value);
         it != JsonEnd(base_obj->value);
         ++it)
    {
        JsonNode * cur_node = *it;
        
        FName cur_name(cur_node->key);
        int32 cur_id =  (int32)GetJSONNodeFromKey(*cur_node, "id")->value.toNumber();
        glm::mat4 cur_parent_mat = ReadJSONMat4(*cur_node, "restParentMat");
        glm::vec4 cur_local_rest_start_pt = ReadJSONVec4_2(*cur_node, "localRestStartPt");
        glm::vec4 cur_local_rest_end_pt = ReadJSONVec4_2(*cur_node, "localRestEndPt");
        TArray<int32> cur_children_ids = ReadIntArray(*cur_node, "children");
        
        meshBone * new_bone = new meshBone(cur_name,
                                           glm::vec4(0),
                                           glm::vec4(0),
                                           cur_parent_mat);
        new_bone->getLocalRestStartPt() = cur_local_rest_start_pt;
        new_bone->getLocalRestEndPt() = cur_local_rest_end_pt;
        new_bone->calcRestData();
        new_bone->setTagId(cur_id);
        
        bone_data.Add(cur_id, std::make_pair(new_bone, cur_children_ids));
        
        for(auto& cur_child_id : cur_children_ids) {
            child_set.Add(cur_child_id);
        }
    }
    
    // Find root
    for(auto& cur_data : bone_data)
    {
        int32 cur_id = cur_data.Key;
        if(child_set.Contains(cur_id) == false) {
            // not a child, so is root
            root_bone = cur_data.Value.first;
            break;
        }
    }
    
    // construct hierarchy
    for(auto& cur_data : bone_data)
    {
        meshBone * cur_bone = cur_data.Value.first;
        const TArray<int32>& children_ids = cur_data.Value.second;
        for(auto& cur_child_id : children_ids)
        {
            meshBone * child_bone = bone_data[cur_child_id].first;
            cur_bone->addChild(child_bone);
        }

    }
    
    return root_bone;
}

static TArray<meshRenderRegion *> CreateRegions(JsonNode& json_obj,
                                                     const FName& key,
                                                     glm::uint32 * indices_in,
                                                     glm::float32 * rest_pts_in,
                                                     glm::float32 * uvs_in)
{
    TArray<meshRenderRegion *> ret_regions;
    JsonNode * base_obj =  GetJSONNodeFromKey(json_obj, key);

    for (JsonIterator it = JsonBegin(base_obj->value);
         it != JsonEnd(base_obj->value);
         ++it)
    {
        JsonNode * cur_node = *it;

        FName cur_name(cur_node->key);
        
        int32 cur_id = (int32)GetJSONNodeFromKey(*cur_node, "id")->value.toNumber();
        int32 cur_start_pt_index = (int32)GetJSONNodeFromKey(*cur_node, "start_pt_index")->value.toNumber();
        int32 cur_end_pt_index = (int32)GetJSONNodeFromKey(*cur_node, "end_pt_index")->value.toNumber();
        int32 cur_start_index = (int32)GetJSONNodeFromKey(*cur_node, "start_index")->value.toNumber();
        int32 cur_end_index = (int32)GetJSONNodeFromKey(*cur_node, "end_index")->value.toNumber();
     
        meshRenderRegion * new_region = new meshRenderRegion(indices_in,
                                                             rest_pts_in,
                                                             uvs_in,
                                                             cur_start_pt_index,
                                                             cur_end_pt_index,
                                                             cur_start_index,
                                                             cur_end_index);
        
        new_region->setName(cur_name);
        new_region->setTagId(cur_id);
        
        // Read in weights
        TMap<FName, TArray<float> >& weight_map =
            new_region->getWeights();
        JsonNode * weight_obj =  GetJSONNodeFromKey(*cur_node, "weights");
        
        for (JsonIterator w_it = JsonBegin(weight_obj->value);
             w_it != JsonEnd(weight_obj->value);
             ++w_it)
        {
            JsonNode * w_node = *w_it;
            
            FName cur_key(w_node->key);
            TArray<float> values = ReadFloatArray(*weight_obj, cur_key);
            weight_map.Add(cur_key, values);
        }
        
        ret_regions.Add(new_region);
    }
    
    return ret_regions;
}

static std::pair<int32, int32> GetStartEndTimes(JsonNode& json_obj,
                                            const FName& key)
{
    std::pair<int32, int32> ret_times(0,0);
    bool first = true;
    JsonNode * base_obj =  GetJSONNodeFromKey(json_obj, key);
    
    for (JsonIterator it = JsonBegin(base_obj->value);
         it != JsonEnd(base_obj->value);
         ++it)
    {
        JsonNode * cur_node = *it;

        int32 cur_val = atoi(cur_node->key);
        if(first) {
            ret_times.first = cur_val;
            ret_times.second = cur_val;
            first = false;
        }
        else {
            if(cur_val > ret_times.second) {
                ret_times.second = cur_val;
            }
            
            if(cur_val < ret_times.first) {
                ret_times.first = cur_val;
            }
        }
    }
    
    return ret_times;
}

static void FillBoneCache(JsonNode& json_obj,
                          const FName& key,
                          int32 start_time,
                          int32 end_time,
                          meshBoneCacheManager& cache_manager)
{
    JsonNode * base_obj =  GetJSONNodeFromKey(json_obj, key);

    cache_manager.init(start_time, end_time);
    
	int32 prev_time = start_time;
    for (JsonIterator it = JsonBegin(base_obj->value);
         it != JsonEnd(base_obj->value);
         ++it)
    {
        JsonNode * cur_node = *it;

        int32 cur_time = atoi(cur_node->key);
        TArray<meshBoneCache> cache_list;
        
        for (JsonIterator bone_it = JsonBegin(cur_node->value);
             bone_it != JsonEnd(cur_node->value);
             ++bone_it)
        {
            JsonNode * bone_node = *bone_it;
            
			FName cur_name(bone_node->key);
            glm::vec4 cur_start_pt = ReadJSONVec4_2(*bone_node, "start_pt");
            glm::vec4 cur_end_pt = ReadJSONVec4_2(*bone_node, "end_pt");
            
            meshBoneCache cache_data(cur_name);
            cache_data.setWorldStartPt(cur_start_pt);
            cache_data.setWorldEndPt(cur_end_pt);
            
            cache_list.Add(cache_data);
        }
        
        int32 set_index = cache_manager.getIndexByTime(cur_time);
        cache_manager.getCacheTable()[set_index] = cache_list;

		int32 gap_diff = cur_time - prev_time;
		if (gap_diff > 1)
		{
			auto ptInterp = [&](const glm::vec4& src_pt, const glm::vec4& target_pt, float fraction)
			{
				return ((1.0f - fraction) * src_pt) + (fraction * target_pt);
			};

			// Gap Step
			auto prev_index = cache_manager.getIndexByTime(prev_time);
			for (int32 j = 1; j < gap_diff; j++)
			{
				auto gap_fraction = (float)j / (float)gap_diff;
				TArray<meshBoneCache> gap_cache_list;
				
				for (int32 k = 0; k < cache_list.Num(); k++)
				{
					auto& cur_data = cache_manager.getCacheTable()[set_index][k];
					auto& prev_data = cache_manager.getCacheTable()[prev_index][k];
					meshBoneCache gap_cache_data(cur_data.getKey());
					gap_cache_data.setWorldStartPt(
						ptInterp(prev_data.getWorldStartPt(), cur_data.getWorldStartPt(), gap_fraction));
					gap_cache_data.setWorldEndPt(
						ptInterp(prev_data.getWorldEndPt(), cur_data.getWorldEndPt(), gap_fraction));

					gap_cache_list.Add(gap_cache_data);
				}

				cache_manager.getCacheTable()[prev_index + j] = gap_cache_list;
			}
		}

		prev_time = cur_time;
	}
    
    cache_manager.makeAllReady();
}

static void FillDeformationCache(JsonNode& json_obj,
                          const FName& key,
                          int32 start_time,
                          int32 end_time,
                          meshDisplacementCacheManager& cache_manager)
{
    JsonNode * base_obj =  GetJSONNodeFromKey(json_obj, key);

    cache_manager.init(start_time, end_time);
    
	int prev_time = start_time;
	for (JsonIterator it = JsonBegin(base_obj->value);
         it != JsonEnd(base_obj->value);
         ++it)
    {
        JsonNode * cur_node = *it;

        int32 cur_time = atoi(cur_node->key);
        TArray<meshDisplacementCache> cache_list;
        
        for (JsonIterator mesh_it = JsonBegin(cur_node->value);
             mesh_it != JsonEnd(cur_node->value);
             ++mesh_it)
        {
            JsonNode * mesh_node = *mesh_it;
            
            FName cur_name(mesh_node->key);
            
            meshDisplacementCache cache_data(cur_name);
            
            bool use_local_displacement = GetJSONNodeFromKey(*mesh_node, "use_local_displacements")->value.toBool();
            bool use_post_displacement = GetJSONNodeFromKey(*mesh_node, "use_post_displacements")->value.toBool();
            
            if(use_local_displacement) {
                TArray<glm::vec2> read_pts = ReadJSONPoints2DVector(*mesh_node, "local_displacements");
                cache_data.setLocalDisplacements(read_pts);
            }
            
            if(use_post_displacement) {
                TArray<glm::vec2> read_pts = ReadJSONPoints2DVector(*mesh_node, "post_displacements");
                cache_data.setPostDisplacements(read_pts);
            }
            
            cache_list.Add(cache_data);
        }
        
        int32 set_index = cache_manager.getIndexByTime(cur_time);
        cache_manager.getCacheTable()[set_index] = cache_list;

		auto gap_diff = cur_time - prev_time;
		if (gap_diff > 1)
		{
			auto ptsInterp = [&](const TArray<glm::vec2> src_pts, const TArray<glm::vec2> target_pts, float fraction)
			{
				TArray<glm::vec2> ret_pts;
				ret_pts.SetNum(src_pts.Num());
				for (int32 i = 0; i < src_pts.Num(); i++)
				{
					ret_pts[i] = ((1.0f - fraction) * src_pts[i]) + (fraction * target_pts[i]);
				}

				return ret_pts;
			};

			// Gap Step
			int32 prev_index = cache_manager.getIndexByTime(prev_time);
			for (int32 j = 1; j < gap_diff; j++)
			{
				auto gap_fraction = (float)j / (float)gap_diff;
				TArray<meshDisplacementCache> gap_cache_list;
				
				for (size_t k = 0; k < cache_list.Num(); k++)
				{
					auto& cur_data = cache_manager.getCacheTable()[set_index][k];
					auto& prev_data = cache_manager.getCacheTable()[prev_index][k];
					meshDisplacementCache gap_cache_data(cur_data.getKey());
					if (cur_data.getLocalDisplacements().Num() > 0)
					{
						gap_cache_data.setLocalDisplacements(
							ptsInterp(prev_data.getLocalDisplacements(), cur_data.getLocalDisplacements(), gap_fraction));
					}
					else {
						gap_cache_data.setPostDisplacements(
							ptsInterp(prev_data.getPostDisplacements(), cur_data.getPostDisplacements(), gap_fraction));
					}

					gap_cache_list.Add(gap_cache_data);
				}

				cache_manager.getCacheTable()[prev_index + j] = gap_cache_list;
			}
		}

		prev_time = cur_time;
    }
    
    cache_manager.makeAllReady();
}


static void FillUVSwapCache(JsonNode& json_obj,
                            const FName& key,
                            int32 start_time,
                            int32 end_time,
                            meshUVWarpCacheManager& cache_manager)
{
    JsonNode * base_obj =  GetJSONNodeFromKey(json_obj, key);

    cache_manager.init(start_time, end_time);
    
    for (JsonIterator it = JsonBegin(base_obj->value);
         it != JsonEnd(base_obj->value);
         ++it)
    {
        JsonNode * cur_node = *it;

        int32 cur_time = atoi(cur_node->key);
        TArray<meshUVWarpCache> cache_list;
        
        for (JsonIterator uv_it = JsonBegin(cur_node->value);
             uv_it != JsonEnd(cur_node->value);
             ++uv_it)
        {
            JsonNode * uv_node = *uv_it;
            
            FName cur_name(uv_node->key);

            meshUVWarpCache cache_data(cur_name);
            bool use_uv = GetJSONNodeFromKey(*uv_node, "enabled")->value.toBool();
            cache_data.setEnabled(use_uv);
            if(use_uv) {
                glm::vec2 local_offset = ReadJSONVec2(*uv_node, "local_offset");
                glm::vec2 global_offset = ReadJSONVec2(*uv_node, "global_offset");
                glm::vec2 scale = ReadJSONVec2(*uv_node, "scale");
                cache_data.setUvWarpLocalOffset(local_offset);
                cache_data.setUvWarpGlobalOffset(global_offset);
                cache_data.setUvWarpScale(scale);
            }
            
            cache_list.Add(cache_data);
        }
        
        int32 set_index = cache_manager.getIndexByTime(cur_time);
        cache_manager.getCacheTable()[set_index] = cache_list;
    }
    
    cache_manager.makeAllReady();
}

static void FillOpacityCache(JsonNode& json_obj,
	const FName& key,
	int32 start_time,
	int32 end_time,
	meshOpacityCacheManager& cache_manager)
{
	JsonNode * base_obj = GetJSONNodeFromKey(json_obj, key);

	cache_manager.init(start_time, end_time);

	if (base_obj == NULL)
	{
		return;
	}

	int prev_time = start_time;
	for (JsonIterator it = JsonBegin(base_obj->value);
		it != JsonEnd(base_obj->value);
		++it)
	{
		JsonNode * cur_node = *it;

		int32 cur_time = atoi(cur_node->key);
		TArray<meshOpacityCache> cache_list;

		for (JsonIterator uv_it = JsonBegin(cur_node->value);
			uv_it != JsonEnd(cur_node->value);
			++uv_it)
		{
			JsonNode * opacity_node = *uv_it;

			FName cur_name(opacity_node->key);

			meshOpacityCache cache_data(cur_name);
			float cur_opacity = (float)GetJSONNodeFromKey(*opacity_node, "opacity")->value.toNumber();
			cache_data.setOpacity(cur_opacity);

			cache_list.Add(cache_data);
		}

		int32 set_index = cache_manager.getIndexByTime(cur_time);
		cache_manager.getCacheTable()[set_index] = cache_list;

		int32 gap_diff = cur_time - prev_time;
		if (gap_diff > 1)
		{
			auto scalarInterp = [&](const float& src_val, const float& target_val, float fraction)
			{
				return ((1.0f - fraction) * src_val) + (fraction * target_val);
			};

			// Gap Step
			int32 prev_index = cache_manager.getIndexByTime(prev_time);
			for (int32 j = 1; j < gap_diff; j++)
			{
				auto gap_fraction = (float)j / (float)gap_diff;
				TArray<meshOpacityCache> gap_cache_list;
				
				for (int32 k = 0; k < cache_list.Num(); k++)
				{
					auto& cur_data = cache_manager.getCacheTable()[set_index][k];
					auto& prev_data = cache_manager.getCacheTable()[prev_index][k];
					meshOpacityCache gap_cache_data(cur_data.getKey());
					gap_cache_data.setOpacity(scalarInterp(prev_data.getOpacity(), cur_data.getOpacity(), gap_fraction));

					gap_cache_list.Add(gap_cache_data);
				}

				cache_manager.getCacheTable()[prev_index + j] = gap_cache_list;
			}
		}

		prev_time = cur_time;
	}

	cache_manager.makeAllReady();

}

static TMap<FName, TArray<CreatureModule::CreatureUVSwapPacket> >
FillSwapUVPacketMap(JsonNode& json_obj)
{
	TMap<FName, TArray<CreatureModule::CreatureUVSwapPacket> > ret_map;
	for (JsonIterator it = JsonBegin(json_obj.value);
		it != JsonEnd(json_obj.value);
		++it)
	{
		JsonNode * cur_node = *it;
		FName cur_name(cur_node->key);
		TArray<CreatureModule::CreatureUVSwapPacket> cur_packets;

		for (JsonIterator node_it = JsonBegin(cur_node->value); 
			node_it != JsonEnd(cur_node->value);
			++node_it)
		{
			JsonNode * packet_node = *node_it;
			CreatureModule::CreatureUVSwapPacket new_packet(ReadJSONVec2(*packet_node, "local_offset"),
				ReadJSONVec2(*packet_node, "global_offset"),
				ReadJSONVec2(*packet_node, "scale"),
				(int32)GetJSONNodeFromKey(*packet_node, "tag")->value.toNumber());

			cur_packets.Add(new_packet);
		}

		
		ret_map.Add(cur_name, cur_packets);
	}

	return ret_map;
}

static TMap<FName, glm::vec2>
FillAnchorPointMap(JsonNode& json_obj)
{
	auto anchor_data_node = GetJSONNodeFromKey(json_obj, "AnchorPoints");

	TMap<FName, glm::vec2> ret_map;
	for (JsonIterator it = JsonBegin(anchor_data_node->value);
	it != JsonEnd(anchor_data_node->value);
		++it)
	{
		JsonNode * cur_node = *it;
		glm::vec2 cur_pt = ReadJSONVec2(*cur_node, "point");
		FName cur_name(GetJSONNodeFromKey(*cur_node, "anim_clip_name")->value.toString());

		ret_map.Add(cur_name, cur_pt);
	}

	return ret_map;
}


namespace CreatureModule {
    // Load the json structure
    void LoadCreatureJSONData(const FName& filename_in,
                              CreatureLoadDataPacket& load_data)
    {
        std::ifstream read_file;
        read_file.open(TCHAR_TO_UTF8(*filename_in.ToString()));
        std::stringstream str_stream;
        str_stream << read_file.rdbuf();
        read_file.close();
        
        LoadCreatureJSONDataFromString(str_stream.str().c_str(), load_data);
    }
    
    void LoadCreatureJSONDataFromString(const FString& string_in,
                                        CreatureLoadDataPacket& load_data)
    {
        char *endptr;
        size_t source_size = string_in.Len();
        char *source_chars = new char[source_size+1];
        source_chars[source_size]=0;
        memcpy(source_chars, TCHAR_TO_UTF8(*string_in), source_size);
        
        JsonParseStatus status = jsonParse(source_chars, &endptr, &load_data.base_node, load_data.allocator);
        
        load_data.src_chars = source_chars;
        
        if(status != JSON_PARSE_OK) {
            std::cerr<<"LoadCreatureJSONData() - Error parsing JSON!"<<std::endl;
        }
    }
    
    void LoadCreatureZipJSONData(const FName& filename_in,
                                 CreatureLoadDataPacket& load_data)
    {
		std::cout << "LoadCreatureZipJSONData() - Function is NOT DEFINED!" << std::endl;
    }

    // Creature class
    Creature::Creature(CreatureLoadDataPacket& load_data)
    {
		anchor_points_active = false;
        LoadFromData(load_data);
    }
    
    Creature::~Creature()
    {
        delete [] global_pts;
        delete [] global_indices;
        delete [] global_uvs;
        delete [] render_colours;
        delete render_composition;
        delete [] render_pts;

		global_pts = nullptr;
		global_indices = nullptr;
		global_uvs = nullptr;
		render_composition = nullptr;
		render_pts = nullptr;
    }
    
    glm::uint32 *
    Creature::GetGlobalIndices()
    {
        return global_indices;
    }
    
    glm::float32 *
    Creature::GetGlobalPts()
    {
        return global_pts;
    }
    
    glm::float32 *
    Creature::GetGlobalUvs()
    {
        return global_uvs;
    }
    
    glm::float32 *
    Creature::GetRenderPts()
    {
        return render_pts;
    }
    
    glm::uint8 *
    Creature::GetRenderColours()
    {
        return render_colours;
    }
    
    int32
    Creature::GetTotalNumPoints() const
    {
        return total_num_pts;
    }
    
    int32
    Creature::GetTotalNumIndices() const
    {
        return total_num_indices;
    }
    
    meshRenderBoneComposition *
    Creature::GetRenderComposition()
    {
        return render_composition;
    }
    
    void
    Creature::FillRenderColours(glm::uint8 r, glm::uint8 g, glm::uint8 b, glm::uint8 a)
    {
        for(auto i = 0; i < total_num_pts; i++)
        {
            glm::uint8 * cur_colour = render_colours + (i * 4);
            cur_colour[0] = r;
            cur_colour[1] = g;
            cur_colour[2] = b;
            cur_colour[3] = a;
        }
    }

    const TArray<FName>& 
    Creature::GetAnimationNames() const
    {
        return animation_names;
    }

	const TMap<FName, TArray<CreatureUVSwapPacket> >& 
	Creature::GetUvSwapPackets() const
	{
		return uv_swap_packets;
	}

	void 
	Creature::SetActiveItemSwap(const FName& region_name, int32 swap_idx)
	{
		active_uv_swap_actions.Add(region_name, swap_idx);
	}

	void 
	Creature::RemoveActiveItemSwap(const FName& region_name)
	{
		active_uv_swap_actions.Remove(region_name);
	}

	TMap<FName, int32>&
	Creature::GetActiveItemSwaps()
	{
		return active_uv_swap_actions;
	}

	void Creature::SetAnchorPointsActive(bool flag_in)
	{
		anchor_points_active = flag_in;
	}

	bool Creature::GetAnchorPointsActive() const
	{
		return anchor_points_active;
	}

	glm::vec2 Creature::GetAnchorPoint(const FName & anim_clip_name_in) const
	{
		if (anchor_point_map.Contains(anim_clip_name_in))
		{
			return anchor_point_map[anim_clip_name_in];
		}

		return glm::vec2(0, 0);
	}

    void
    Creature::LoadFromData(CreatureLoadDataPacket& load_data)
    {
        JsonNode * json_root = load_data.base_node.toNode();
        
        // Load points and topology
        JsonNode * json_mesh = GetJSONLevelNodeFromKey(*json_root, "mesh");
        global_pts = ReadJSONPoints3D(*json_mesh, "points", total_num_pts);
        global_indices = ReadJSONUints(*json_mesh,"indices", total_num_indices);
        global_uvs = ReadJSONPoints2D(*json_mesh, "uvs", total_num_pts);
        
        render_colours = new glm::uint8[total_num_pts * 4];
        render_pts = new glm::float32[total_num_pts * 3];
        FillRenderColours(255, 255, 255, 255);
        
        // Load bones
        meshBone * root_bone = CreateBones(*json_root, "skeleton");
        
        // Load regions
        TArray<meshRenderRegion *> regions = CreateRegions(*json_mesh,
                                                                "regions",
                                                                global_indices,
                                                                global_pts,
                                                                global_uvs);
        
        // Add into composition
        render_composition = new meshRenderBoneComposition();
        render_composition->setRootBone(root_bone);
        render_composition->getRootBone()->computeRestParentTransforms();
        
        for(auto& cur_region : regions) {
            cur_region->setMainBoneKey(root_bone->getKey());
            cur_region->determineMainBone(root_bone);
            render_composition->addRegion(cur_region);
        }
        
        render_composition->initBoneMap();
        render_composition->initRegionsMap();
        
        for(auto& cur_region : regions) {
            cur_region->initFastNormalWeightMap(render_composition->getBonesMap());
        }
        
        render_composition->resetToWorldRestPts();

        // Fill up available animation names
        JsonNode * json_anim_base = GetJSONLevelNodeFromKey(*json_root, "animation");
        animation_names = GetJSONKeysFromNode(*json_anim_base);

		// Fill up uv swap packets
		
		JsonNode * json_uv_swap_base = GetJSONLevelNodeFromKey(*json_root, "uv_swap_items");
		if (json_uv_swap_base)
		{
			uv_swap_packets = FillSwapUVPacketMap(*json_uv_swap_base);
		}

		// Load Anchor Points
		JsonNode * anchor_point_base = GetJSONLevelNodeFromKey(*json_root, "anchor_points_items");
		if (anchor_point_base)
		{
			anchor_point_map = FillAnchorPointMap(*anchor_point_base);
		}
    }

    
    // CreatureAnimation class
    CreatureAnimation::CreatureAnimation(CreatureLoadDataPacket& load_data,
                                         const FName& name_in)
    : name(name_in)
    {
            LoadFromData(name_in, load_data);
    }
    
    CreatureAnimation::~CreatureAnimation()
    {
        if(cache_pts.Num() > 0)
        {
            for(auto pt_pool : cache_pts)
            {
                delete [] pt_pool;
            }
        }
    }
    
    float CreatureAnimation::getStartTime() const
    {
        return start_time;
    }
    
    float CreatureAnimation::getEndTime() const
    {
        return end_time;
    }

	void CreatureAnimation::setStartTime(int32 value_in)
	{
		start_time = (float)value_in;
	}

	void CreatureAnimation::setEndTime(int32 value_in)
	{
		end_time = (float)value_in;
	}
    
    meshBoneCacheManager&
    CreatureAnimation::getBonesCache()
    {
        return bones_cache;
    }
    
    meshDisplacementCacheManager&
    CreatureAnimation::getDisplacementCache()
    {
        return displacement_cache;
    }
    
    meshUVWarpCacheManager&
    CreatureAnimation::getUVWarpCache()
    {
        return uv_warp_cache;
    }

	meshOpacityCacheManager& 
	CreatureAnimation::getOpacityCache()
	{
		return opacity_cache;
	}
    
    const FName&
    CreatureAnimation::getName() const
    {
        return name;
    }

    void
    CreatureAnimation::LoadFromData(const FName& name_in,
                                    CreatureLoadDataPacket& load_data)
    {
        JsonNode * json_root = load_data.base_node.toNode();
        JsonNode * json_anim_base = GetJSONLevelNodeFromKey(*json_root, "animation");
        JsonNode * json_clip = GetJSONNodeFromKey(*json_anim_base, name_in);
        
        std::pair<int32, int32> start_end_times = GetStartEndTimes(*json_clip, "bones");
        start_time = (float)start_end_times.first;
        end_time = (float)start_end_times.second;
        
        // bone animation
        FillBoneCache(*json_clip,
                      "bones",
                      (int32)start_time,
					  (int32)end_time,
                      bones_cache);
        
        // mesh deformation animation
        FillDeformationCache(*json_clip,
                             "meshes",
							 (int32)start_time,
							 (int32)end_time,
                             displacement_cache);
        
        // uv swapping animation
        FillUVSwapCache(*json_clip,
                        "uv_swaps",
						(int32)start_time,
						(int32)end_time,
                        uv_warp_cache);

		// opacity animation
		FillOpacityCache(*json_clip,
			"mesh_opacities",
			(int32)start_time,
			(int32)end_time,
			opacity_cache);
    }
    
    bool
    CreatureAnimation::hasCachePts() const
    {
        return cache_pts.Num() > 0;
    }
    
    TArray<glm::float32 *>&
    CreatureAnimation::getCachePts()
    {
        return cache_pts;
    }

	void 
	CreatureAnimation::clearCachePts()
	{
		for (size_t i = 0; i < cache_pts.Num(); i++)
		{
			delete[] cache_pts[i];
		}

		cache_pts.Empty();
	}
    
    int32
    CreatureAnimation::getIndexByTime(int32 time_in) const
    {
        int32 retval = time_in - (int32)start_time;
        retval = clipNum(retval, 0, (int32)cache_pts.Num() - 1);
        
        return retval;
    }
    
    void
    CreatureAnimation::poseFromCachePts(float time_in, glm::float32 * target_pts, int32 num_pts)
    {
        int32 cur_floor_time = getIndexByTime((int32)floorf(time_in));
        int32 cur_ceil_time = getIndexByTime((int32)ceilf(time_in));
        float cur_ratio = (time_in - (float)floorf(time_in));       
        
#ifdef CREATURE_MULTICORE
		ParallelFor(num_pts, [&](int32 i) {
#else
		for (int32 i = 0; i < num_pts; i++) {
#endif
			glm::float32 * set_pt = target_pts + (i * 3);
			glm::float32 * floor_pts = cache_pts[cur_floor_time] + (i * 3);
			glm::float32 * ceil_pts = cache_pts[cur_ceil_time] + (i * 3);

			set_pt[0] = ((1.0f - cur_ratio) * floor_pts[0]) + (cur_ratio * ceil_pts[0]);
			set_pt[1] = ((1.0f - cur_ratio) * floor_pts[1]) + (cur_ratio * ceil_pts[1]);
			set_pt[2] = ((1.0f - cur_ratio) * floor_pts[2]) + (cur_ratio * ceil_pts[2]);
#ifdef CREATURE_MULTICORE
		});
#else
		}
#endif
        
    }
    
    // CreatureManager class
    CreatureManager::CreatureManager(TSharedPtr<CreatureModule::Creature> target_creature_in)
    : target_creature(target_creature_in), is_playing(false), run_time(0), time_scale(30.0),
        do_blending(false),
        blending_factor(0), mirror_y(false), use_custom_time_range(false),
        custom_start_time(0), custom_end_time(0), should_loop(true),
        do_auto_blending(false), auto_blend_delta(0.1f), do_point_caching(false)
    {
        for(int32 i = 0; i < 2; i++) {
            blend_render_pts[i] = NULL;
        }
    }
    
    CreatureManager::~CreatureManager()
    {
        for(int32 i = 0; i < 2; i++) {
            if(blend_render_pts[i] != NULL) {
                delete [] blend_render_pts[i];
            }
        }
    }
    
    void
    CreatureManager::AddAnimation(TSharedPtr<CreatureModule::CreatureAnimation> animation_in)
    {
        animations.Add(animation_in->getName(), animation_in);
		active_blend_run_times.Add(animation_in->getName(), animation_in->getStartTime());
    }
    
    void
    CreatureManager::CreateAnimation(CreatureLoadDataPacket& load_data,
                                     const FName& name_in)
    {
        auto new_animation = TSharedPtr<CreatureModule::CreatureAnimation>(new CreatureAnimation(load_data,
                                                                                                      name_in));
        AddAnimation(new_animation);
    }

    
    CreatureModule::CreatureAnimation *
    CreatureManager::GetAnimation(const FName name_in)
    {
        if(animations.Contains(name_in)) {
            return animations[name_in].Get();
        }
        
        return NULL;
    }

    CreatureModule::Creature *
    CreatureManager::GetCreature()
    {
        return target_creature.Get();
    }

    void
    CreatureManager::SetActiveAnimationName(const FName& name_in, bool check_already_active)
    {
        if(check_already_active)
        {
            if(active_animation_name == name_in)
            {
                // animation clip already set to this name, so just return
                return;
            }
        }
        
        if(animations.Contains(name_in)) {
            active_animation_name = name_in;
            auto& cur_animation = animations[active_animation_name];
            run_time = cur_animation->getStartTime();
            
			UpdateRegionSwitches(name_in);
        }
    }

	void 
	CreatureManager::UpdateRegionSwitches(const FName& animation_name_in)
	{
		if (animations.Contains(animation_name_in)) {
			auto& cur_animation = animations[animation_name_in];

			auto& displacement_cache_manager = cur_animation->getDisplacementCache();
			TArray<meshDisplacementCache>& displacement_table =
				displacement_cache_manager.getCacheTable()[0];

			auto& uv_warp_cache_manager = cur_animation->getUVWarpCache();
			TArray<meshUVWarpCache>& uv_swap_table =
				uv_warp_cache_manager.getCacheTable()[0];

			meshRenderBoneComposition * render_composition =
				target_creature->GetRenderComposition();
			TArray<meshRenderRegion *>& all_regions = render_composition->getRegions();

			int32 index = 0;
			for (auto& cur_region : all_regions) {
				// Setup active or inactive displacements
				bool use_local_displacements = (displacement_table[index].getLocalDisplacements().Num() > 0);
				bool use_post_displacements = (displacement_table[index].getPostDisplacements().Num() > 0);
				cur_region->setUseLocalDisplacements(use_local_displacements);
				cur_region->setUsePostDisplacements(use_post_displacements);

				// Setup active or inactive uv swaps
				cur_region->setUseUvWarp(uv_swap_table[index].getEnabled());

				index++;
			}
		}
	}

    const FName&
    CreatureManager::GetActiveAnimationName() const
    {
        return active_animation_name;
    }

    TMap<FName, TSharedPtr<CreatureModule::CreatureAnimation> >&
    CreatureManager::GetAllAnimations()
    {
        return animations;
    }

    bool
    CreatureManager::GetIsPlaying() const
    {
        return is_playing;
    }
    
    void
    CreatureManager::SetIsPlaying(bool flag_in)
    {
        is_playing = flag_in;
    }
    
    void
    CreatureManager::SetTimeScale(float scale_in)
    {
        time_scale = scale_in;
    }

	float
	CreatureManager::GetTimeScale() const
	{
		return time_scale;
	}
    
    void
    CreatureManager::SetBlending(bool flag_in)
    {
        do_blending = flag_in;
        
        if(do_blending) {
            if(blend_render_pts[0] == NULL) {
                blend_render_pts[0] = new glm::float32[target_creature->GetTotalNumPoints() * 3];
            }
            
            if(blend_render_pts[1] == NULL) {
                blend_render_pts[1] = new glm::float32[target_creature->GetTotalNumPoints() * 3];
            }
        }
    }
    
    void
    CreatureManager::SetBlendingAnimations(const FName& name_1, const FName& name_2)
    {
        active_blend_animation_names[0] = name_1;
        active_blend_animation_names[1] = name_2;
    }
    
    void
    CreatureManager::SetAutoBlending(bool flag_in)
    {
        do_auto_blending = flag_in;
        SetBlending(flag_in);
        
        if(do_auto_blending)
        {
            AutoBlendTo(active_animation_name, 0.1f);
        }
    }
    
    void
    CreatureManager::AutoBlendTo(const FName& animation_name_in, float blend_delta)
    {
        if(animation_name_in == auto_blend_names[1])
        {
            // already blending to that so just return
            return;
        }

		ResetBlendTime(animation_name_in);
        
        auto_blend_delta = blend_delta;
        auto_blend_names[0] = active_animation_name;
        auto_blend_names[1] = animation_name_in;
        blending_factor = 0;

		active_animation_name = animation_name_in;
        
        SetBlendingAnimations(auto_blend_names[0], auto_blend_names[1]);
    }

	void CreatureManager::SetDoPointCache(bool flag_in)
	{
		do_point_caching = flag_in;
	}

	bool CreatureManager::GetDoPointCache() const
	{
		return do_point_caching;
	}

	void 
	CreatureManager::ResetBlendTime(const FName& name_in)
	{
		auto& cur_animation = animations[name_in];
		active_blend_run_times[name_in] = cur_animation->getStartTime();
	}

    void
    CreatureManager::ResetToStartTimes()
    {
        if(animations.Contains(active_animation_name) == false)
        {
            return;
        }
        
		// reset non blend time
        auto& cur_animation = animations[active_animation_name];
        run_time = cur_animation->getStartTime();

		// reset blend times too
		for (auto& blend_time_data : active_blend_run_times)
		{
			ResetBlendTime(blend_time_data.Key);
		}
    }

    void
    CreatureManager::SetBlendingFactor(float value_in)
    {
        blending_factor = value_in;
    }

	void 
	CreatureManager::ClearPointCache(const FName& animation_name_in)
	{
		if (animations.Contains(animation_name_in) == false)
		{
			return;
		}

		auto cur_animation = animations[animation_name_in];
		if (cur_animation->hasCachePts())
		{
			cur_animation->clearCachePts();
		}
	}
    
    void
	CreatureManager::MakePointCache(const FName& animation_name_in, int32 gap_step)
    {
		if (animations.Contains(animation_name_in) == false)
		{
			return;
		}

		if (gap_step < 1) {
			gap_step = 1;
		}

        float store_run_time = getRunTime();
        auto cur_animation = animations[animation_name_in];
        if(cur_animation->hasCachePts())
        {
            // cache already generated, just exit
            return;
        }
        
        TArray<glm::float32 *>& cache_pts_list = cur_animation->getCachePts();
		int32 array_size = target_creature->GetTotalNumPoints() * 3;

        UpdateRegionSwitches(animation_name_in);

        //for(int32 i = (int32)cur_animation->getStartTime(); i <= (int32)cur_animation->getEndTime(); i++)
		int32 i = (int32)cur_animation->getStartTime();
		while (true)
        {
            run_time = (float)i;
			auto new_pts = new glm::float32[array_size];
            PoseCreature(animation_name_in, new_pts, getRunTime());
            
			int32 real_step = gap_step;
			if (i + real_step > cur_animation->getEndTime())
			{
				real_step = (int32)cur_animation->getEndTime() - i;
			}

			bool firstCase = real_step > 1;
			bool secondCase = (cache_pts_list.Num() >= 1);
			if (firstCase && secondCase)
			{
				// fill in the gaps
				glm::float32 * prev_pts = cache_pts_list[cache_pts_list.Num() - 1];
				for (int32 j = 0; j < real_step; j++)
				{
					float factor = (float)j / (float)real_step;
					glm::float32 * gap_pts = interpFloatArray(prev_pts, new_pts, factor, array_size);
					cache_pts_list.Add(gap_pts);
				}
			}

			cache_pts_list.Add(new_pts);
			i += real_step;

			if (i > cur_animation->getEndTime() || real_step == 0)
			{
				break;
			}
        }
        
        setRunTime(store_run_time);
    }
    
	glm::float32 * 
	CreatureManager::interpFloatArray(glm::float32 * first_list, glm::float32 * second_list, float factor, int32 array_size)
	{
		glm::float32 * ret_array = new glm::float32[array_size];
		for (int32 i = 0; i < array_size; i++)
		{
			float new_val = ((1.0f - factor) * first_list[i]) + (factor * second_list[i]);
			ret_array[i] = new_val;
		}

		return ret_array;
	}

    void
    CreatureManager::PoseCreature(const FName& animation_name_in,
                                  glm::float32 * target_pts,
								  float input_run_time)
    {
		SCOPE_CYCLE_COUNTER(STAT_CreatureManager_PoseCreature);
        if(animations.Contains(animation_name_in) == false)
        {
#ifndef CREATURE_NO_USE_EXCEPTIONS
            throw "CreatureManager::PoseCreature() - Invalid animation name!";
#endif
            return;
        }
        
        auto& cur_animation = animations[animation_name_in];
        
        auto& bone_cache_manager = cur_animation->getBonesCache();
        auto& displacement_cache_manager = cur_animation->getDisplacementCache();
        auto& uv_warp_cache_manager = cur_animation->getUVWarpCache();
		auto& opacity_cache_manager = cur_animation->getOpacityCache();
        
        meshRenderBoneComposition * render_composition =
        target_creature->GetRenderComposition();
        
        // Extract values from caches
        TMap<FName, meshBone *>& bones_map =
        render_composition->getBonesMap();
        TMap<FName, meshRenderRegion *>& regions_map =
        render_composition->getRegionsMap();
        
		bone_cache_manager.retrieveValuesAtTime(input_run_time,
                                                bones_map);

		AlterBonesByAnchor(bones_map, animation_name_in);
        
        if(bones_override_callback)
        {
            bones_override_callback(bones_map);
        }
        
		displacement_cache_manager.retrieveValuesAtTime(input_run_time,
                                                        regions_map);
		uv_warp_cache_manager.retrieveValuesAtTime(input_run_time,
                                                   regions_map);
		opacity_cache_manager.retrieveValuesAtTime(input_run_time,
													regions_map);
        
        
        // Do posing, decide if we are blending or not
        TArray<meshRenderRegion *>& cur_regions =
        render_composition->getRegions();
        
        render_composition->updateAllTransforms(false);
        for(auto j = 0; j < cur_regions.Num(); j++) {
            meshRenderRegion * cur_region = cur_regions[j];
            
            int32 cur_pt_index = cur_region->getStartPtIndex();
            cur_region->poseFastFinalPts(target_pts + (cur_pt_index * 3));
        }

    }
    
    void
    CreatureManager::ProcessAutoBlending()
    {
        // process blending factor
        blending_factor += auto_blend_delta;
        if(blending_factor > 1)
        {
            blending_factor = 1;
        }
    }

	void 
	CreatureManager::PoseJustBones(const FName& animation_name_in, float input_run_time)
	{
		SCOPE_CYCLE_COUNTER(STAT_CreatureManager_PoseJustBones);

		auto *animEntry = animations.Find(animation_name_in);
		if (animEntry == nullptr)
		{
#ifndef CREATURE_NO_USE_EXCEPTIONS
			throw "CreatureManager::PoseCreature() - Invalid animation name!";
#endif
			return;
		}

		auto& cur_animation = *animEntry;

		auto& bone_cache_manager = cur_animation->getBonesCache();
		auto& opacity_cache_manager = cur_animation->getOpacityCache();

		meshRenderBoneComposition * render_composition =
			target_creature->GetRenderComposition();

		// Extract values from caches
		TMap<FName, meshBone *>& bones_map =
			render_composition->getBonesMap();
		TMap<FName, meshRenderRegion *>& regions_map =
			render_composition->getRegionsMap();

		bone_cache_manager.retrieveValuesAtTime(input_run_time,
			bones_map);

		AlterBonesByAnchor(bones_map, animation_name_in);

		if (bones_override_callback)
		{
			bones_override_callback(bones_map);
		}

		opacity_cache_manager.retrieveValuesAtTime(input_run_time,
			regions_map);

		JustRunUVWarps(animation_name_in, input_run_time);
	}

	void CreatureManager::JustRunUVWarps(const FName& animation_name_in, float input_run_time)
	{
		SCOPE_CYCLE_COUNTER(STAT_CreatureManager_JustRunUVWarps);

		auto& cur_animation = animations[animation_name_in];

		meshRenderBoneComposition * render_composition =
			target_creature->GetRenderComposition();
		TMap<FName, meshRenderRegion *>& regions_map =
			render_composition->getRegionsMap();

		auto& uv_warp_cache_manager = cur_animation->getUVWarpCache();
		uv_warp_cache_manager.retrieveValuesAtTime(input_run_time,
			regions_map);
		TArray<meshRenderRegion *>& all_regions = render_composition->getRegions();

		int32 index = 0;
		for (auto& cur_region : all_regions) {
			if (cur_region->getUseUvWarp())
			{
				cur_region->runUvWarp();
			}
		}
	}

	void 
	CreatureManager::RunUVItemSwap()
	{
		SCOPE_CYCLE_COUNTER(STAT_CreatureManager_RunUVItemSwap);
		meshRenderBoneComposition * render_composition =
			target_creature->GetRenderComposition();
		TMap<FName, meshRenderRegion *>& regions_map =
			render_composition->getRegionsMap();

		auto& swap_packets = target_creature->GetUvSwapPackets();
		auto& active_swap_actions = target_creature->GetActiveItemSwaps();

		if ((swap_packets.Num() == 0) || (active_swap_actions.Num() == 0))
		{
			return;
		}

		for(auto& cur_action : active_swap_actions)
		{
			if (regions_map.Contains(cur_action.Key))
			{
				auto swap_tag = cur_action.Value;
				auto& swap_list = swap_packets[cur_action.Key];
				for (auto& cur_item : swap_list)
				{
					if (cur_item.tag == swap_tag)
					{
						// Perfrom UV Item Swap
						auto cur_region = regions_map[cur_action.Key];
						cur_region->setUvWarpLocalOffset(cur_item.local_offset);
						cur_region->setUvWarpGlobalOffset(cur_item.global_offset);
						cur_region->setUvWarpScale(cur_item.scale);
						cur_region->runUvWarp();

						break;
					}
				}
			}
		}
	}

	void CreatureManager::AlterBonesByAnchor(TMap<FName, meshBone*>& bones_map, const FName & animation_name_in)
	{
		SCOPE_CYCLE_COUNTER(STAT_CreatureManager_AlterBonesByAnchor);
		if (target_creature->GetAnchorPointsActive() == false)
		{
			return;
		}

		auto anchor_point = target_creature->GetAnchorPoint(animation_name_in);
		for (auto& cur_data : bones_map)
		{
			auto cur_bone = cur_data.Value;
			auto start_pt = cur_bone->getWorldStartPt();
			auto end_pt = cur_bone->getWorldEndPt();

			start_pt -= glm::vec4(anchor_point.x, anchor_point.y, 0, 0);
			end_pt -= glm::vec4(anchor_point.x, anchor_point.y, 0, 0);

			cur_bone->setWorldStartPt(start_pt);
			cur_bone->setWorldEndPt(end_pt);
		}
	}

    void
    CreatureManager::Update(float delta)
    {
		SCOPE_CYCLE_COUNTER(STAT_CreatureManager_Update);
        if(!is_playing)
        {
            return;
        }
        
        increRunTime(delta * time_scale);
        
        if(do_auto_blending)
        {
            ProcessAutoBlending();
			// process run times for blends
			increAutoBlendRuntimes(delta * time_scale);
        }
        
        if(do_blending && checkAnimationBlendValid())
        {
            for(int32 i = 0; i < 2; i++) {
				auto& cur_animation_name = active_blend_animation_names[i];
				auto& cur_animation = animations[cur_animation_name];
				auto& cur_animation_run_time = active_blend_run_times[cur_animation_name];

                if(cur_animation->hasCachePts() && do_point_caching)
                {
					UpdateRegionSwitches(cur_animation_name);
					cur_animation->poseFromCachePts(cur_animation_run_time, blend_render_pts[i], target_creature->GetTotalNumPoints());
					PoseJustBones(cur_animation_name, cur_animation_run_time);
                }
                else {
					UpdateRegionSwitches(active_blend_animation_names[i]);
					PoseCreature(active_blend_animation_names[i], blend_render_pts[i], cur_animation_run_time);
                }
            }
            
            for(int32 j = 0; j < target_creature->GetTotalNumPoints() * 3; j++)
            {
                glm::float32 * set_data = target_creature->GetRenderPts() + j;
                glm::float32 * read_data_1 = blend_render_pts[0] + j;
                glm::float32 * read_data_2 = blend_render_pts[1] + j;
                
                *set_data = ((1.0f - blending_factor) * (*read_data_1)) +
                (blending_factor * (*read_data_2));
            }
        }
        else {
            auto& cur_animation = animations[active_animation_name];
            if(cur_animation->hasCachePts() && do_point_caching)
            {
				cur_animation->poseFromCachePts(getRunTime(), target_creature->GetRenderPts(), target_creature->GetTotalNumPoints());
				PoseJustBones(active_animation_name, getRunTime());
            }
            else {
				PoseCreature(active_animation_name, target_creature->GetRenderPts(), getRunTime());
            }
        }

		RunUVItemSwap();
        
        if(mirror_y)
        {
            glm::float32 * set_data = target_creature->GetRenderPts();
            for(int32 j = 0; j < target_creature->GetTotalNumPoints(); j++)
            {
                set_data[0] = -set_data[0];
                set_data += 3;
            }
        }
    }
    
    void
    CreatureManager::SetMirrorY(bool flag_in)
    {
        mirror_y = flag_in;
    }
    
    FName
    CreatureManager::IsContactBone(const glm::vec2& pt_in,
                                   const glm::mat4& creature_xform,
                                   float radius) const
    {
        FName ret_name;
        meshBone * cur_bone = target_creature->GetRenderComposition()->getRootBone();
        
        glm::vec4 local_pt = glm::inverse(creature_xform) * glm::vec4(pt_in, 0, 1);
        glm::vec2 real_pt(local_pt.x, local_pt.y);
        if(mirror_y)
        {
            real_pt.x = -real_pt.x;
        }
        
        return ProcessContactBone(real_pt, radius, cur_bone);
    }
    
    FName
    CreatureManager::ProcessContactBone(const glm::vec2& pt_in,
                                        float radius,
                                        meshBone * bone_in) const
    {
        FName ret_name;
        glm::vec2 cur_vec = glm::vec2(bone_in->getWorldEndPt() - bone_in->getWorldStartPt());
        float cur_length = glm::length(cur_vec);
        glm::vec2 unit_vec = glm::normalize(cur_vec);
        glm::vec2 norm_vec(unit_vec.y, unit_vec.x);
        
        glm::vec2 rel_vec = pt_in - glm::vec2(bone_in->getWorldStartPt());
        float proj = glm::dot(rel_vec, unit_vec);
        
        if( (proj >= 0) && (proj <= cur_length))
        {
            float norm_proj = glm::dot(rel_vec, norm_vec);
            if(norm_proj <= radius)
            {
                return bone_in->getKey();
            }
        }
        
        auto& cur_children = bone_in->getChildren();
        for(auto& cur_child : cur_children)
        {
            ret_name = ProcessContactBone(pt_in, radius, cur_child);
            if(ret_name != NAME_None) {
                break;
            }
        }
        
        return ret_name;
    }

    float CreatureManager::getRunTime() const
    {
        return run_time;
    }
    
    void
    CreatureManager::setRunTime(float time_in)
    {
        run_time = time_in;
    }

	float 
	CreatureManager::getActualRunTime() const
	{
		if (do_auto_blending)
		{
			if (active_blend_run_times.Contains(active_animation_name)) {
				return active_blend_run_times[active_animation_name];
			}
		}

		return run_time;
	}

 
	bool CreatureManager::checkAnimationBlendValid() const
	{
		for (int32 i = 0; i < 2; i++) {
			auto& cur_animation_name = active_blend_animation_names[i];
			if ((!animations.Contains(cur_animation_name)) || 
				(!active_blend_run_times.Contains(cur_animation_name)))
			{
				return false;
			}
		}

		return true;
	}

	float
	CreatureManager::correctRunTime(float time_in, const FName& animation_name)
	{
		float ret_time = time_in;
		auto& cur_animation = animations[animation_name];
		float anim_start_time = cur_animation->getStartTime();
		float anim_end_time = cur_animation->getEndTime();

		if (use_custom_time_range)
		{
			anim_start_time = (float)custom_start_time;
			anim_end_time = (float)custom_end_time;
		}


		if (ret_time > anim_end_time)
		{
			if (should_loop)
			{
				ret_time = anim_start_time;
			}
			else {
				ret_time = anim_end_time;
			}
		}
		else if (ret_time < anim_start_time)
		{
			if (should_loop)
			{
				ret_time = anim_end_time;
			}
			else {
				ret_time = anim_start_time;
			}
		}

		return ret_time;
	}

    void
    CreatureManager::increRunTime(float delta_in)
    {
		SCOPE_CYCLE_COUNTER(STAT_CreatureManager_IncreRunTime);
        if(animations.Contains(active_animation_name) == false)
        {
            return;
        }
        
        run_time += delta_in;
		run_time = correctRunTime(run_time, active_animation_name);
    }
    
	void 
	CreatureManager::increAutoBlendRuntimes(float delta_in)
	{
		FName set_animation_name;
		for (auto& cur_animation_name : auto_blend_names)
		{
			if ((animations.Contains(cur_animation_name)) 
				&& (set_animation_name != cur_animation_name))
			{
				float& cur_run_time = active_blend_run_times[cur_animation_name];
				cur_run_time += delta_in;
				cur_run_time = correctRunTime(cur_run_time, cur_animation_name);

				set_animation_name = cur_animation_name;
			}
		}
	}

    void
    CreatureManager::SetShouldLoop(bool flag_in)
    {
        should_loop = flag_in;
    }

    void
    CreatureManager::SetUseCustomTimeRange(bool flag_in)
    {
        use_custom_time_range = flag_in;
    }
    
    void
    CreatureManager::SetCustomTimeRange(int32 start_time_in, int32 end_time_in)
    {
        custom_start_time = start_time_in;
        custom_end_time = end_time_in;
    }
    
    void
    CreatureManager::SetBonesOverrideCallback(std::function<void (TMap<FName, meshBone *>&) >& callback_in)
    {
        bones_override_callback = callback_in;
    }

}
