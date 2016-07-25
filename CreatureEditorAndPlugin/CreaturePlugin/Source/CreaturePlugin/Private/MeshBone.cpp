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
#include "MeshBone.h"
#include <math.h>

dualQuat::dualQuat() {
    real.w = 0;
    real.x = 0;
    real.y = 0;
    real.z = 0;
    
    imaginary = real;
}

dualQuat::dualQuat(const glm::quat& q0, const glm::vec3& t) {
    real = q0;
    imaginary.w = -0.5f * ( t.x * q0.x + t.y * q0.y + t.z * q0.z);
    imaginary.x =  0.5f * ( t.x * q0.w + t.y * q0.z - t.z * q0.y);
    imaginary.y =  0.5f * (-t.x * q0.z + t.y * q0.w + t.z * q0.x);
    imaginary.z =  0.5f * ( t.x * q0.y - t.y * q0.x + t.z * q0.w);
}

void dualQuat::add(const dualQuat& quat_in, float real_factor, float imaginary_factor)
{
    real += (quat_in.real * real_factor);
    imaginary += (quat_in.imaginary * imaginary_factor);
}

void dualQuat::convertToMat(glm::mat4& m) {
    float cur_length = glm::dot(real, real);
    float w = real.w , x = real.x, y = real.y, z = real.z;
    float t0 = imaginary.w, t1 = imaginary.x, t2 = imaginary.y, t3 = imaginary.z;
    m[0][0] = w*w + x*x - y*y - z*z;
    m[1][0] = 2 * x * y - 2 * w * z;
    m[2][0] = 2 * x * z + 2 * w * y;
    m[0][1] = 2 * x * y + 2 * w * z;
    m[1][1] = w * w + y * y - x * x - z * z;
    m[2][1] = 2 * y * z - 2 * w * x;
    m[0][2] = 2 * x * z - 2 * w * y;
    m[1][2] = 2 * y * z + 2 * w * x;
    m[2][2] = w * w + z * z - x * x - y * y;
    
    m[3][0] = -2 * t0 * x + 2 * w * t1 - 2 * t2 * z + 2 * y * t3;
    m[3][1] = -2 * t0 * y + 2 * t1 * z - 2 * x * t3 + 2 * w * t2;
    m[3][2] = -2 * t0 * z + 2 * x * t2 + 2 * w * t3 - 2 * t1 * y;
    
    m[0][3] = 0;
    m[1][3] = 0;
    m[2][3] = 0;
    m[3][3] = cur_length;
    m /= cur_length;
}

void dualQuat::normalize() {
    float norm = sqrtf(real.w * real.w + real.x * real.x + real.y * real.y + real.z * real.z);
    
    real = real / norm;
    imaginary = imaginary / norm;
}

glm::vec3 dualQuat::transform(const glm::vec3& p ) const
{
    // Translation from the normalized dual quaternion equals :
    // 2.f * qblend_e * conjugate(qblend_0)
    glm::vec3 v0(real.x, real.y, real.z);
    glm::vec3 ve(imaginary.x, imaginary.y, imaginary.z);
    glm::vec3 trans = (ve*real.w - v0*imaginary.w + glm::cross(v0, ve)) * 2.f;
    
    // Rotate
    return (real * p) + trans;
}

template <typename T>
static T clipNumber(const T& n, const T& lower, const T& upper) {
	return FMath::Clamp(n, lower, upper);
}

static glm::vec4 rotateVec4_90(const glm::vec4& vec_in)
{
    return glm::vec4(-vec_in.y, vec_in.x, vec_in.z, vec_in.w);
}

static glm::mat4 calcRotateMat(const glm::vec4& vec_in)
{
    glm::vec4 dir = vec_in;
    dir = glm::normalize(dir);
    
    glm::vec4 pep_dir = rotateVec4_90(dir);
    
    glm::vec4 cur_tangent(dir.x, dir.y, 0, 0);
    glm::vec4 cur_normal(pep_dir.x, pep_dir.y, 0, 0);
    glm::vec4 cur_binormal(0, 0, 1, 0);
    
    glm::mat4 cur_rotate(cur_tangent, cur_normal, cur_binormal, glm::vec4(0,0,0,1));

    return cur_rotate;
}

static float angleVec4(const glm::vec4& vec_in)
{
    float theta = atan2f(vec_in.y, vec_in.x);
    if(theta < 0) {
        theta += 2.0f * (float)PI;
    }

    return theta;
}

// meshBone
meshBone::meshBone(const FString& key_in,
                   const glm::vec4& start_pt_in,
                   const glm::vec4& end_pt_in,
                   const glm::mat4& parent_transform)
{
    key = key_in;
    world_rest_angle = 0;
    setRestParentMat(parent_transform);
    setLocalRestStartPt(start_pt_in);
    setLocalRestEndPt(end_pt_in);
    setParentWorldInvMat(glm::mat4(1.0));
    setParentWorldMat(glm::mat4(1.0));
    local_binormal_dir = glm::vec4(0,0,1,1);
    tag_id = 0;
	parent = NULL;
}

meshBone::~meshBone()
{
    
}

void meshBone::setTagId(int32 value_in)
{
    tag_id = value_in;
}

int32 meshBone::getTagId() const
{
    return tag_id;
}

void meshBone::deleteChildren()
{
    for(auto i = 0; i < children.Num(); i++) {
        meshBone * cur_bone = children[i];
        cur_bone->deleteChildren();
        delete cur_bone;
    }
    
    children.Empty();
}

void meshBone::removeChildBone(meshBone * bone_in)
{
    TArray<meshBone *> new_children;
    bool remove_found = false;
    for(auto i = 0; i < children.Num(); i++) {
        if(children[i] != bone_in) {
            new_children.Add(children[i]);
        }
        else {
            remove_found = true;
        }
    }
    
    if(remove_found == true) {
        bone_in->deleteChildren();
        delete bone_in;
        children = new_children;
    }
    else {
        for(auto i = 0; i < children.Num(); i++) {
            children[i]->removeChildBone(bone_in);
        }
    }
}

int32
meshBone::getBoneDepth(meshBone * bone_in, int32 depth) const
{
    if(bone_in == this) {
        return depth;
    }
    
    for(auto i = 0; i < children.Num(); i++) {
        meshBone * cur_bone = children[i];
        int32 ret_val = cur_bone->getBoneDepth(bone_in, depth + 1);
        if(ret_val != -1) {
            return ret_val;
        }
    }
    
    return -1;
}

TArray<FString>
meshBone::getAllBoneKeys() const
{
    TArray<FString> ret_data;
    ret_data.Add(getKey());
    
    for(auto i = 0; i < children.Num(); i++) {
        TArray<FString> append_data = children[i]->getAllBoneKeys();
        ret_data.Append(append_data);
    }
    
    return ret_data;
}

TArray<meshBone *>
meshBone::getAllChildren()
{
    TArray<meshBone *> ret_data;
    ret_data.Add(this);
    for(auto i = 0; i < children.Num(); i++) {
        TArray<meshBone *> append_data = children[i]->getAllChildren();
        ret_data.Append(append_data);
    }
    
    return ret_data;
}

void meshBone::setRestParentMat(const glm::mat4& transform_in,
                                glm::mat4 * inverse_in)
{
    rest_parent_mat = transform_in;
    
    if(inverse_in == NULL) {
        rest_parent_inv_mat = glm::inverse(rest_parent_mat);
    }
    else {
        rest_parent_inv_mat = *inverse_in;
    }
}

glm::vec4& meshBone::getLocalRestStartPt()
{
    return local_rest_start_pt;
}

glm::vec4& meshBone::getLocalRestEndPt()
{
    return local_rest_end_pt;
}

const glm::vec4& meshBone::getLocalRestStartPt() const
{
    return local_rest_start_pt;
}

const glm::vec4& meshBone::getLocalRestEndPt() const
{
    return local_rest_end_pt;
}

glm::vec4 meshBone::getWorldRestStartPt() const
{
    return rest_parent_mat * local_rest_start_pt;
}

