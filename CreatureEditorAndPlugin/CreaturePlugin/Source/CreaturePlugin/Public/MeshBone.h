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

#ifndef __EngineApp__MeshBone__
#define __EngineApp__MeshBone__

#include <iostream>
//#include "glm.hpp"
#include <glm/gtx/transform.hpp>
#include  <glm/gtc/quaternion.hpp>
#include  <glm/gtx/quaternion.hpp>
#include  <glm/gtx/dual_quaternion.hpp>
#include  <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>
//#include "ext.hpp"

class dualQuat {
public:
    dualQuat();
    
    dualQuat(const glm::quat& q0, const glm::vec3& t);
    
    void add(const dualQuat& quat_in, float real_factor, float imaginary_factor);
    
    void convertToMat(glm::mat4& m);
    
    void normalize();
    
    glm::vec3 transform(const glm::vec3& p) const;
    
    glm::quat real, imaginary;
};

class meshBone {
public:
    meshBone(const FName& key_in,
             const glm::vec4& start_pt_in,
             const glm::vec4& end_pt_in,
             const glm::mat4& parent_transform);
    
    virtual ~meshBone();
    
    void setRestParentMat(const glm::mat4& transform_in,
                          glm::mat4 * inverse_in=NULL);
    
    void setParentWorldMat(const glm::mat4& transform_in);
    
    void setParentWorldInvMat(const glm::mat4& transform_in);

    glm::vec4& getLocalRestStartPt();

    glm::vec4& getLocalRestEndPt();

    const glm::vec4& getLocalRestStartPt() const;
    
    const glm::vec4& getLocalRestEndPt() const;
    
    void setLocalRestStartPt(const glm::vec4& world_pt_in);

    void setLocalRestEndPt(const glm::vec4& world_pt_in);
    
    void calcRestData();
        
    void setWorldStartPt(const glm::vec4& world_pt_in);
    
    void setWorldEndPt(const glm::vec4& world_pt_in);
    
    void fixDQs(const dualQuat& ref_dq);
    
    void initWorldPts();
    
    glm::vec4 getWorldRestStartPt() const;
    
    glm::vec4 getWorldRestEndPt() const;
    
    float getWorldRestAngle() const;
    
    glm::vec4 getWorldRestPos() const;
    
    glm::vec4 getWorldStartPt() const;
    
    glm::vec4 getWorldEndPt() const;
    
    const glm::mat4& getRestParentMat() const;

    const glm::mat4& getRestWorldMat() const;
    
    const glm::mat4& getWorldDeltaMat() const;

    const glm::mat4& getParentWorldMat() const;

    const glm::mat4& getParentWorldInvMat() const;

    const dualQuat& getWorldDq() const;

    void computeRestParentTransforms();
    
    void computeParentTransforms();
    
    void computeWorldDeltaTransforms();
    
    void addChild(meshBone * bone_in);
    
    TArray<meshBone *>& getChildren() {
        return children;
    }
    
    bool hasBone(meshBone * bone_in) const;
    
    meshBone * getChildByKey(const FName& search_key);
    
    const FName& getKey() const;
    
    void setKey(const FName& key_in);
    
    TArray<FName> getAllBoneKeys() const;
    
    TArray<meshBone *> getAllChildren();
    
    int32 getBoneDepth(meshBone * bone_in, int32 depth=0) const;
    
    bool isLeaf() const {
        return (children.Num() == 0);
    }
    
    void deleteChildren();
    
    void removeChildBone(meshBone * bone_in);
    
    void setTagId(int32 value_in);
    
    int32 getTagId() const;

	void setParent(meshBone * parent_in);

	meshBone * getParent();
    
protected:
    std::pair<glm::vec4, glm::vec4> computeDirs(const glm::vec4& start_pt, const glm::vec4& end_pt);
    
    void computeRestLength();
    
    glm::mat4 rest_parent_mat, rest_parent_inv_mat;
    glm::mat4 rest_world_mat, rest_world_inv_mat;
    glm::mat4 bind_world_mat, bind_world_inv_mat,
                    parent_world_mat, parent_world_inv_mat;
    glm::vec4 local_rest_start_pt, local_rest_end_pt;
    glm::vec4 local_rest_dir, local_rest_normal_dir, local_binormal_dir;
    glm::vec4 world_rest_start_pt, world_rest_end_pt;
    glm::vec4 world_rest_pos;
    float world_rest_angle;
    float rest_length;
    FName key;
    int32 tag_id;

