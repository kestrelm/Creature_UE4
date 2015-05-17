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

#include "MeshBone.h"
#include <algorithm>
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
static T clipNum(const T& n, const T& lower, const T& upper) {
    return std::max(lower, std::min(n, upper));
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
        theta += 2.0f * (float)M_PI;
    }

    return theta;
}

// meshBone
meshBone::meshBone(const std::string& key_in,
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
}

meshBone::~meshBone()
{
    
}

void meshBone::setTagId(int value_in)
{
    tag_id = value_in;
}

int meshBone::getTagId() const
{
    return tag_id;
}

void meshBone::deleteChildren()
{
    for(size_t i = 0; i < children.size(); i++) {
        meshBone * cur_bone = children[i];
        cur_bone->deleteChildren();
        delete cur_bone;
    }
    
    children.clear();
}

void meshBone::removeChildBone(meshBone * bone_in)
{
    std::vector<meshBone *> new_children;
    bool remove_found = false;
    for(size_t i = 0; i < children.size(); i++) {
        if(children[i] != bone_in) {
            new_children.push_back(children[i]);
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
        for(size_t i = 0; i < children.size(); i++) {
            children[i]->removeChildBone(bone_in);
        }
    }
}

int
meshBone::getBoneDepth(meshBone * bone_in, int depth) const
{
    if(bone_in == this) {
        return depth;
    }
    
    for(size_t i = 0; i < children.size(); i++) {
        meshBone * cur_bone = children[i];
        int ret_val = cur_bone->getBoneDepth(bone_in, depth + 1);
        if(ret_val != -1) {
            return ret_val;
        }
    }
    
    return -1;
}

std::vector<std::string>
meshBone::getAllBoneKeys() const
{
    std::vector<std::string> ret_data;
    ret_data.push_back(getKey());
    
    for(size_t i = 0; i < children.size(); i++) {
        std::vector<std::string> append_data = children[i]->getAllBoneKeys();
        ret_data.insert(ret_data.end(), append_data.begin(), append_data.end());
    }
    
    return ret_data;
}

std::vector<meshBone *>
meshBone::getAllChildren()
{
    std::vector<meshBone *> ret_data;
    ret_data.push_back(this);
    for(size_t i = 0; i < children.size(); i++) {
        std::vector<meshBone *> append_data = children[i]->getAllChildren();
        ret_data.insert(ret_data.end(), append_data.begin(), append_data.end());
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

const std::string& meshBone::getKey() const
{
    return key;
}

void meshBone::setKey(const std::string& key_in)
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
    
    for(size_t i = 0; i < children.size(); i++) {
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
    
    for(size_t i = 0; i < children.size(); i++) {
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
    for(size_t i = 0; i < children.size(); i++) {
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

void meshBone::computeParentTransforms()
{
    glm::mat4 translate_parent =
    glm::translate(glm::mat4(1.0),
                   glm::vec3(getWorldEndPt().x, getWorldEndPt().y, 0));
    glm::mat4 rotate_parent = calcRotateMat(getWorldEndPt() - getWorldStartPt());
    
    glm::mat4 final_transform = translate_parent * rotate_parent;
    glm::mat4 final_inv_transform = glm::inverse(final_transform);
    
    for(size_t i = 0; i < children.size(); i++) {
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

    
    for(size_t i = 0; i < children.size(); i++) {
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
    children.push_back(bone_in);
}

bool
meshBone::hasBone(meshBone * bone_in) const
{
    for(size_t i = 0; i < children.size(); i++) {
        meshBone* cur_bone = children[i];
        if(cur_bone == bone_in) {
            return true;
        }
    }
    
    return false;
}

meshBone * meshBone::getChildByKey(const std::string& search_key)
{
    if(key == search_key) {
        return this;
    }
    
    meshBone * ret_data = NULL;
    for(size_t i = 0; i < children.size(); i++) {
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
                                   int start_pt_index_in,
                                   int end_pt_index_in,
                                   int start_index_in,
                                   int end_index_in)
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
    
    initUvWarp();
}

meshRenderRegion::~meshRenderRegion() {
}

void meshRenderRegion::setUVLevel(int value_in)
{
	uv_level = value_in;
}

int meshRenderRegion::getUVLevel() const
{
	return uv_level;
}

int meshRenderRegion::getTagId() const
{
    return tag_id;
}

void meshRenderRegion::setTagId(int value_in)
{
    tag_id = value_in;
}

int meshRenderRegion::getStartPtIndex() const
{
    return start_pt_index;
}

int meshRenderRegion::getEndPtIndex() const
{
    return end_pt_index;
}

int meshRenderRegion::getStartIndex() const
{
    return start_index;
}

int meshRenderRegion::getEndIndex() const
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

std::unordered_map<std::string, std::vector<float> >&
meshRenderRegion::getWeights()
{
    return normal_weight_map;
}

void
meshRenderRegion::renameWeightValuesByKey(const std::string& old_key,
                                          const std::string& new_key)
{
    if(normal_weight_map.count(old_key) == 0)
    {
        return;
    }
    
    std::vector<float> weight_values = normal_weight_map[old_key];
    normal_weight_map.erase(old_key);
    normal_weight_map[new_key] = weight_values;
}

void
meshRenderRegion::initFastNormalWeightMap(const std::unordered_map<std::string, meshBone *>& bones_map)
{
    fast_normal_weight_map.clear();
    fast_bones_map.clear();
    reverse_fast_normal_weight_map.clear();
    relevant_bones_indices.clear();
    fill_dq_array.clear();
    
    for(auto& bone_data : bones_map)
    {
        std::vector<float> values = normal_weight_map[bone_data.first];
        fast_normal_weight_map.push_back(values);
        
        fast_bones_map.push_back(bone_data.second);
        
        if(reverse_fast_normal_weight_map.empty())
        {
            reverse_fast_normal_weight_map.resize(values.size());
            relevant_bones_indices.resize(values.size());
        }
    }
    
    fill_dq_array.resize(bones_map.size());
    
    for(size_t i = 0; i < reverse_fast_normal_weight_map.size(); i++)
    {
        // reverse normal weight map
        std::vector<float> new_values(fast_normal_weight_map.size());
        for(size_t j = 0; j < fast_normal_weight_map.size(); j++)
        {
            new_values[j]  = fast_normal_weight_map[j][i];
        }
        
        reverse_fast_normal_weight_map[i] = new_values;
        
        // relevant bone indices
        std::vector<int> relevant_array;
        const float cutoff_val = 0.05f;
        for(size_t j = 0; j < fast_normal_weight_map.size(); j++)
        {
            float sample_val = fast_normal_weight_map[j][i];
            if(sample_val > cutoff_val)
            {
                relevant_array.push_back((int)j);
            }
        }
        
        relevant_bones_indices[i] = relevant_array;
    }
    
    fast_normal_weight_map.clear();
}

int meshRenderRegion::getNumPts() const
{
    return end_pt_index - start_pt_index + 1;
}

int meshRenderRegion::getNumIndices() const
{
    return end_index - start_index + 1;
}

void meshRenderRegion::setMainBoneKey(const std::string& key_in)
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
meshRenderRegion::setName(const std::string& name_in)
{
    name = name_in;
}

const std::string&
meshRenderRegion::getName() const
{
    return name;
}

void
meshRenderRegion::setUseLocalDisplacements(bool flag_in)
{
    use_local_displacements = flag_in;
    if((local_displacements.size() != getNumPts())
       && use_local_displacements)
    {
        local_displacements.resize(getNumPts());
    }
}

bool
meshRenderRegion::getUseLocalDisplacements() const
{
    return use_local_displacements;
}

std::vector<glm::vec2>&
meshRenderRegion::getLocalDisplacements()
{
    return local_displacements;
}

void
meshRenderRegion::clearLocalDisplacements()
{
    for(size_t i = 0; i < local_displacements.size(); i++) {
        local_displacements[i].x = 0;
        local_displacements[i].y = 0;
    }
}

void
meshRenderRegion::clearPostDisplacements()
{
    for(size_t i = 0; i < post_displacements.size(); i++) {
        post_displacements[i].x = 0;
        post_displacements[i].y = 0;
    }
}

void
meshRenderRegion::setUsePostDisplacements(bool flag_in)
{
    use_post_displacements = flag_in;
    if((post_displacements.size() != getNumPts())
       && use_post_displacements)
    {
        post_displacements.resize(getNumPts());
    }
}

bool
meshRenderRegion::getUsePostDisplacements() const
{
    return use_post_displacements;
}

std::vector<glm::vec2>&
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
    uv_warp_ref_uvs.resize(getNumPts());
    glm::float32 * cur_uvs = getUVs();
    for(size_t i = 0; i < uv_warp_ref_uvs.size(); i++) {
        uv_warp_ref_uvs[i].x = *cur_uvs;
        uv_warp_ref_uvs[i].y = *(cur_uvs + 1);
        
        cur_uvs += 2;
    }
}

void
meshRenderRegion::runUvWarp()
{
    glm::float32 * cur_uvs = getUVs();
    for(size_t i = 0; i < uv_warp_ref_uvs.size(); i++) {
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
    for(size_t i = 0; i < uv_warp_ref_uvs.size(); i++) {
        glm::vec2 set_uv = uv_warp_ref_uvs[i];
        cur_uvs[0] = set_uv.x;
        cur_uvs[1] = set_uv.y;
        
        cur_uvs += 2;
    }
}


glm::vec2
meshRenderRegion::getRestLocalPt(int index_in) const
{
    glm::float32 * read_pt = getRestPts() + (3 * index_in);
    glm::vec2 return_pt(read_pt[0], read_pt[1]);
    return return_pt;
}

glm::vec2
meshRenderRegion::getRestGlobalPt(int index_in) const
{
    glm::float32 * read_pt = store_rest_pts + (3 * index_in);
    glm::vec2 return_pt(read_pt[0], read_pt[1]);
    return return_pt;
}

glm::uint32
meshRenderRegion::getLocalIndex(int index_in) const
{
    glm::uint32 * read_index = getIndices() + index_in;
    return *read_index;
}

void meshRenderRegion::poseFinalPts(glm::float32 * output_pts,
                                    std::unordered_map<std::string, meshBone *>& bones_map)
{
    glm::float32 * read_pt = getRestPts();
    glm::float32 * write_pt = output_pts;
    
    // point posing
    for(int i = 0; i < getNumPts(); i++) {
        glm::vec4 cur_rest_pt(read_pt[0], read_pt[1], read_pt[2], 1);
        
        if(use_local_displacements) {
            cur_rest_pt.x += local_displacements[i].x;
            cur_rest_pt.y += local_displacements[i].y;
        }
        
        glm::mat4 accum_mat(0);
        dualQuat accum_dq;
        
        int n_index = 0;
        for(auto cur_iter = bones_map.begin(); cur_iter != bones_map.end();
            ++cur_iter)
        {
            const std::string& cur_key = cur_iter->first;
            meshBone * cur_bone = cur_iter->second;
            float cur_weight_val = 0;
            if(fast_normal_weight_map.empty() == false) {
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

void meshRenderRegion::poseFastFinalPts(glm::float32 * output_pts)
{
    glm::float32 * read_pt = getRestPts();
    glm::float32 * write_pt = output_pts;
    
    // fill up dqs
    for(size_t i = 0; i < fill_dq_array.size(); i++)
    {
        fill_dq_array[i] = fast_bones_map[i]->getWorldDq();
    }
    
    // pose points
    for(int i = 0; i < getNumPts(); i++) {
        glm::vec4 cur_rest_pt(read_pt[0], read_pt[1], read_pt[2], 1);
        
        if(use_local_displacements) {
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

// meshRenderBoneComposition
meshRenderBoneComposition::meshRenderBoneComposition()
{
    
}

meshRenderBoneComposition::~meshRenderBoneComposition()
{
    for(size_t i = 0; i < regions.size(); i++) {
        delete regions[i];
    }
    
}

void meshRenderBoneComposition::addRegion(meshRenderRegion * region_in)
{
    regions.push_back(region_in);
}

meshRenderRegion *
meshRenderBoneComposition::getRegionWithId(int id_in)
{
    for(size_t i = 0; i < regions.size(); i++) {
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

std::unordered_map<std::string, meshBone *>
meshRenderBoneComposition::genBoneMap(meshBone * input_bone)
{
    std::unordered_map<std::string, meshBone *> ret_map;
    std::vector<std::string> all_keys = input_bone->getAllBoneKeys();
    for(size_t i = 0; i < all_keys.size(); i++) {
        std::string cur_key = all_keys[i];
        ret_map[cur_key] = input_bone->getChildByKey(cur_key);
    }
    
    return ret_map;
}

void
meshRenderBoneComposition::initRegionsMap()
{
    regions_map.clear();
    for(size_t i = 0; i < regions.size(); i++) {
        std::string cur_key = regions[i]->getName();
        regions_map[cur_key] = regions[i];
    }
}

std::unordered_map<std::string, meshBone *>&
meshRenderBoneComposition::getBonesMap()
{
    return bones_map;
}

std::unordered_map<std::string, meshRenderRegion *>&
meshRenderBoneComposition::getRegionsMap()
{
    return regions_map;
}

std::vector<meshRenderRegion *>&
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
meshBoneCache::meshBoneCache(const std::string& key_in)
{
    key = key_in;
}

meshBoneCache::~meshBoneCache()
{
    
}

// meshDisplacementCache
meshDisplacementCache::meshDisplacementCache(const std::string& key_in)
{
    key = key_in;
}

meshDisplacementCache::~meshDisplacementCache()
{
    
}

const std::vector<glm::vec2>&
meshDisplacementCache::getLocalDisplacements() const
{
    return local_displacements;
}

const std::vector<glm::vec2>&
meshDisplacementCache::getPostDisplacements() const
{
    return post_displacements;
}


// meshUVWarpCache
meshUVWarpCache::meshUVWarpCache(const std::string& key_in)
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
meshBoneCacheManager::init(int start_time_in, int end_time_in)
{
    start_time = start_time_in;
    end_time = end_time_in;
    
    int num_frames = end_time - start_time + 1;
    bone_cache_table.clear();
    bone_cache_table.resize(num_frames);
    
    bone_cache_data_ready.clear();
    bone_cache_data_ready.resize(num_frames);
    for(size_t i = 0; i < bone_cache_data_ready.size(); i++) {
        bone_cache_data_ready[i] = false;
    }
    
    is_ready = false;
}

void
meshBoneCacheManager::makeAllReady()
{
    for(size_t i = 0; i < bone_cache_data_ready.size(); i++) {
        bone_cache_data_ready[i] = true;
    }
}

std::vector<std::vector<meshBoneCache> >&
meshBoneCacheManager::getCacheTable()
{
    return bone_cache_table;
}

int meshBoneCacheManager::getStartTime() const
{
    return start_time;
}

int meshBoneCacheManager::getEndime() const
{
    return end_time;
}

int
meshBoneCacheManager::getIndexByTime(int time_in) const
{
    int retval = time_in - start_time;
    retval = clipNum(retval, 0, (int)bone_cache_table.size() - 1);

    return retval;
}

void
meshBoneCacheManager::setValuesAtTime(int time_in,
                                      std::unordered_map<std::string, meshBone *>& bone_map)
{
    data_lock.lock();

    std::vector<meshBoneCache> cache_list;
    int set_index = getIndexByTime(time_in);
    for(auto cur_iter = bone_map.begin();
        cur_iter != bone_map.end();
        ++cur_iter)
    {
        const std::string& cur_key = cur_iter->first;
        meshBone * cur_bone = cur_iter->second;
        
        meshBoneCache new_cache(cur_key);
        new_cache.setWorldStartPt(cur_bone->getWorldStartPt());
        new_cache.setWorldEndPt(cur_bone->getWorldEndPt());
        cache_list.push_back(new_cache);
    }
    
    bone_cache_table[set_index] = cache_list;
    bone_cache_data_ready[set_index] = true;
    
    data_lock.unlock();
}

bool
meshBoneCacheManager::allReady()
{
    if(is_ready) {
        return true;
    }
    else {
        int num_frames = end_time - start_time + 1;
        int ready_cnt = 0;
        for(size_t i = 0; i < bone_cache_data_ready.size(); i++) {
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
                                           std::unordered_map<std::string, meshBone *>& bone_map)
{
    int base_time = getIndexByTime((int)floorf(time_in));
    int end_time = getIndexByTime((int)ceilf(time_in));

    float ratio = (time_in - (float)floorf(time_in));

    if(bone_cache_data_ready.empty()) {
        return;
    }
    
    if((bone_cache_data_ready[base_time] == false)
       || (bone_cache_data_ready[end_time] == false))
    {
        return;
    }
    
    data_lock.lock();

    std::vector<meshBoneCache>& base_cache = bone_cache_table[base_time];
    std::vector<meshBoneCache>& end_cache = bone_cache_table[end_time];
    
    for(size_t i = 0; i < base_cache.size(); i++) {
        const meshBoneCache& base_data = base_cache[i];
        const meshBoneCache& end_data = end_cache[i];
        const std::string& cur_key = base_data.getKey();
        
        glm::vec4 final_world_start_pt = ((1.0f - ratio) * base_data.getWorldStartPt()) +
                                        (ratio * end_data.getWorldStartPt());
        
        glm::vec4 final_world_end_pt = ((1.0f - ratio) * base_data.getWorldEndPt()) +
                                        (ratio * end_data.getWorldEndPt());
        
        bone_map[cur_key]->setWorldStartPt(final_world_start_pt);
        bone_map[cur_key]->setWorldEndPt(final_world_end_pt);
    }
    
    data_lock.unlock();
}

std::pair<glm::vec4, glm::vec4>
meshBoneCacheManager::retrieveSingleBoneValueAtTime(const std::string& key_in,
                                                    float time_in)
{
    int base_time = getIndexByTime((int)floorf(time_in));
    int end_time = getIndexByTime((int)ceilf(time_in));
    float ratio = (time_in - (float)floorf(time_in));
    std::pair<glm::vec4, glm::vec4> ret_data;

    if(bone_cache_data_ready.empty()) {
        return ret_data;
    }
    
    if((bone_cache_data_ready[base_time] == false)
       || (bone_cache_data_ready[end_time] == false))
    {
        return ret_data;
    }

    data_lock.lock();

    std::vector<meshBoneCache>& base_cache = bone_cache_table[base_time];
    std::vector<meshBoneCache>& end_cache = bone_cache_table[end_time];
    
    for(size_t i = 0; i < base_cache.size(); i++) {
        const meshBoneCache& base_data = base_cache[i];
        const meshBoneCache& end_data = end_cache[i];
        const std::string& cur_key = base_data.getKey();
        
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

    data_lock.unlock();

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

void meshDisplacementCacheManager::init(int start_time_in, int end_time_in)
{
    start_time = start_time_in;
    end_time = end_time_in;
    
    int num_frames = end_time - start_time + 1;
    displacement_cache_table.clear();
    displacement_cache_table.resize(num_frames);
    
    displacement_cache_data_ready.clear();
    displacement_cache_data_ready.resize(num_frames);
    for(size_t i = 0; i < displacement_cache_data_ready.size(); i++) {
        displacement_cache_data_ready[i] = false;
    }
    
    is_ready = false;

}

void
meshDisplacementCacheManager::makeAllReady()
{
    for(size_t i = 0; i < displacement_cache_data_ready.size(); i++) {
        displacement_cache_data_ready[i] = true;
    }
}

std::vector<std::vector<meshDisplacementCache> >&
meshDisplacementCacheManager::getCacheTable()
{
    return displacement_cache_table;
}

int meshDisplacementCacheManager::getStartTime() const
{
    return start_time;
}

int meshDisplacementCacheManager::getEndime() const
{
    return end_time;
}

int meshDisplacementCacheManager::getIndexByTime(int time_in) const
{
    int retval = time_in - start_time;
    retval = clipNum(retval, 0, (int)displacement_cache_table.size() - 1);

    return retval;
}

void meshDisplacementCacheManager::setValuesAtTime(int time_in,
                                                   std::unordered_map<std::string,meshRenderRegion *>& regions_map)
{
    data_lock.lock();
    
    std::vector<meshDisplacementCache> cache_list;
    int set_index = getIndexByTime(time_in);
    for(auto cur_iter = regions_map.begin();
        cur_iter != regions_map.end();
        ++cur_iter)
    {
        const std::string& cur_key = cur_iter->first;
        meshRenderRegion * cur_region = cur_iter->second;
        
        meshDisplacementCache new_cache(cur_key);
        if(cur_region->getUseLocalDisplacements()) {
            new_cache.setLocalDisplacements(cur_region->getLocalDisplacements());
        }
        
        if(cur_region->getUsePostDisplacements()) {
            new_cache.setPostDisplacements(cur_region->getPostDisplacements());
        }
        
        cache_list.push_back(new_cache);
    }
    
    displacement_cache_table[set_index] = cache_list;
    displacement_cache_data_ready[set_index] = true;
    
    data_lock.unlock();
}

void meshDisplacementCacheManager::retrieveValuesAtTime(float time_in,
                                                        std::unordered_map<std::string,meshRenderRegion *>& regions_map)
{
    int base_time = getIndexByTime((int)floorf(time_in));
    int end_time = getIndexByTime((int)ceilf(time_in));
    
    float ratio = (time_in - (float)floorf(time_in));
    
    if(displacement_cache_data_ready.empty()) {
        return;
    }
    
    if((displacement_cache_data_ready[base_time] == false)
       || (displacement_cache_data_ready[end_time] == false))
    {
        return;
    }
    
    data_lock.lock();
    
    std::vector<meshDisplacementCache>& base_cache = displacement_cache_table[base_time];
    std::vector<meshDisplacementCache>& end_cache = displacement_cache_table[end_time];
    
    for(size_t i = 0; i < base_cache.size(); i++) {
        const meshDisplacementCache& base_data = base_cache[i];
        const meshDisplacementCache& end_data = end_cache[i];
        const std::string& cur_key = base_data.getKey();
        
        meshRenderRegion * set_region = regions_map[cur_key];

        if(set_region->getUseLocalDisplacements()) {
            std::vector<glm::vec2>& displacements =
                set_region->getLocalDisplacements();
            if((base_data.getLocalDisplacements().size() == displacements.size())
               && (end_data.getLocalDisplacements().size() == displacements.size()))
            {
                for(size_t j = 0; j < displacements.size(); j++) {
                    glm::vec2 interp_val =
                    ((1.0f - ratio) * base_data.getLocalDisplacements()[j]) +
                    (ratio * end_data.getLocalDisplacements()[j]);
                    displacements[j] = interp_val;
                }
            }
            else {
                std::fill(displacements.begin(), displacements.end(), glm::vec2(0, 0));
            }
        }
        
        if(set_region->getUsePostDisplacements()) {
            std::vector<glm::vec2>& displacements =
                set_region->getPostDisplacements();
            if((base_data.getPostDisplacements().size() == displacements.size())
               && (end_data.getPostDisplacements().size() == displacements.size()))
            {
                
                for(size_t j = 0; j < displacements.size(); j++) {
                    glm::vec2 interp_val =
                    ((1.0f - ratio) * base_data.getPostDisplacements()[j]) +
                    (ratio * end_data.getPostDisplacements()[j]);
                    displacements[j] = interp_val;
                }
            }
            else {
                std::fill(displacements.begin(), displacements.end(), glm::vec2(0, 0));
            }
        }
    }
    
    data_lock.unlock();
}

void
meshDisplacementCacheManager::retrieveSingleDisplacementValueAtTime(const std::string& key_in,
                                                                   float time_in,
                                                                    meshRenderRegion * region)
{
    int base_time = getIndexByTime((int)floorf(time_in));
    int end_time = getIndexByTime((int)ceilf(time_in));
    float ratio = (time_in - (float)floorf(time_in));
    std::pair<glm::vec4, glm::vec4> ret_data;
    
    if(displacement_cache_data_ready.empty()) {
        return;
    }
    
    if((displacement_cache_data_ready[base_time] == false)
       || (displacement_cache_data_ready[end_time] == false))
    {
        return;
    }
    
    data_lock.lock();
    
    std::vector<meshDisplacementCache>& base_cache = displacement_cache_table[base_time];
    std::vector<meshDisplacementCache>& end_cache = displacement_cache_table[end_time];
    
    for(size_t i = 0; i < base_cache.size(); i++) {
        const meshDisplacementCache& base_data = base_cache[i];
        const meshDisplacementCache& end_data = end_cache[i];
        const std::string& cur_key = base_data.getKey();
        
        if(cur_key == key_in) {
            if(region->getUseLocalDisplacements()) {
                std::vector<glm::vec2>& displacements =
                region->getLocalDisplacements();
                for(size_t i = 0; i < displacements.size(); i++) {
                    glm::vec2 interp_val =
                    ((1.0f - ratio) * base_data.getLocalDisplacements()[i]) +
                    (ratio * end_data.getLocalDisplacements()[i]);
                    displacements[i] = interp_val;
                }

            }

            if(region->getUsePostDisplacements()) {
                std::vector<glm::vec2>& displacements =
                region->getPostDisplacements();
                for(size_t i = 0; i < displacements.size(); i++) {
                    glm::vec2 interp_val =
                    ((1.0f - ratio) * base_data.getPostDisplacements()[i]) +
                    (ratio * end_data.getPostDisplacements()[i]);
                    displacements[i] = interp_val;
                }
            }
            
            break;
        }
    }
    
    data_lock.unlock();
}

void
meshDisplacementCacheManager::retrieveSingleDisplacementValueNoRegionAtTime(const std::string& key_in,
                                                                            float time_in,
                                                                            meshRenderRegion * region,
                                                                            std::vector<glm::vec2>& out_displacements)
{
    int base_time = getIndexByTime((int)floorf(time_in));
    int end_time = getIndexByTime((int)ceilf(time_in));
    float ratio = (time_in - (float)floorf(time_in));
    std::pair<glm::vec4, glm::vec4> ret_data;
    
    if(displacement_cache_data_ready.empty()) {
        return;
    }
    
    if((displacement_cache_data_ready[base_time] == false)
       || (displacement_cache_data_ready[end_time] == false))
    {
        return;
    }
    
    data_lock.lock();
    
    std::vector<meshDisplacementCache>& base_cache = displacement_cache_table[base_time];
    std::vector<meshDisplacementCache>& end_cache = displacement_cache_table[end_time];
    
    for(size_t i = 0; i < base_cache.size(); i++) {
        const meshDisplacementCache& base_data = base_cache[i];
        const meshDisplacementCache& end_data = end_cache[i];
        const std::string& cur_key = base_data.getKey();
        
        if(cur_key == key_in) {
            if(region->getUseLocalDisplacements()) {
                std::vector<glm::vec2>& displacements =
                region->getLocalDisplacements();
                for(size_t i = 0; i < displacements.size(); i++) {
                    glm::vec2 interp_val =
                    ((1.0f - ratio) * base_data.getLocalDisplacements()[i]) +
                    (ratio * end_data.getLocalDisplacements()[i]);
                    out_displacements[i] = interp_val;
                }
                
            }
            
            if(region->getUsePostDisplacements()) {
                std::vector<glm::vec2>& displacements =
                region->getPostDisplacements();
                for(size_t i = 0; i < displacements.size(); i++) {
                    glm::vec2 interp_val =
                    ((1.0f - ratio) * base_data.getPostDisplacements()[i]) +
                    (ratio * end_data.getPostDisplacements()[i]);
                    out_displacements[i] = interp_val;
                }
            }
            
            break;
        }
    }
    
    data_lock.unlock();
}

void
meshDisplacementCacheManager::retrieveSingleDisplacementValueDirectAtTime(const std::string& key_in,
                                                                          float time_in,
                                                                          std::vector<glm::vec2>& out_local_displacements,
                                                                          std::vector<glm::vec2>& out_post_displacements)
{
    int base_time = getIndexByTime((int)floorf(time_in));
    int end_time = getIndexByTime((int)ceilf(time_in));
    float ratio = (time_in - (float)floorf(time_in));
    std::pair<glm::vec4, glm::vec4> ret_data;
    
    if(displacement_cache_data_ready.empty()) {
        return;
    }
    
    if((displacement_cache_data_ready[base_time] == false)
       || (displacement_cache_data_ready[end_time] == false))
    {
        return;
    }
    
    data_lock.lock();
    
    std::vector<meshDisplacementCache>& base_cache = displacement_cache_table[base_time];
    std::vector<meshDisplacementCache>& end_cache = displacement_cache_table[end_time];
    
    for(size_t i = 0; i < base_cache.size(); i++) {
        const meshDisplacementCache& base_data = base_cache[i];
        const meshDisplacementCache& end_data = end_cache[i];
        const std::string& cur_key = base_data.getKey();
        
        if(cur_key == key_in) {
            bool has_local_displacements = !base_data.getLocalDisplacements().empty();
            bool has_post_displacements = !base_data.getPostDisplacements().empty();
            
            if(has_local_displacements) {
                out_local_displacements.resize(base_data.getLocalDisplacements().size());
                for(size_t i = 0; i < base_data.getLocalDisplacements().size(); i++) {
                    glm::vec2 interp_val =
                    ((1.0f - ratio) * base_data.getLocalDisplacements()[i]) +
                    (ratio * end_data.getLocalDisplacements()[i]);
                    out_local_displacements[i] = interp_val;
                }
                
            }
            
            if(has_post_displacements) {
                out_post_displacements.resize(base_data.getPostDisplacements().size());
                for(size_t i = 0; i < base_data.getPostDisplacements().size(); i++) {
                    glm::vec2 interp_val =
                    ((1.0f - ratio) * base_data.getPostDisplacements()[i]) +
                    (ratio * end_data.getPostDisplacements()[i]);
                    out_post_displacements[i] = interp_val;
                }
            }
            
            break;
        }
    }
    
    data_lock.unlock();
}


bool meshDisplacementCacheManager::allReady()
{
    if(is_ready) {
        return true;
    }
    else {
        int num_frames = end_time - start_time + 1;
        int ready_cnt = 0;
        for(size_t i = 0; i < displacement_cache_data_ready.size(); i++) {
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
meshUVWarpCacheManager::init(int start_time_in, int end_time_in)
{
    start_time = start_time_in;
    end_time = end_time_in;
    
    int num_frames = end_time - start_time + 1;
    uv_cache_table.clear();
    uv_cache_table.resize(num_frames);
    
    uv_cache_data_ready.clear();
    uv_cache_data_ready.resize(num_frames);
    for(size_t i = 0; i < uv_cache_data_ready.size(); i++) {
        uv_cache_data_ready[i] = false;
    }
    
    is_ready = false;
}

void
meshUVWarpCacheManager::makeAllReady()
{
    for(size_t i = 0; i < uv_cache_data_ready.size(); i++) {
        uv_cache_data_ready[i] = true;
    }
}

int
meshUVWarpCacheManager::getStartTime() const
{
    return start_time;
}

int
meshUVWarpCacheManager::getEndime() const
{
    return end_time;
}

std::vector<std::vector<meshUVWarpCache> >&
meshUVWarpCacheManager::getCacheTable()
{
    return uv_cache_table;
}

int
meshUVWarpCacheManager::getIndexByTime(int time_in) const
{
    int retval = time_in - start_time;
    retval = clipNum(retval, 0, (int)uv_cache_table.size() - 1);

    return retval;
}

void
meshUVWarpCacheManager::setValuesAtTime(int time_in,
                                        std::unordered_map<std::string, meshRenderRegion *>& regions_map)
{
    data_lock.lock();
    
    int set_index = getIndexByTime(time_in);
    std::vector<meshUVWarpCache> cache_list;
    for(auto cur_iter : regions_map) {
        meshUVWarpCache new_data(cur_iter.second->getName());
        new_data.setUvWarpLocalOffset(cur_iter.second->getUvWarpLocalOffset());
        new_data.setUvWarpGlobalOffset(cur_iter.second->getUvWarpGlobalOffset());
        new_data.setUvWarpScale(cur_iter.second->getUvWarpScale());
		new_data.setLevel(cur_iter.second->getUVLevel());
        
        cache_list.push_back(new_data);
    }
    
    uv_cache_table[set_index] = cache_list;
    uv_cache_data_ready[set_index] = true;
    
    data_lock.unlock();
}

void
meshUVWarpCacheManager::retrieveValuesAtTime(float time_in,
                                            std::unordered_map<std::string, meshRenderRegion *>& regions_map)
{
    int base_time = getIndexByTime((int)floorf(time_in));
    int end_time = getIndexByTime((int)ceilf(time_in));
    
    if(uv_cache_data_ready.empty()) {
        return;
    }
    
    if((uv_cache_data_ready[base_time] == false)
       || (uv_cache_data_ready[end_time] == false))
    {
        return;
    }
    
    data_lock.lock();
    
    std::vector<meshUVWarpCache>& base_cache = uv_cache_table[base_time];
    
    for(size_t i = 0; i < base_cache.size(); i++) {
        const meshUVWarpCache& base_data = base_cache[i];
        const std::string& cur_key = base_data.getKey();
        
        meshRenderRegion * set_region = regions_map[cur_key];
        if(set_region->getUseUvWarp()) {
            glm::vec2 final_local_offset = base_data.getUvWarpLocalOffset();
            
            glm::vec2 final_global_offset = base_data.getUvWarpGlobalOffset();
            
            glm::vec2 final_scale = base_data.getUvWarpScale();
            
            
            set_region->setUvWarpLocalOffset(final_local_offset);
            set_region->setUvWarpGlobalOffset(final_global_offset);
            set_region->setUvWarpScale(final_scale);
			set_region->setUVLevel(base_data.getLevel());
        }
    }
    
    data_lock.unlock();
}

void
meshUVWarpCacheManager::retrieveSingleValueAtTime(float time_in,
                                                  meshRenderRegion * region,
                                                  glm::vec2& local_offset,
                                                  glm::vec2& global_offset,
                                                  glm::vec2& scale)
{
    int base_time = getIndexByTime((int)floorf(time_in));
    int end_time = getIndexByTime((int)ceilf(time_in));
    
    if(uv_cache_data_ready.empty()) {
        return;
    }
    
    if((uv_cache_data_ready[base_time] == false)
       || (uv_cache_data_ready[end_time] == false))
    {
        return;
    }
    
    data_lock.lock();
    
    std::vector<meshUVWarpCache>& base_cache = uv_cache_table[base_time];
    std::vector<meshUVWarpCache>& end_cache = uv_cache_table[end_time];
    
    local_offset = glm::vec2(0,0);
    global_offset = glm::vec2(0,0);
    scale = glm::vec2(-1,-1);
    
    for(size_t i = 0; i < base_cache.size(); i++) {
        const meshUVWarpCache& base_data = base_cache[i];
        const meshUVWarpCache& end_data = end_cache[i];
        const std::string& cur_key = base_data.getKey();
        
        meshRenderRegion * set_region = region;
        if(cur_key == set_region->getName()) {
            if(set_region->getUseUvWarp()) {
				local_offset = base_data.getUvWarpLocalOffset();
                
				global_offset = base_data.getUvWarpGlobalOffset();
                
				scale = base_data.getUvWarpScale();
            }
            
            break;
        }
    }
    
    data_lock.unlock();
}

bool
meshUVWarpCacheManager::allReady()
{
    if(is_ready) {
        return true;
    }
    else {
        int num_frames = end_time - start_time + 1;
        int ready_cnt = 0;
        for(size_t i = 0; i < uv_cache_data_ready.size(); i++) {
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