glm::vec4 meshBone::getWorldRestEndPt() const
{
    return rest_parent_mat * local_rest_end_pt;
}

glm::vec4 meshBone::getWorldStartPt() const
{
    return world_start_pt;
}

glm::vec4 meshBone::getWorldEndPt() const
{
    return world_end_pt;
}

const FString& meshBone::getKey() const
{
    return key;
}

void meshBone::setKey(const FString& key_in)
{
    key = key_in;
}

void meshBone::setLocalRestStartPt(const glm::vec4& world_pt_in)
{
    local_rest_start_pt = rest_parent_inv_mat * world_pt_in;
    calcRestData();
}

void meshBone::setLocalRestEndPt(const glm::vec4& world_pt_in)
{
    local_rest_end_pt = rest_parent_inv_mat * world_pt_in;
    calcRestData();
}

void meshBone::calcRestData()
{
    std::pair<glm::vec4, glm::vec4> calc = computeDirs(local_rest_start_pt, local_rest_end_pt);
    local_rest_dir = calc.first;
    local_rest_normal_dir = calc.second;
    
    computeRestLength();
}

void meshBone::initWorldPts()
{
    setWorldStartPt(getWorldRestStartPt());
    setWorldEndPt(getWorldRestEndPt());
    
    for(auto i = 0; i < children.Num(); i++) {
        children[i]->initWorldPts();
    }
}

void meshBone::computeRestLength()
{
    glm::vec4 tmp_dir = local_rest_end_pt - local_rest_start_pt;
    rest_length = glm::length(tmp_dir);
}

void meshBone::setWorldStartPt(const glm::vec4& world_pt_in)
{
    world_start_pt = world_pt_in;
}

void meshBone::setWorldEndPt(const glm::vec4& world_pt_in)
{
    world_end_pt = world_pt_in;
}

void meshBone::fixDQs(const dualQuat& ref_dq)
{
    if( glm::dot(world_dq.real, ref_dq.real) < 0) {
        world_dq.real = -world_dq.real;
        world_dq.imaginary = -world_dq.imaginary;
    }
    
    for(auto i = 0; i < children.Num(); i++) {
        meshBone * cur_child = children[i];
        cur_child->fixDQs(world_dq);
    }
}

const glm::mat4&
meshBone::getWorldDeltaMat() const
{
    return world_delta_mat;
}

const glm::mat4&
meshBone::getParentWorldInvMat() const
{
    return parent_world_inv_mat;
}

const glm::mat4&
meshBone::getParentWorldMat() const
{
    return parent_world_mat;
}

const dualQuat&
meshBone::getWorldDq() const
{
    return world_dq;
}

void meshBone::computeWorldDeltaTransforms()
{
    std::pair<glm::vec4, glm::vec4> calc = computeDirs(world_start_pt, world_end_pt);
    glm::vec4 cur_tangent(calc.first.x, calc.first.y, 0, 0);
    glm::vec4 cur_normal(calc.second.x, calc.second.y, 0, 0);
    glm::vec4 cur_binormal(local_binormal_dir.x, local_binormal_dir.y, local_binormal_dir.z, 0);
    
//    std::cout<<"World Start Pt: "<<glm::to_string(world_start_pt)<<std::endl;
    
    glm::mat4 cur_rotate(cur_tangent, cur_normal, cur_binormal, glm::vec4(0,0,0,1));
    glm::mat4 cur_translate =
        glm::translate(glm::mat4(1.0),
                   glm::vec3(world_start_pt.x, world_start_pt.y, 0));

    world_delta_mat = (cur_translate * cur_rotate)
                        * bind_world_inv_mat;
    
    
//    glm::quat cur_quat = glm::normalize(glm::toQuat(world_delta_mat));
    glm::quat cur_quat = glm::toQuat(world_delta_mat);
    if(cur_quat.z < 0) {
        //cur_quat = -cur_quat;
    }
    

    world_dq = dualQuat(cur_quat, glm::vec3(world_delta_mat[3]));
//    std::cout<<"angle: "<<acos(world_dq.real.w) * 2.0 * (180.0 / M_PI)<<std::endl;
//    std::cout<<"vector: "<<glm::to_string(glm::vec3(cur_quat.x, cur_quat.y, cur_quat.z))<<std::endl;

/*
    std::cout<<"cur_translate: "<<glm::to_string(cur_translate)<<std::endl;
    std::cout<<"cur_rotate: "<<glm::to_string(cur_rotate)<<std::endl;
    std::cout<<"Delta Mat: "<<glm::to_string(world_delta_mat)<<std::endl;
*/
    for(auto i = 0; i < children.Num(); i++) {
        meshBone * cur_bone = children[i];
        cur_bone->computeWorldDeltaTransforms();
    }
}

const glm::mat4&
meshBone::getRestParentMat() const
{
    return rest_parent_mat;
}

const glm::mat4&
meshBone::getRestWorldMat() const
{
    return rest_world_mat;
}

float
meshBone::getWorldRestAngle() const
{
    return world_rest_angle;
}

glm::vec4
meshBone::getWorldRestPos() const
{
    return world_rest_pos;
}

void meshBone::setParentWorldMat(const glm::mat4& transform_in)
{
    parent_world_mat = transform_in;
}

void meshBone::setParentWorldInvMat(const glm::mat4& transform_in)
{
    parent_world_inv_mat = transform_in;
}

void meshBone::setParent(meshBone * parent_in)
{
	parent = parent_in;
}

meshBone * meshBone::getParent()
{
	return parent;
}

void meshBone::computeParentTransforms()
{
    glm::mat4 translate_parent =
    glm::translate(glm::mat4(1.0),
                   glm::vec3(getWorldEndPt().x, getWorldEndPt().y, 0));
    glm::mat4 rotate_parent = calcRotateMat(getWorldEndPt() - getWorldStartPt());
    
    glm::mat4 final_transform = translate_parent * rotate_parent;
    glm::mat4 final_inv_transform = glm::inverse(final_transform);
    
    for(auto i = 0; i < children.Num(); i++) {
        meshBone * cur_bone = children[i];
        cur_bone->setParentWorldMat(final_transform);
        cur_bone->setParentWorldInvMat(final_inv_transform);
        cur_bone->computeParentTransforms();
    }
}

void meshBone::computeRestParentTransforms()
{
    glm::vec4 cur_tangent(local_rest_dir.x, local_rest_dir.y, 0, 0);
    glm::vec4 cur_binormal(local_binormal_dir.x, local_binormal_dir.y, local_binormal_dir.z, 0);
    glm::vec4 cur_normal(local_rest_normal_dir.x, local_rest_normal_dir.y, 0, 0);
    
    glm::mat4 cur_translate =
        glm::translate(glm::mat4(1.0),
                       glm::vec3(local_rest_end_pt.x, local_rest_end_pt.y, 0));
    glm::mat4 cur_rotate(cur_tangent, cur_normal, cur_binormal, glm::vec4(0,0,0,1));
    glm::mat4 cur_final = cur_translate * cur_rotate;
    
    rest_world_mat = rest_parent_mat * cur_final;
    rest_world_inv_mat = glm::inverse(rest_world_mat);
    
    glm::vec4 world_rest_dir = getWorldRestEndPt() - getWorldRestStartPt();
    world_rest_dir = glm::normalize(world_rest_dir);
    world_rest_angle = angleVec4(world_rest_dir);
    world_rest_pos = getWorldRestStartPt();
    
    
    glm::mat4 bind_translate =
    glm::translate(glm::mat4(1.0),
                   glm::vec3(getWorldRestStartPt().x, getWorldRestStartPt().y, 0));
    glm::mat4 bind_rotate = calcRotateMat(getWorldRestEndPt() - getWorldRestStartPt());
    glm::mat4 cur_bind_final = bind_translate * bind_rotate;
    
    bind_world_mat = cur_bind_final;
    bind_world_inv_mat = glm::inverse(bind_world_mat);

    
    for(auto i = 0; i < children.Num(); i++) {
        meshBone * cur_bone = children[i];
        cur_bone->setRestParentMat(rest_world_mat, &rest_world_inv_mat);
        cur_bone->computeRestParentTransforms();
    }
}