    glm::vec4 world_start_pt, world_end_pt;
    glm::mat4 world_delta_mat;
    dualQuat world_dq;

    TArray<meshBone *> children;
	meshBone * parent;
};

class meshRenderRegion {
public:
    meshRenderRegion(glm::uint32 * indices_in,
                     glm::float32 * rest_pts_in,
                     glm::float32 * uvs_in,
                     int32 start_pt_index_in,
                     int32 end_pt_index_in,
                     int32 start_index_in,
                     int32 end_index_in);
    
    virtual ~meshRenderRegion();

    glm::uint32 * getIndices() const;
    
    glm::float32 * getRestPts() const;
    
    glm::float32 * getUVs() const;
    
    TMap<FName, TArray<float> >& getWeights();
    
    void renameWeightValuesByKey(const FName& old_key,
                                 const FName& new_key);
    
    int32 getNumPts() const;
    
    int32 getStartPtIndex() const;
    
    int32 getEndPtIndex() const;
    
    int32 getNumIndices() const;
    
    int32 getStartIndex() const;
    
    int32 getEndIndex() const;
    
    void poseFinalPts(glm::float32 * output_pts,
                      TMap<FName, meshBone *>& bones_map);
    
    void poseFastFinalPts(glm::float32 * output_pts,
						  bool try_local_displacements=true,
						  bool try_post_displacements=true,
						  bool try_uv_swap=true);
    
    void setMainBoneKey(const FName& key_in);

    void determineMainBone(meshBone * root_bone_in);
    
    void setUseDq(bool flag_in);
    
    bool getUseDq() const;
    
    void setName(const FName& name_in);
    
    const FName& getName() const;
    
    void setUseLocalDisplacements(bool flag_in);
    
    bool getUseLocalDisplacements() const;
    
    TArray<glm::vec2>& getLocalDisplacements();
    
    void setUsePostDisplacements(bool flag_in);
    
    bool getUsePostDisplacements() const;
    
    TArray<glm::vec2>& getPostDisplacements();
    
    glm::vec2 getRestLocalPt(int32 index_in) const;
    
    glm::vec2 getRestGlobalPt(int32 index_in) const;
    
    glm::uint32 getLocalIndex(int32 index_in) const;
    
    void clearLocalDisplacements();
    
    void clearPostDisplacements();
    
    void setUseUvWarp(bool flag_in);
    
    bool getUseUvWarp() const;
    
    void setUvWarpLocalOffset(const glm::vec2& vec_in);

    void setUvWarpGlobalOffset(const glm::vec2& vec_in);

    void setUvWarpScale(const glm::vec2& vec_in);
    
    glm::vec2 getUvWarpLocalOffset() const;

    glm::vec2 getUvWarpGlobalOffset() const;

    glm::vec2 getUvWarpScale() const;
    
    void runUvWarp();
    
    void restoreRefUv();

    int32 getTagId() const;
    
    void setTagId(int32 value_in);
    
    void initFastNormalWeightMap(const TMap<FName, meshBone *>& bones_map);

	void setUVLevel(int32 value_in);

	int32 getUVLevel() const;

	void setOpacity(float value_in);

	float getOpacity() const;

	void setRed(float value_in);

	float getRed() const;

	void setGreen(float value_in);

	float getGreen() const;

	void setBlue(float value_in);

	float getBlue() const;

protected:
    
    void initUvWarp();