std::pair<glm::vec4, glm::vec4>
meshBone::computeDirs(const glm::vec4& start_pt, const glm::vec4& end_pt)
{
    glm::vec4 tangent = end_pt - start_pt;
    tangent = glm::normalize(tangent);
    
    glm::vec4 normal = rotateVec4_90(tangent);
    
    return std::pair<glm::vec4, glm::vec4>(tangent, normal);
}

void meshBone::addChild(meshBone * bone_in)
{
    bone_in->setRestParentMat(rest_world_mat, &rest_world_inv_mat);
	bone_in->setParent(this);
    children.Add(bone_in);
}

bool
meshBone::hasBone(meshBone * bone_in) const
{
    for(auto i = 0; i < children.Num(); i++) {
        meshBone* cur_bone = children[i];
        if(cur_bone == bone_in) {
            return true;
        }
    }
    
    return false;
}

meshBone * meshBone::getChildByKey(const FString& search_key)
{
    if(key == search_key) {
        return this;
    }
    
    meshBone * ret_data = NULL;
    for(auto i = 0; i < children.Num(); i++) {
        meshBone* cur_bone = children[i];
        
        meshBone * result = cur_bone->getChildByKey(search_key);
        if(result != NULL) {
            ret_data = result;
            break;
        }
    }
    
    return ret_data;
}

// meshRenderRegion
meshRenderRegion::meshRenderRegion(glm::uint32 * indices_in,
                                   glm::float32 * rest_pts_in,
                                   glm::float32 * uvs_in,
                                   int32 start_pt_index_in,
                                   int32 end_pt_index_in,
                                   int32 start_index_in,
                                   int32 end_index_in)
{
    store_indices = indices_in;
    store_rest_pts = rest_pts_in;
    store_uvs = uvs_in;
    
    use_local_displacements = false;
    use_post_displacements = false;
    use_uv_warp = false;
    uv_warp_local_offset = glm::vec2(0,0);
    uv_warp_global_offset = glm::vec2(0,0);
    uv_warp_scale = glm::vec2(1,1);
    start_pt_index = start_pt_index_in;
    end_pt_index = end_pt_index_in;
    start_index = start_index_in;
    end_index = end_index_in;
    main_bone = NULL;
    use_dq = true;
    tag_id = -1;
	uv_level = 0;
	opacity = 100.0f;
    
    initUvWarp();
}

meshRenderRegion::~meshRenderRegion() {
}

void meshRenderRegion::setUVLevel(int32 value_in)
{
	uv_level = value_in;
}

int32 meshRenderRegion::getUVLevel() const
{
	return uv_level;
}

int32 meshRenderRegion::getTagId() const
{
    return tag_id;
}

void meshRenderRegion::setTagId(int32 value_in)
{
    tag_id = value_in;
}

int32 meshRenderRegion::getStartPtIndex() const
{
    return start_pt_index;
}

int32 meshRenderRegion::getEndPtIndex() const
{
    return end_pt_index;
}

int32 meshRenderRegion::getStartIndex() const
{
    return start_index;
}

int32 meshRenderRegion::getEndIndex() const
{
    return end_index;
}

glm::uint32 * meshRenderRegion::getIndices() const
{
    return store_indices + (start_index);
}

glm::float32 * meshRenderRegion::getRestPts() const
{
    return store_rest_pts + (3 * start_pt_index);
}

glm::float32 * meshRenderRegion::getUVs() const
{
    return store_uvs + (2  * start_pt_index);
}

TMap<FString, TArray<float> >&
meshRenderRegion::getWeights()
{
    return normal_weight_map;
}

void
meshRenderRegion::renameWeightValuesByKey(const FString& old_key,
                                          const FString& new_key)
{
    if(normal_weight_map.Contains(old_key) == false)
    {
        return;
    }
    
    TArray<float> weight_values = normal_weight_map[old_key];
    normal_weight_map.Remove(old_key);
    normal_weight_map.Add(new_key, weight_values);
}

void
meshRenderRegion::initFastNormalWeightMap(const TMap<FString, meshBone *>& bones_map)
{
    fast_normal_weight_map.Empty();
    fast_bones_map.Empty();
    reverse_fast_normal_weight_map.Empty();
    relevant_bones_indices.Empty();
    fill_dq_array.Empty();
    
    for(auto& bone_data : bones_map)
    {
        TArray<float> values = normal_weight_map[bone_data.Key];
        fast_normal_weight_map.Add(values);
        
        fast_bones_map.Add(bone_data.Value);
        
        if(reverse_fast_normal_weight_map.Num() == 0)
        {
            reverse_fast_normal_weight_map.SetNumZeroed(values.Num());
            relevant_bones_indices.SetNumZeroed(values.Num());
        }
    }
    
    fill_dq_array.SetNumZeroed(bones_map.Num());
    
    for(auto i = 0; i < reverse_fast_normal_weight_map.Num(); i++)
    {
        // reverse normal weight map
        TArray<float> new_values;
        new_values.SetNumZeroed(fast_normal_weight_map.Num());

        for(auto j = 0; j < fast_normal_weight_map.Num(); j++)
        {
            new_values[j]  = fast_normal_weight_map[j][i];
        }
        
        reverse_fast_normal_weight_map[i] = new_values;
        
        // relevant bone indices
        TArray<int32> relevant_array;
        const float cutoff_val = 0.05f;
        for(auto j = 0; j < fast_normal_weight_map.Num(); j++)
        {
            float sample_val = fast_normal_weight_map[j][i];
            if(sample_val > cutoff_val)
            {
                relevant_array.Add((int32)j);
            }
        }
        
        relevant_bones_indices[i] = relevant_array;
    }
    
    fast_normal_weight_map.Empty();
}

int32 meshRenderRegion::getNumPts() const
{
    return end_pt_index - start_pt_index + 1;
}

int32 meshRenderRegion::getNumIndices() const
{
    return end_index - start_index + 1;
}

void meshRenderRegion::setMainBoneKey(const FString& key_in)
{
    main_bone_key = key_in;
}

void meshRenderRegion::determineMainBone(meshBone * root_bone_in)
{
    main_bone = root_bone_in->getChildByKey(main_bone_key);
}

void meshRenderRegion::setUseDq(bool flag_in)
{
    use_dq = flag_in;
}

bool
meshRenderRegion::getUseDq() const
{
    return use_dq;
}

void
meshRenderRegion::setName(const FString& name_in)
{
    name = name_in;
}

const FString&
meshRenderRegion::getName() const
{
    return name;
}

void
meshRenderRegion::setUseLocalDisplacements(bool flag_in)
{
    use_local_displacements = flag_in;
    if((local_displacements.Num() != getNumPts())
       && use_local_displacements)
    {
        local_displacements.SetNumZeroed(getNumPts());
    }
}

bool
meshRenderRegion::getUseLocalDisplacements() const
{
    return use_local_displacements;
}

TArray<glm::vec2>&
meshRenderRegion::getLocalDisplacements()
{
    return local_displacements;
}

void
meshRenderRegion::clearLocalDisplacements()
{
    for(auto i = 0; i < local_displacements.Num(); i++) {
        local_displacements[i].x = 0;
        local_displacements[i].y = 0;
    }
}

void
meshRenderRegion::clearPostDisplacements()
{
    for(auto i = 0; i < post_displacements.Num(); i++) {
        post_displacements[i].x = 0;
        post_displacements[i].y = 0;
    }
}

void
meshRenderRegion::setUsePostDisplacements(bool flag_in)
{
    use_post_displacements = flag_in;
    if((post_displacements.Num() != getNumPts())
       && use_post_displacements)
    {
        post_displacements.SetNumZeroed(getNumPts());
    }
}

bool
meshRenderRegion::getUsePostDisplacements() const
{
    return use_post_displacements;
}

TArray<glm::vec2>&
meshRenderRegion::getPostDisplacements()
{
    return post_displacements;
}

void
meshRenderRegion::setUseUvWarp(bool flag_in)
{
    use_uv_warp = flag_in;
    if(use_uv_warp == false) {
        restoreRefUv();
    }
}