    int32 start_pt_index, end_pt_index;
    int32 start_index, end_index;
    glm::uint32 * store_indices;
    glm::float32 * store_rest_pts,
                * store_uvs;
    TArray<glm::vec2> local_displacements;
    bool use_local_displacements;
    TArray<glm::vec2> post_displacements;
    bool use_post_displacements;
    bool use_uv_warp;
    glm::vec2 uv_warp_local_offset, uv_warp_global_offset, uv_warp_scale;
    TArray<glm::vec2> uv_warp_ref_uvs;
	int32 uv_level;
	float opacity;
	float red, green, blue;
    TMap<FName, TArray<float> > normal_weight_map;
//    TMap<int32, TArray<float> > fast_normal_weight_map;
    TArray<TArray<float> > fast_normal_weight_map;
    TArray<TArray<float> > reverse_fast_normal_weight_map;
    TArray<meshBone *> fast_bones_map;
    TArray<TArray<int32> > relevant_bones_indices;
    TArray<dualQuat> fill_dq_array;
    FName main_bone_key;
    meshBone * main_bone;
    bool use_dq;
	FName name;
    int32 tag_id;
};

class meshRenderBoneComposition {
public:
    meshRenderBoneComposition();
    
    virtual ~meshRenderBoneComposition();
    
    void addRegion(meshRenderRegion * region_in);
    
    void setRootBone(meshBone * root_bone_in);
    
    meshBone * getRootBone();
    
    void initBoneMap();
    
    void initRegionsMap();
    
    static TMap<FName, meshBone *> genBoneMap(meshBone * input_bone);
    
    TMap<FName, meshBone *>& getBonesMap();
    
    TMap<FName, meshRenderRegion *>& getRegionsMap();
    
    TArray<meshRenderRegion *>& getRegions();
    
    meshRenderRegion * getRegionWithId(int32 id_in);
    
    void resetToWorldRestPts();
    
    void updateAllTransforms(bool update_parent_xf);
    
protected:
    
    meshBone * root_bone;
    TMap<FName, meshBone *> bones_map;
    TArray<meshRenderRegion *> regions;
    TMap<FName, meshRenderRegion *> regions_map;
};

class meshBoneCache {
public:
    meshBoneCache(const FName& key_in);
    virtual ~meshBoneCache();
    
    void setWorldStartPt(const glm::vec4& pt_in) {
        world_start_pt = pt_in;
    }

    void setWorldEndPt(const glm::vec4& pt_in) {
        world_end_pt = pt_in;
    }

    const glm::vec4& getWorldStartPt() const {
        return world_start_pt;
    }
    
    const glm::vec4& getWorldEndPt() const {
        return world_end_pt;
    }
    
    const FName& getKey() const {
        return key;
    }
    
    void setKey(const FName& key_in)
    {
        key = key_in;
    }

protected:
    FName key;
    glm::vec4 world_start_pt, world_end_pt;
};

class meshDisplacementCache {
public:
    meshDisplacementCache(const FName& key_in);
    virtual ~meshDisplacementCache();
    
    void setLocalDisplacements(const TArray<glm::vec2>& displacements_in)
    {
        local_displacements = displacements_in;
    }

    void setPostDisplacements(const TArray<glm::vec2>& displacements_in)
    {
        post_displacements = displacements_in;
    }
    
    const FName& getKey() const {
        return key;
    }
    
    void setKey(const FName& key_in)
    {
        key = key_in;
    }
    
    const TArray<glm::vec2>& getLocalDisplacements() const;

    const TArray<glm::vec2>& getPostDisplacements() const;
    
protected:
    FName key;
    TArray<glm::vec2> local_displacements;
    TArray<glm::vec2> post_displacements;
};

class meshUVWarpCache {
public:
    meshUVWarpCache(const FName& key_in);
    virtual ~meshUVWarpCache();
    
    void setUvWarpLocalOffset(const glm::vec2& vec_in);
    
    void setUvWarpGlobalOffset(const glm::vec2& vec_in);
    
    void setUvWarpScale(const glm::vec2& vec_in);
    
    const glm::vec2& getUvWarpLocalOffset() const;
    
    const glm::vec2& getUvWarpGlobalOffset() const;
    
    const glm::vec2& getUvWarpScale() const;
    
    const FName& getKey() const {
        return key;
    }
    
    void setKey(const FName& key_in)
    {
        key = key_in;
    }
    
    void setEnabled(bool flag_in)
    {
        enabled = flag_in;
    }
    
    bool getEnabled() const {
        return enabled;
    }

	void setLevel(int32 value_in) {
		level = value_in;
	}

	int32 getLevel() const {
		return level;
	}

protected:
	FName key;
    glm::vec2 uv_warp_local_offset, uv_warp_global_offset, uv_warp_scale;
    bool enabled;
	int32 level;
};

class meshOpacityCache {
public:
	meshOpacityCache(const FName& key_in)
	{
		key = key_in;
		opacity = 100.0f;
		red = 100.0f;
		green = 100.0f;
		blue = 100.0f;
	}

	virtual ~meshOpacityCache() {}

	void setOpacity(float value_in)
	{
		opacity = value_in;
	}

	float getOpacity() const
	{
		return opacity;
	}

	void setRed(float value_in)
	{
		red = value_in;
	}

	float getRed() const
	{
		return red;
	}

	void setGreen(float value_in)
	{
		green = value_in;
	}

	float getGreen() const
	{
		return green;
	}

	void setBlue(float value_in)
	{
		blue = value_in;
	}

	float getBlue() const
	{
		return blue;
	}

	const FName& getKey() const {
		return key;
	}

	void setKey(const FName& key_in)
	{
		key = key_in;
	}

protected:
	FName key;
	float opacity;
	float red, green, blue;
};

class meshBoneCacheManager {
public:
    meshBoneCacheManager();
    
    meshBoneCacheManager( const meshBoneCacheManager& other )
    : bone_cache_table( other.bone_cache_table),
    bone_cache_data_ready( other.bone_cache_data_ready),
    start_time( other.start_time),
    end_time( other.end_time),
    is_ready( other.is_ready)
    {}
    
    meshBoneCacheManager& operator=( const meshBoneCacheManager& other ) {
        bone_cache_table = other.bone_cache_table;
        bone_cache_data_ready = other.bone_cache_data_ready;
        start_time = other.start_time;
        end_time = other.end_time;
        is_ready = other.is_ready;
        
        return *this;
    }
    
    virtual ~meshBoneCacheManager();
    
    void init(int32 start_time_in, int32 end_time_in);
    
    int32 getStartTime() const;
    
    int32 getEndime() const;

    int32 getIndexByTime(int32 time_in) const;
    
    void setValuesAtTime(int32 time_in,
                         TMap<FName, meshBone *>& bone_map);
    
    void retrieveValuesAtTime(float time_in,
                              TMap<FName, meshBone *>& bone_map);
    
    std::pair<glm::vec4, glm::vec4> retrieveSingleBoneValueAtTime(const FName& key_in,
                                                                  float time_in);
    
    bool allReady();
    
    void makeAllReady();
    
    TArray<TArray<meshBoneCache> >& getCacheTable();

protected:
    TArray<TArray<meshBoneCache> > bone_cache_table;
    TArray<bool> bone_cache_data_ready;
    int32 start_time, end_time;
    bool is_ready;
    
    FCriticalSection data_lock;
};

class meshDisplacementCacheManager {
public:
    meshDisplacementCacheManager();
    
    meshDisplacementCacheManager( const meshDisplacementCacheManager& other )
    : displacement_cache_table( other.displacement_cache_table),
    displacement_cache_data_ready( other.displacement_cache_data_ready),
    start_time( other.start_time),
    end_time( other.end_time),
    is_ready( other.is_ready)
    {}
    
    meshDisplacementCacheManager& operator=( const meshDisplacementCacheManager& other ) {
        displacement_cache_table = other.displacement_cache_table;
        displacement_cache_data_ready = other.displacement_cache_data_ready;
        start_time = other.start_time;
        end_time = other.end_time;
        is_ready = other.is_ready;
        
        return *this;
    }
    
    virtual ~meshDisplacementCacheManager();
    
    void init(int32 start_time_in, int32 end_time_in);
    
    int32 getStartTime() const;
    
    int32 getEndime() const;
    
    int32 getIndexByTime(int32 time_in) const;
    
    void setValuesAtTime(int32 time_in,
                         TMap<FName,meshRenderRegion *>& regions_map);
    
    void retrieveValuesAtTime(float time_in,
                              TMap<FName, meshRenderRegion *>& regions_map);
    
    void retrieveSingleDisplacementValueAtTime(const FName& key_in,
                                               float time_in,
                                               meshRenderRegion * region);
    