bool
meshRenderRegion::getUseUvWarp() const
{
    return use_uv_warp;
}

void
meshRenderRegion::setUvWarpLocalOffset(const glm::vec2& vec_in)
{
    uv_warp_local_offset = vec_in;
}

void
meshRenderRegion::setUvWarpGlobalOffset(const glm::vec2& vec_in)
{
    uv_warp_global_offset = vec_in;
}

void
meshRenderRegion::setUvWarpScale(const glm::vec2& vec_in)
{
    uv_warp_scale = vec_in;
}

glm::vec2
meshRenderRegion::getUvWarpLocalOffset() const
{
    return uv_warp_local_offset;
}

glm::vec2
meshRenderRegion::getUvWarpGlobalOffset() const
{
    return uv_warp_global_offset;
}

glm::vec2
meshRenderRegion::getUvWarpScale() const
{
    return uv_warp_scale;
}

void
meshRenderRegion::initUvWarp()
{
    uv_warp_ref_uvs.SetNumZeroed(getNumPts());
    glm::float32 * cur_uvs = getUVs();
    for(auto i = 0; i < uv_warp_ref_uvs.Num(); i++) {
        uv_warp_ref_uvs[i].x = *cur_uvs;
        uv_warp_ref_uvs[i].y = *(cur_uvs + 1);
        
        cur_uvs += 2;
    }
}

void
meshRenderRegion::runUvWarp()
{
    glm::float32 * cur_uvs = getUVs();
    for(auto i = 0; i < uv_warp_ref_uvs.Num(); i++) {
        glm::vec2 set_uv = uv_warp_ref_uvs[i];
        set_uv -= uv_warp_local_offset;
        set_uv *= uv_warp_scale;
        set_uv += uv_warp_global_offset;
        
        cur_uvs[0] = set_uv.x;
        cur_uvs[1] = set_uv.y;
        
        cur_uvs += 2;
    }
}

void
meshRenderRegion::restoreRefUv()
{
    glm::float32 * cur_uvs = getUVs();
    for(auto i = 0; i < uv_warp_ref_uvs.Num(); i++) {
        glm::vec2 set_uv = uv_warp_ref_uvs[i];
        cur_uvs[0] = set_uv.x;
        cur_uvs[1] = set_uv.y;
        
        cur_uvs += 2;
    }
}

void 
meshRenderRegion::setOpacity(float value_in)
{
	opacity = value_in;
}

float 
meshRenderRegion::getOpacity() const
{
	return opacity;
}

glm::vec2
meshRenderRegion::getRestLocalPt(int32 index_in) const
{
    glm::float32 * read_pt = getRestPts() + (3 * index_in);
    glm::vec2 return_pt(read_pt[0], read_pt[1]);
    return return_pt;
}

glm::vec2
meshRenderRegion::getRestGlobalPt(int32 index_in) const
{
    glm::float32 * read_pt = store_rest_pts + (3 * index_in);
    glm::vec2 return_pt(read_pt[0], read_pt[1]);
    return return_pt;
}

glm::uint32
meshRenderRegion::getLocalIndex(int32 index_in) const
{
    glm::uint32 * read_index = getIndices() + index_in;
    return *read_index;
}

void meshRenderRegion::poseFinalPts(glm::float32 * output_pts,
                                    TMap<FString, meshBone *>& bones_map)
{
    glm::float32 * read_pt = getRestPts();
    glm::float32 * write_pt = output_pts;
    
    // point posing
    for(int32 i = 0; i < getNumPts(); i++) {
        glm::vec4 cur_rest_pt(read_pt[0], read_pt[1], read_pt[2], 1);
        
        if(use_local_displacements) {
            cur_rest_pt.x += local_displacements[i].x;
            cur_rest_pt.y += local_displacements[i].y;
        }
        
        glm::mat4 accum_mat(0);
        dualQuat accum_dq;
        
        int32 n_index = 0;
        for(auto& cur_iter : bones_map)
        {
            const FString& cur_key = cur_iter.Key;
            meshBone * cur_bone = cur_iter.Value;
            float cur_weight_val = 0;
            if(fast_normal_weight_map.Num() > 0) {
                cur_weight_val = fast_normal_weight_map[n_index][i];
            }
            else {
                cur_weight_val = normal_weight_map[cur_key][i];
            }
            
            float cur_im_weight_val = cur_weight_val;
            
            if(use_dq == false) {
                const glm::mat4& world_delta_mat = cur_bone->getWorldDeltaMat();
                accum_mat += world_delta_mat * cur_weight_val;
            }
            else {
                const dualQuat& world_dq = cur_bone->getWorldDq();
                accum_dq.add(world_dq, cur_weight_val, cur_im_weight_val);
            }
            
            ++n_index;
        }

        glm::vec4 final_pt(0);
        if(use_dq == false) {
            glm::mat4 final_mat = accum_mat;
            final_pt = accum_mat * cur_rest_pt;
        }
        else {
            accum_dq.normalize();
            final_pt = glm::vec4(accum_dq.transform(glm::vec3(cur_rest_pt)), 1);
        }
        
        write_pt[0] = final_pt.x;
        write_pt[1] = final_pt.y;
        write_pt[2] = final_pt.z;
        
        if(use_post_displacements) {
            write_pt[0] += post_displacements[i].x;
            write_pt[1] += post_displacements[i].y;
        }
        
        read_pt += 3;
        write_pt += 3;
    }
    
    // uv warping
    if(use_uv_warp) {
        runUvWarp();
    }
}

void meshRenderRegion::poseFastFinalPts(glm::float32 * output_pts,
										bool try_local_displacements,
										bool try_post_displacements,
										bool try_uv_swap)
{
    glm::float32 * read_pt = getRestPts();
    glm::float32 * write_pt = output_pts;
    
    // fill up dqs
    for(auto i = 0; i < fill_dq_array.Num(); i++)
    {
        fill_dq_array[i] = fast_bones_map[i]->getWorldDq();
    }
    
    // pose points
    for(int32 i = 0; i < getNumPts(); i++) {
        glm::vec4 cur_rest_pt(read_pt[0], read_pt[1], read_pt[2], 1);
        
        if(use_local_displacements && try_local_displacements) {
            cur_rest_pt.x += local_displacements[i].x;
            cur_rest_pt.y += local_displacements[i].y;
        }
        
        glm::mat4 accum_mat(0);
        dualQuat accum_dq;
        
        const auto& weight_map_vals = reverse_fast_normal_weight_map[i];
        const auto& bone_indices = relevant_bones_indices[i];
        
        for(auto j : bone_indices)
        {
            float cur_im_weight_val = weight_map_vals[j];
            const dualQuat& world_dq = fill_dq_array[j];
            accum_dq.add(world_dq, cur_im_weight_val, cur_im_weight_val);
        }
        
        glm::vec4 final_pt(0);
        accum_dq.normalize();
        final_pt = glm::vec4(accum_dq.transform(glm::vec3(cur_rest_pt)), 1);
        
        write_pt[0] = final_pt.x;
        write_pt[1] = final_pt.y;
        write_pt[2] = 1;
        
		if (use_post_displacements && try_post_displacements)
		{
            write_pt[0] += post_displacements[i].x;
            write_pt[1] += post_displacements[i].y;
        }
        
        read_pt += 3;
        write_pt += 3;
    }
    
    // uv warping
	if (use_uv_warp && try_uv_swap) {
        runUvWarp();
    }
}

// meshRenderBoneComposition
meshRenderBoneComposition::meshRenderBoneComposition()
{
    
}

meshRenderBoneComposition::~meshRenderBoneComposition()
{
    for(auto i = 0; i < regions.Num(); i++) {
        delete regions[i];
    }
    
}

void meshRenderBoneComposition::addRegion(meshRenderRegion * region_in)
{
    regions.Add(region_in);
}

meshRenderRegion *
meshRenderBoneComposition::getRegionWithId(int32 id_in)
{
    for(auto i = 0; i < regions.Num(); i++) {
        meshRenderRegion * cur_region = regions[i];
        if(cur_region->getTagId() == id_in) {
            return cur_region;
        }
    }
    
    return NULL;
}

void meshRenderBoneComposition::setRootBone(meshBone * root_bone_in)
{
    root_bone = root_bone_in;
}

meshBone *
meshRenderBoneComposition::getRootBone()
{
    return root_bone;
}

void meshRenderBoneComposition::initBoneMap()
{
    bones_map = meshRenderBoneComposition::genBoneMap(root_bone);
}

TMap<FString, meshBone *>
meshRenderBoneComposition::genBoneMap(meshBone * input_bone)
{
    TMap<FString, meshBone *> ret_map;
    TArray<FString> all_keys = input_bone->getAllBoneKeys();
    for(auto i = 0; i < all_keys.Num(); i++) {
        FString cur_key = all_keys[i];
        ret_map.Add(cur_key, input_bone->getChildByKey(cur_key));
    }
    
    return ret_map;
}

void
meshRenderBoneComposition::initRegionsMap()
{
    regions_map.Empty();
    for(auto i = 0; i < regions.Num(); i++) {
        FString cur_key = regions[i]->getName();
        regions_map.Add(cur_key, regions[i]);
    }
}

TMap<FString, meshBone *>&
meshRenderBoneComposition::getBonesMap()
{
    return bones_map;
}

TMap<FString, meshRenderRegion *>&
meshRenderBoneComposition::getRegionsMap()
{
    return regions_map;
}

TArray<meshRenderRegion *>&
meshRenderBoneComposition::getRegions()
{
    return regions;
}

void meshRenderBoneComposition::updateAllTransforms(bool update_parent_xf)
{
    if(update_parent_xf) {
        getRootBone()->computeParentTransforms();
    }
    
    getRootBone()->computeWorldDeltaTransforms();
    getRootBone()->fixDQs(getRootBone()->getWorldDq());
}

void
meshRenderBoneComposition::resetToWorldRestPts()
{
    getRootBone()->initWorldPts();
}


// meshBoneCache
meshBoneCache::meshBoneCache(const FString& key_in)
{
    key = key_in;
}

meshBoneCache::~meshBoneCache()
{
    
}

// meshDisplacementCache
meshDisplacementCache::meshDisplacementCache(const FString& key_in)
{
    key = key_in;
}

meshDisplacementCache::~meshDisplacementCache()
{
    
}

const TArray<glm::vec2>&
meshDisplacementCache::getLocalDisplacements() const
{
    return local_displacements;
}

const TArray<glm::vec2>&
meshDisplacementCache::getPostDisplacements() const
{
    return post_displacements;
}


// meshUVWarpCache
meshUVWarpCache::meshUVWarpCache(const FString& key_in)
{
    uv_warp_global_offset = uv_warp_local_offset = glm::vec2(0,0);
    uv_warp_scale = glm::vec2(-1,-1);
    key = key_in;
    enabled = false;
	level = 0;
}

meshUVWarpCache::~meshUVWarpCache()
{
    
}

void
meshUVWarpCache::setUvWarpLocalOffset(const glm::vec2& vec_in)
{
    uv_warp_local_offset = vec_in;
}

void
meshUVWarpCache::setUvWarpGlobalOffset(const glm::vec2& vec_in)
{
    uv_warp_global_offset = vec_in;
}

void
meshUVWarpCache::setUvWarpScale(const glm::vec2& vec_in)
{
    uv_warp_scale = vec_in;
}

const glm::vec2&
meshUVWarpCache::getUvWarpLocalOffset() const
{
    return uv_warp_local_offset;
}

const glm::vec2&
meshUVWarpCache::getUvWarpGlobalOffset() const
{
    return uv_warp_global_offset;
}

const glm::vec2&
meshUVWarpCache::getUvWarpScale() const
{
    return uv_warp_scale;
}

// meshBoneCacheManager
meshBoneCacheManager::meshBoneCacheManager()
{
    is_ready = false;
}

meshBoneCacheManager::~meshBoneCacheManager()
{
    
}

void
meshBoneCacheManager::init(int32 start_time_in, int32 end_time_in)
{
    start_time = start_time_in;
    end_time = end_time_in;
    
    int32 num_frames = end_time - start_time + 1;
    bone_cache_table.Empty();
    bone_cache_table.SetNumZeroed(num_frames);
    
    bone_cache_data_ready.Empty();
    bone_cache_data_ready.SetNumZeroed(num_frames);
    for(auto i = 0; i < bone_cache_data_ready.Num(); i++) {
        bone_cache_data_ready[i] = false;
    }
    
    is_ready = false;
}

void
meshBoneCacheManager::makeAllReady()
{
    for(auto i = 0; i < bone_cache_data_ready.Num(); i++) {
        bone_cache_data_ready[i] = true;
    }
}

TArray<TArray<meshBoneCache> >&
meshBoneCacheManager::getCacheTable()
{
    return bone_cache_table;
}

int32 meshBoneCacheManager::getStartTime() const
{
    return start_time;
}

int32 meshBoneCacheManager::getEndime() const
{
    return end_time;
}

int32
meshBoneCacheManager::getIndexByTime(int32 time_in) const
{
    int32 retval = time_in - start_time;
    retval = clipNumber(retval, 0, (int32)bone_cache_table.Num() - 1);

    return retval;
}

void
meshBoneCacheManager::setValuesAtTime(int32 time_in,
                                      TMap<FString, meshBone *>& bone_map)
{
    TArray<meshBoneCache> cache_list;
    int32 set_index = getIndexByTime(time_in);
    for(auto& cur_iter : bone_map)
    {
        const FString& cur_key = cur_iter.Key;
        meshBone * cur_bone = cur_iter.Value;
        
        meshBoneCache new_cache(cur_key);
        new_cache.setWorldStartPt(cur_bone->getWorldStartPt());
        new_cache.setWorldEndPt(cur_bone->getWorldEndPt());
        cache_list.Add(new_cache);
    }
    
    bone_cache_table[set_index] = cache_list;
    bone_cache_data_ready[set_index] = true;    
}

bool
meshBoneCacheManager::allReady()
{
    if(is_ready) {
        return true;
    }
    else {
        int32 num_frames = end_time - start_time + 1;
        int32 ready_cnt = 0;
        for(auto i = 0; i < bone_cache_data_ready.Num(); i++) {
            if(bone_cache_data_ready[i]) {
                ready_cnt++;
            }
        }
        
        if(ready_cnt == num_frames) {
            is_ready = true;
        }
    }
    
    return is_ready;
}

void
meshBoneCacheManager::retrieveValuesAtTime(float time_in,
                                           TMap<FString, meshBone *>& bone_map)
{
    int32 base_time = getIndexByTime((int32)floorf(time_in));
    int32 end_time = getIndexByTime((int32)ceilf(time_in));

    float ratio = (time_in - (float)floorf(time_in));

    if(bone_cache_data_ready.Num() == 0) {
        return;
    }
    
    if((bone_cache_data_ready[base_time] == false)
       || (bone_cache_data_ready[end_time] == false))
    {
        return;
    }
    
    TArray<meshBoneCache>& base_cache = bone_cache_table[base_time];
    TArray<meshBoneCache>& end_cache = bone_cache_table[end_time];
    
    for(auto i = 0; i < base_cache.Num(); i++) {
        const meshBoneCache& base_data = base_cache[i];
        const meshBoneCache& end_data = end_cache[i];
        const FString& cur_key = base_data.getKey();
        
        glm::vec4 final_world_start_pt = ((1.0f - ratio) * base_data.getWorldStartPt()) +
                                        (ratio * end_data.getWorldStartPt());
        
        glm::vec4 final_world_end_pt = ((1.0f - ratio) * base_data.getWorldEndPt()) +
                                        (ratio * end_data.getWorldEndPt());
        
        bone_map[cur_key]->setWorldStartPt(final_world_start_pt);
        bone_map[cur_key]->setWorldEndPt(final_world_end_pt);
    }    
}