    void retrieveSingleDisplacementValueNoRegionAtTime(const FName& key_in,
                                                       float time_in,
                                                       meshRenderRegion * region,
                                                       TArray<glm::vec2>& out_displacements);

    void retrieveSingleDisplacementValueDirectAtTime(const FName& key_in,
                                                     float time_in,
                                                     TArray<glm::vec2>& out_local_displacements,
                                                     TArray<glm::vec2>& out_post_displacements);

    bool allReady();
    
    void makeAllReady();

    TArray<TArray<meshDisplacementCache> >& getCacheTable();
    
protected:
    TArray<TArray<meshDisplacementCache> > displacement_cache_table;
    TArray<bool> displacement_cache_data_ready;
    int32 start_time, end_time;
    bool is_ready;
    
	FCriticalSection data_lock;
};

class meshUVWarpCacheManager {
public:
    meshUVWarpCacheManager();
    
    meshUVWarpCacheManager( const meshUVWarpCacheManager& other )
    : uv_cache_table( other.uv_cache_table),
    uv_cache_data_ready( other.uv_cache_data_ready),
    start_time( other.start_time),
    end_time( other.end_time),
    is_ready( other.is_ready)
    {}
    
    meshUVWarpCacheManager& operator=( const meshUVWarpCacheManager& other ) {
        uv_cache_table = other.uv_cache_table;
        uv_cache_data_ready = other.uv_cache_data_ready;
        start_time = other.start_time;
        end_time = other.end_time;
        is_ready = other.is_ready;
        
        return *this;
    }
    
    virtual ~meshUVWarpCacheManager();
    
    void init(int32 start_time_in, int32 end_time_in);
    
    int32 getStartTime() const;
    
    int32 getEndime() const;
    
    int32 getIndexByTime(int32 time_in) const;
    
    void setValuesAtTime(int32 time_in,
                         TMap<FName, meshRenderRegion *>& regions_map);
    
    void retrieveValuesAtTime(float time_in,
                              TMap<FName, meshRenderRegion *>& regions_map);
    
    void retrieveSingleValueAtTime(float time_in,
                                   meshRenderRegion * region,
                                   glm::vec2& local_offset,
                                   glm::vec2& global_offset,
                                   glm::vec2& scale);
    
    bool allReady();
    
    void makeAllReady();

    TArray<TArray<meshUVWarpCache> >& getCacheTable();

protected:
    TArray<TArray<meshUVWarpCache> > uv_cache_table;
    TArray<bool> uv_cache_data_ready;
    int32 start_time, end_time;
    bool is_ready;
    
	FCriticalSection data_lock;
};

class meshOpacityCacheManager {
public:
	meshOpacityCacheManager();

	meshOpacityCacheManager(const meshOpacityCacheManager& other)
		: opacity_cache_table(other.opacity_cache_table),
		opacity_cache_data_ready(other.opacity_cache_data_ready),
		start_time(other.start_time),
		end_time(other.end_time),
		is_ready(other.is_ready)
	{}

	meshOpacityCacheManager& operator=(const meshOpacityCacheManager& other) {
		opacity_cache_table = other.opacity_cache_table;
		opacity_cache_data_ready = other.opacity_cache_data_ready;
		start_time = other.start_time;
		end_time = other.end_time;
		is_ready = other.is_ready;

		return *this;
	}

	virtual ~meshOpacityCacheManager();

	void init(int32 start_time_in, int32 end_time_in);

	int32 getStartTime() const;

	int32 getEndime() const;

	int32 getIndexByTime(int32 time_in) const;

	void setValuesAtTime(int32 time_in,
		TMap<FName, meshRenderRegion *>& regions_map);

	void retrieveValuesAtTime(float time_in,
		TMap<FName, meshRenderRegion *>& regions_map);

	void retrieveSingleValueAtTime(float time_in,
		meshRenderRegion * region,
		float& out_opacity);

	bool allReady();

	void makeAllReady();

	TArray<TArray<meshOpacityCache> >& getCacheTable();

protected:
	TArray<TArray<meshOpacityCache> > opacity_cache_table;
	TArray<bool> opacity_cache_data_ready;
	int32 start_time, end_time;
	bool is_ready;

	FCriticalSection data_lock;
};



#endif /* defined(__EngineApp__MeshBone__) */