std::pair<glm::vec4, glm::vec4>
meshBoneCacheManager::retrieveSingleBoneValueAtTime(const FString& key_in,
                                                    float time_in)
{
    int32 base_time = getIndexByTime((int32)floorf(time_in));
    int32 end_time = getIndexByTime((int32)ceilf(time_in));
    float ratio = (time_in - (float)floorf(time_in));
    std::pair<glm::vec4, glm::vec4> ret_data;

    if(bone_cache_data_ready.Num() == 0) {
        return ret_data;
    }
    
    if((bone_cache_data_ready[base_time] == false)
       || (bone_cache_data_ready[end_time] == false))
    {
        return ret_data;
    }

    TArray<meshBoneCache>& base_cache = bone_cache_table[base_time];
    TArray<meshBoneCache>& end_cache = bone_cache_table[end_time];
    
    for(auto i = 0; i < base_cache.Num(); i++) {
        const meshBoneCache& base_data = base_cache[i];
        const meshBoneCache& end_data = end_cache[i];
        const FString& cur_key = base_data.getKey();
        
        if(cur_key == key_in) {
            glm::vec4 final_world_start_pt = ((1.0f - ratio) * base_data.getWorldStartPt()) +
                                                (ratio * end_data.getWorldStartPt());
            
            glm::vec4 final_world_end_pt = ((1.0f - ratio) * base_data.getWorldEndPt()) +
                                                (ratio * end_data.getWorldEndPt());

            ret_data.first = final_world_start_pt;
            ret_data.second = final_world_end_pt;
            
            break;
        }
    }

    return ret_data;
}


// meshDisplacementCacheManager
meshDisplacementCacheManager::meshDisplacementCacheManager()
{
    is_ready = false;
}

meshDisplacementCacheManager::~meshDisplacementCacheManager()
{
    
}

void meshDisplacementCacheManager::init(int32 start_time_in, int32 end_time_in)
{
    start_time = start_time_in;
    end_time = end_time_in;
    
    int32 num_frames = end_time - start_time + 1;
    displacement_cache_table.Empty();
    displacement_cache_table.SetNumZeroed(num_frames);
    
    displacement_cache_data_ready.Empty();
    displacement_cache_data_ready.SetNumZeroed(num_frames);
    for(auto i = 0; i < displacement_cache_data_ready.Num(); i++) {
        displacement_cache_data_ready[i] = false;
    }
    
    is_ready = false;

}

void
meshDisplacementCacheManager::makeAllReady()
{
    for(auto i = 0; i < displacement_cache_data_ready.Num(); i++) {
        displacement_cache_data_ready[i] = true;
    }
}

TArray<TArray<meshDisplacementCache> >&
meshDisplacementCacheManager::getCacheTable()
{
    return displacement_cache_table;
}

int32 meshDisplacementCacheManager::getStartTime() const
{
    return start_time;
}

int32 meshDisplacementCacheManager::getEndime() const
{
    return end_time;
}

int32 meshDisplacementCacheManager::getIndexByTime(int32 time_in) const
{
    int32 retval = time_in - start_time;
    retval = clipNumber(retval, 0, (int32)displacement_cache_table.Num() - 1);

    return retval;
}

void meshDisplacementCacheManager::setValuesAtTime(int32 time_in,
                                                   TMap<FString,meshRenderRegion *>& regions_map)
{
    TArray<meshDisplacementCache> cache_list;
    int32 set_index = getIndexByTime(time_in);
    for(auto& cur_iter : regions_map)
    {
        const FString& cur_key = cur_iter.Key;
        meshRenderRegion * cur_region = cur_iter.Value;
        
        meshDisplacementCache new_cache(cur_key);
        if(cur_region->getUseLocalDisplacements()) {
            new_cache.setLocalDisplacements(cur_region->getLocalDisplacements());
        }
        
        if(cur_region->getUsePostDisplacements()) {
            new_cache.setPostDisplacements(cur_region->getPostDisplacements());
        }
        
        cache_list.Add(new_cache);
    }
    
    displacement_cache_table[set_index] = cache_list;
    displacement_cache_data_ready[set_index] = true;
}

void meshDisplacementCacheManager::retrieveValuesAtTime(float time_in,
                                                        TMap<FString,meshRenderRegion *>& regions_map)
{
    int32 base_time = getIndexByTime((int32)floorf(time_in));
    int32 end_time = getIndexByTime((int32)ceilf(time_in));
    
    float ratio = (time_in - (float)floorf(time_in));
    
    if(displacement_cache_data_ready.Num() == 0) {
        return;
    }
    
    if((displacement_cache_data_ready[base_time] == false)
       || (displacement_cache_data_ready[end_time] == false))
    {
        return;
    }
        
    TArray<meshDisplacementCache>& base_cache = displacement_cache_table[base_time];
    TArray<meshDisplacementCache>& end_cache = displacement_cache_table[end_time];
    
    for(auto i = 0; i < base_cache.Num(); i++) {
        const meshDisplacementCache& base_data = base_cache[i];
        const meshDisplacementCache& end_data = end_cache[i];
        const FString& cur_key = base_data.getKey();
        
        meshRenderRegion * set_region = regions_map[cur_key];

        if(set_region->getUseLocalDisplacements()) {
            TArray<glm::vec2>& displacements =
                set_region->getLocalDisplacements();
            if((base_data.getLocalDisplacements().Num() == displacements.Num())
               && (end_data.getLocalDisplacements().Num() == displacements.Num()))
            {
                for(auto j = 0; j < displacements.Num(); j++) {
                    glm::vec2 interp_val =
                    ((1.0f - ratio) * base_data.getLocalDisplacements()[j]) +
                    (ratio * end_data.getLocalDisplacements()[j]);
                    displacements[j] = interp_val;
                }
            }
            else {
                for(auto j = 0; j < displacements.Num(); j++) {
                    displacements[j] = glm::vec2(0, 0);
                }
            }
        }
        
        if(set_region->getUsePostDisplacements()) {
            TArray<glm::vec2>& displacements =
                set_region->getPostDisplacements();
            if((base_data.getPostDisplacements().Num() == displacements.Num())
               && (end_data.getPostDisplacements().Num() == displacements.Num()))
            {
                
                for(auto j = 0; j < displacements.Num(); j++) {
                    glm::vec2 interp_val =
                    ((1.0f - ratio) * base_data.getPostDisplacements()[j]) +
                    (ratio * end_data.getPostDisplacements()[j]);
                    displacements[j] = interp_val;
                }
            }
            else {
                for(auto j = 0; j < displacements.Num(); j++) {
                    displacements[j] = glm::vec2(0, 0);
                }                
            }
        }
    }
}

void
meshDisplacementCacheManager::retrieveSingleDisplacementValueAtTime(const FString& key_in,
                                                                   float time_in,
                                                                    meshRenderRegion * region)
{
    int32 base_time = getIndexByTime((int32)floorf(time_in));
    int32 end_time = getIndexByTime((int32)ceilf(time_in));
    float ratio = (time_in - (float)floorf(time_in));
    std::pair<glm::vec4, glm::vec4> ret_data;
    
    if(displacement_cache_data_ready.Num() == 0) {
        return;
    }
    
    if((displacement_cache_data_ready[base_time] == false)
       || (displacement_cache_data_ready[end_time] == false))
    {
        return;
    }
        
    TArray<meshDisplacementCache>& base_cache = displacement_cache_table[base_time];
    TArray<meshDisplacementCache>& end_cache = displacement_cache_table[end_time];
    
    for(auto i = 0; i < base_cache.Num(); i++) {
        const meshDisplacementCache& base_data = base_cache[i];
        const meshDisplacementCache& end_data = end_cache[i];
        const FString& cur_key = base_data.getKey();
        
        if(cur_key == key_in) {
            if(region->getUseLocalDisplacements()) {
                TArray<glm::vec2>& displacements =
                region->getLocalDisplacements();
                for(auto i = 0; i < displacements.Num(); i++) {
                    glm::vec2 interp_val =
                    ((1.0f - ratio) * base_data.getLocalDisplacements()[i]) +
                    (ratio * end_data.getLocalDisplacements()[i]);
                    displacements[i] = interp_val;
                }

            }

            if(region->getUsePostDisplacements()) {
                TArray<glm::vec2>& displacements =
                region->getPostDisplacements();
                for(auto i = 0; i < displacements.Num(); i++) {
                    glm::vec2 interp_val =
                    ((1.0f - ratio) * base_data.getPostDisplacements()[i]) +
                    (ratio * end_data.getPostDisplacements()[i]);
                    displacements[i] = interp_val;
                }
            }
            
            break;
        }
    }
}

void
meshDisplacementCacheManager::retrieveSingleDisplacementValueNoRegionAtTime(const FString& key_in,
                                                                            float time_in,
                                                                            meshRenderRegion * region,
                                                                            TArray<glm::vec2>& out_displacements)
{
    int32 base_time = getIndexByTime((int32)floorf(time_in));
    int32 end_time = getIndexByTime((int32)ceilf(time_in));
    float ratio = (time_in - (float)floorf(time_in));
    std::pair<glm::vec4, glm::vec4> ret_data;
    
    if(displacement_cache_data_ready.Num() == 0) {
        return;
    }
    
    if((displacement_cache_data_ready[base_time] == false)
       || (displacement_cache_data_ready[end_time] == false))
    {
        return;
    }
        
    TArray<meshDisplacementCache>& base_cache = displacement_cache_table[base_time];
    TArray<meshDisplacementCache>& end_cache = displacement_cache_table[end_time];
    
    for(auto i = 0; i < base_cache.Num(); i++) {
        const meshDisplacementCache& base_data = base_cache[i];
        const meshDisplacementCache& end_data = end_cache[i];
        const FString& cur_key = base_data.getKey();
        
        if(cur_key == key_in) {
            if(region->getUseLocalDisplacements()) {
                TArray<glm::vec2>& displacements =
                region->getLocalDisplacements();
                for(auto i = 0; i < displacements.Num(); i++) {
                    glm::vec2 interp_val =
                    ((1.0f - ratio) * base_data.getLocalDisplacements()[i]) +
                    (ratio * end_data.getLocalDisplacements()[i]);
                    out_displacements[i] = interp_val;
                }
                
            }
            
            if(region->getUsePostDisplacements()) {
                TArray<glm::vec2>& displacements =
                region->getPostDisplacements();
                for(auto i = 0; i < displacements.Num(); i++) {
                    glm::vec2 interp_val =
                    ((1.0f - ratio) * base_data.getPostDisplacements()[i]) +
                    (ratio * end_data.getPostDisplacements()[i]);
                    out_displacements[i] = interp_val;
                }
            }
            
            break;
        }
    }
}

void
meshDisplacementCacheManager::retrieveSingleDisplacementValueDirectAtTime(const FString& key_in,
                                                                          float time_in,
                                                                          TArray<glm::vec2>& out_local_displacements,
                                                                          TArray<glm::vec2>& out_post_displacements)
{
    int32 base_time = getIndexByTime((int32)floorf(time_in));
    int32 end_time = getIndexByTime((int32)ceilf(time_in));
    float ratio = (time_in - (float)floorf(time_in));
    std::pair<glm::vec4, glm::vec4> ret_data;
    
    if(displacement_cache_data_ready.Num() == 0) {
        return;
    }
    
    if((displacement_cache_data_ready[base_time] == false)
       || (displacement_cache_data_ready[end_time] == false))
    {
        return;
    }
        
    TArray<meshDisplacementCache>& base_cache = displacement_cache_table[base_time];
    TArray<meshDisplacementCache>& end_cache = displacement_cache_table[end_time];
    
    for(auto i = 0; i < base_cache.Num(); i++) {
        const meshDisplacementCache& base_data = base_cache[i];
        const meshDisplacementCache& end_data = end_cache[i];
        const FString& cur_key = base_data.getKey();
        
        if(cur_key == key_in) {
            bool has_local_displacements = (base_data.getLocalDisplacements().Num() != 0);
            bool has_post_displacements = (base_data.getPostDisplacements().Num() != 0);
            
            if(has_local_displacements) {
                out_local_displacements.SetNumZeroed(base_data.getLocalDisplacements().Num());
                for(auto i = 0; i < base_data.getLocalDisplacements().Num(); i++) {
                    glm::vec2 interp_val =
                    ((1.0f - ratio) * base_data.getLocalDisplacements()[i]) +
                    (ratio * end_data.getLocalDisplacements()[i]);
                    out_local_displacements[i] = interp_val;
                }
                
            }
            
            if(has_post_displacements) {
                out_post_displacements.SetNumZeroed(base_data.getPostDisplacements().Num());
                for(auto i = 0; i < base_data.getPostDisplacements().Num(); i++) {
                    glm::vec2 interp_val =
                    ((1.0f - ratio) * base_data.getPostDisplacements()[i]) +
                    (ratio * end_data.getPostDisplacements()[i]);
                    out_post_displacements[i] = interp_val;
                }
            }
            
            break;
        }
    }
}


bool meshDisplacementCacheManager::allReady()
{
    if(is_ready) {
        return true;
    }
    else {
        int32 num_frames = end_time - start_time + 1;
        int32 ready_cnt = 0;
        for(auto i = 0; i < displacement_cache_data_ready.Num(); i++) {
            if(displacement_cache_data_ready[i]) {
                ready_cnt++;
            }
        }
        
        if(ready_cnt == num_frames) {
            is_ready = true;
        }
    }
    
    return is_ready;
}

// meshUVWarpCacheManager
meshUVWarpCacheManager::meshUVWarpCacheManager()
{
    is_ready = false;
}

meshUVWarpCacheManager::~meshUVWarpCacheManager()
{
    
}

void
meshUVWarpCacheManager::init(int32 start_time_in, int32 end_time_in)
{
    start_time = start_time_in;
    end_time = end_time_in;
    
    int32 num_frames = end_time - start_time + 1;
    uv_cache_table.Empty();
    uv_cache_table.SetNumZeroed(num_frames);
    
    uv_cache_data_ready.Empty();
    uv_cache_data_ready.SetNumZeroed(num_frames);
    for(auto i = 0; i < uv_cache_data_ready.Num(); i++) {
        uv_cache_data_ready[i] = false;
    }
    
    is_ready = false;
}

void
meshUVWarpCacheManager::makeAllReady()
{
    for(auto i = 0; i < uv_cache_data_ready.Num(); i++) {
        uv_cache_data_ready[i] = true;
    }
}

int32
meshUVWarpCacheManager::getStartTime() const
{
    return start_time;
}

int32
meshUVWarpCacheManager::getEndime() const
{
    return end_time;
}

TArray<TArray<meshUVWarpCache> >&
meshUVWarpCacheManager::getCacheTable()
{
    return uv_cache_table;
}

int32
meshUVWarpCacheManager::getIndexByTime(int32 time_in) const
{
    int32 retval = time_in - start_time;
    retval = clipNumber(retval, 0, (int32)uv_cache_table.Num() - 1);

    return retval;
}

void
meshUVWarpCacheManager::setValuesAtTime(int32 time_in,
                                        TMap<FString, meshRenderRegion *>& regions_map)
{
    int32 set_index = getIndexByTime(time_in);
    TArray<meshUVWarpCache> cache_list;
    for(auto& cur_iter : regions_map) {
        meshUVWarpCache new_data(cur_iter.Value->getName());
        new_data.setUvWarpLocalOffset(cur_iter.Value->getUvWarpLocalOffset());
        new_data.setUvWarpGlobalOffset(cur_iter.Value->getUvWarpGlobalOffset());
        new_data.setUvWarpScale(cur_iter.Value->getUvWarpScale());
		new_data.setLevel(cur_iter.Value->getUVLevel());
        
        auto cur_region = cur_iter.Value;
        if(cur_region->getUseUvWarp())
        {
            new_data.setEnabled(true);
        }
        
        cache_list.Add(new_data);
    }
    
    uv_cache_table[set_index] = cache_list;
    uv_cache_data_ready[set_index] = true;
}

void
meshUVWarpCacheManager::retrieveValuesAtTime(float time_in,
                                            TMap<FString, meshRenderRegion *>& regions_map)
{
    int32 base_time = getIndexByTime((int32)floorf(time_in));
    int32 end_time = getIndexByTime((int32)ceilf(time_in));
    
    if(uv_cache_data_ready.Num() == 0) {
        return;
    }
    
    if((uv_cache_data_ready[base_time] == false)
       || (uv_cache_data_ready[end_time] == false))
    {
        return;
    }
        
    TArray<meshUVWarpCache>& base_cache = uv_cache_table[base_time];
    
    for(auto i = 0; i < base_cache.Num(); i++) {
        const meshUVWarpCache& base_data = base_cache[i];
        const FString& cur_key = base_data.getKey();
        
        meshRenderRegion * set_region = regions_map[cur_key];
        if(set_region->getUseUvWarp() || base_data.getEnabled())
        {
            glm::vec2 final_local_offset = base_data.getUvWarpLocalOffset();
            
            glm::vec2 final_global_offset = base_data.getUvWarpGlobalOffset();
            
            glm::vec2 final_scale = base_data.getUvWarpScale();
            
            
            set_region->setUvWarpLocalOffset(final_local_offset);
            set_region->setUvWarpGlobalOffset(final_global_offset);
            set_region->setUvWarpScale(final_scale);
			set_region->setUVLevel(base_data.getLevel());
        }
    }
}

void
meshUVWarpCacheManager::retrieveSingleValueAtTime(float time_in,
                                                  meshRenderRegion * region,
                                                  glm::vec2& local_offset,
                                                  glm::vec2& global_offset,
                                                  glm::vec2& scale)
{
    int32 base_time = getIndexByTime((int32)floorf(time_in));
    int32 end_time = getIndexByTime((int32)ceilf(time_in));
    
    if(uv_cache_data_ready.Num() == 0) {
        return;
    }
    
    if((uv_cache_data_ready[base_time] == false)
       || (uv_cache_data_ready[end_time] == false))
    {
        return;
    }
    
    TArray<meshUVWarpCache>& base_cache = uv_cache_table[base_time];
    TArray<meshUVWarpCache>& end_cache = uv_cache_table[end_time];
    
    local_offset = glm::vec2(0,0);
    global_offset = glm::vec2(0,0);
    scale = glm::vec2(-1,-1);
    
    for(auto i = 0; i < base_cache.Num(); i++) {
        const meshUVWarpCache& base_data = base_cache[i];
        const meshUVWarpCache& end_data = end_cache[i];
        const FString& cur_key = base_data.getKey();
        
        meshRenderRegion * set_region = region;
        if(cur_key == set_region->getName()) {
            if(set_region->getUseUvWarp() || base_data.getEnabled()) {
				local_offset = base_data.getUvWarpLocalOffset();
                
				global_offset = base_data.getUvWarpGlobalOffset();
                
				scale = base_data.getUvWarpScale();
            }
            
            break;
        }
    }
}

bool
meshUVWarpCacheManager::allReady()
{
    if(is_ready) {
        return true;
    }
    else {
        int32 num_frames = end_time - start_time + 1;
        int32 ready_cnt = 0;
        for(auto i = 0; i < uv_cache_data_ready.Num(); i++) {
            if(uv_cache_data_ready[i]) {
                ready_cnt++;
            }
        }
        
        if(ready_cnt == num_frames) {
            is_ready = true;
        }
    }
    
    return is_ready;
}

// meshOpacityCacheManager
meshOpacityCacheManager::meshOpacityCacheManager()
{
	is_ready = false;
}

meshOpacityCacheManager::~meshOpacityCacheManager()
{

}

void
meshOpacityCacheManager::init(int32 start_time_in, int32 end_time_in)
{
	start_time = start_time_in;
	end_time = end_time_in;

	int32 num_frames = end_time - start_time + 1;
	opacity_cache_table.Empty();
	opacity_cache_table.SetNumZeroed(num_frames);

	opacity_cache_data_ready.Empty();
	opacity_cache_data_ready.SetNumZeroed(num_frames);
	for (auto i = 0; i < opacity_cache_data_ready.Num(); i++) {
		opacity_cache_data_ready[i] = false;
	}

	is_ready = false;
}

void
meshOpacityCacheManager::makeAllReady()
{
	for (auto i = 0; i < opacity_cache_data_ready.Num(); i++) {
		opacity_cache_data_ready[i] = true;
	}
}

int32
meshOpacityCacheManager::getStartTime() const
{
	return start_time;
}

int32
meshOpacityCacheManager::getEndime() const
{
	return end_time;
}

TArray<TArray<meshOpacityCache> >&
meshOpacityCacheManager::getCacheTable()
{
	return opacity_cache_table;
}

int32
meshOpacityCacheManager::getIndexByTime(int32 time_in) const
{
	int32 retval = time_in - start_time;
	retval = clipNumber(retval, 0, (int32)opacity_cache_table.Num() - 1);

	return retval;
}

void
meshOpacityCacheManager::setValuesAtTime(int32 time_in,
						TMap<FString, meshRenderRegion *>& regions_map)
{
	int32 set_index = getIndexByTime(time_in);
	TArray<meshOpacityCache> cache_list;
	for (auto cur_iter : regions_map) {
		meshOpacityCache new_data(cur_iter.Value->getName());
		new_data.setOpacity(cur_iter.Value->getOpacity());

		cache_list.Add(new_data);
	}

	opacity_cache_table[set_index] = cache_list;
	opacity_cache_data_ready[set_index] = true;
}

void
meshOpacityCacheManager::retrieveValuesAtTime(float time_in,
											TMap<FString, meshRenderRegion *>& regions_map)
{
	int32 base_time = getIndexByTime((int32)floorf(time_in));
	int32 end_time = getIndexByTime((int32)ceilf(time_in));

	if (opacity_cache_data_ready.Num() == 0) {
		return;
	}

	if ((opacity_cache_data_ready[base_time] == false)
		|| (opacity_cache_data_ready[end_time] == false))
	{
		return;
	}

	TArray<meshOpacityCache>& base_cache = opacity_cache_table[base_time];

	for (auto i = 0; i < base_cache.Num(); i++) {
		const meshOpacityCache& base_data = base_cache[i];
		const FString& cur_key = base_data.getKey();

		meshRenderRegion * set_region = regions_map[cur_key];
		float final_opacity = base_data.getOpacity();
		set_region->setOpacity(final_opacity);
	}
}

void
meshOpacityCacheManager::retrieveSingleValueAtTime(float time_in,
												meshRenderRegion * region,
												float& out_opacity)
{
	int32 base_time = getIndexByTime((int32)floorf(time_in));
	int32 end_time = getIndexByTime((int32)ceilf(time_in));

	if (opacity_cache_data_ready.Num() == 0) {
		return;
	}

	if ((opacity_cache_data_ready[base_time] == false)
		|| (opacity_cache_data_ready[end_time] == false))
	{
		return;
	}

	TArray<meshOpacityCache>& base_cache = opacity_cache_table[base_time];
	TArray<meshOpacityCache>& end_cache = opacity_cache_table[end_time];


	for (auto i = 0; i < base_cache.Num(); i++) {
		const meshOpacityCache& base_data = base_cache[i];
		const meshOpacityCache& end_data = end_cache[i];
		const FString& cur_key = base_data.getKey();

		meshRenderRegion * set_region = region;
		if (cur_key == set_region->getName()) {
			out_opacity = base_data.getOpacity();

			break;
		}
	}
}

bool
meshOpacityCacheManager::allReady()
{
	if (is_ready) {
		return true;
	}
	else {
		int32 num_frames = end_time - start_time + 1;
		int32 ready_cnt = 0;
		for (auto i = 0; i < opacity_cache_data_ready.Num(); i++) {
			if (opacity_cache_data_ready[i]) {
				ready_cnt++;
			}
		}

		if (ready_cnt == num_frames) {
			is_ready = true;
		}
	}

	return is_ready;
}



