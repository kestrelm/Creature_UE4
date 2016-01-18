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

#ifndef __CocosEngineTest__CreatureModule__
#define __CocosEngineTest__CreatureModule__

#include <iostream>
#include <functional>
#include "MeshBone.h"
#include "gason.h"
#include <map>
#include <unordered_map>
#include <set>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>

namespace CreatureModule {
    
    class CreatureLoadDataPacket {
    public:
        CreatureLoadDataPacket()
        {
            src_chars = NULL;
        }
        
        ~CreatureLoadDataPacket()
        {
            if(src_chars)
            {
                delete [] src_chars;
            }
        }
        
        JsonValue base_node;
        JsonAllocator allocator;
        char * src_chars;
    };
    
    // Opens the json file and returns the entire json structure for a creature
    // Use this to load your creatures and animatons
    void LoadCreatureJSONData(const std::string& filename_in,
                              CreatureLoadDataPacket& load_data);
    
    // Opens the json file compressed in .zip format and returns the entire json structure for a creature
    // Use this to load your creatures and animatons
    void LoadCreatureZipJSONData(const std::string& filename_in,
                                 CreatureLoadDataPacket& load_data);
    
    // Parses and creates a json from an input string and returns the entire json structure for a creature
    // Use this to load your creatures and animatons
    void LoadCreatureJSONDataFromString(const std::string& string_in,
                                        CreatureLoadDataPacket& load_data);

	struct CreatureUVSwapPacket {
		CreatureUVSwapPacket(const glm::vec2& local_offset_in,
			const glm::vec2& global_offset_in,
			const glm::vec2& scale_in,
			int tag_in)
		{
			local_offset = local_offset_in;
			global_offset = global_offset_in;
			scale = scale_in;
			tag = tag_in;
		}

		glm::vec2 local_offset;
		glm::vec2 global_offset;
		glm::vec2 scale;
		int tag;
	};
    
    // Class for the creature character
    class Creature {
    public:
        Creature(CreatureLoadDataPacket& load_data);
        
        virtual ~Creature();
        
        // Returns the filename of the texture
        const std::string& getTextureFilename() const;

        // Fills entire mesh with (r,g,b,a) colours
        void FillRenderColours(glm::uint8 r, glm::uint8 g, glm::uint8 b, glm::uint8 a);
        
        // Returns the global indices
        glm::uint32 * GetGlobalIndices();
        
        // Returns the global rest points
        glm::float32 * GetGlobalPts();
        
        // Returns the global uvs
        glm::float32 * GetGlobalUvs();
        
        // Returns the render points
        glm::float32 * GetRenderPts();

        // Returns the render colours
        glm::uint8 * GetRenderColours();

        // Returns the total number of points
        int GetTotalNumPoints() const;

        // Returns the total number of indices
        int GetTotalNumIndices() const;

        // Returns the render composition object
        meshRenderBoneComposition * GetRenderComposition();

        // Get Available Animation names. Note that these animations might not have been loaded yet.
        const std::vector<std::string>& GetAnimationNames() const;

		// Returns the UV Swap Item Packet map
		const std::map<std::string, std::vector<CreatureUVSwapPacket> >& GetUvSwapPackets() const;

		// Sets up an Active UV Item Swap for a particular region
		void SetActiveItemSwap(const std::string& region_name, int swap_idx);

		// Removes an Active UV Item Swap from a particular region
		void RemoveActiveItemSwap(const std::string& region_name);

		// Returns the Actiev UV Swap Items
		std::map<std::string, int>& GetActiveItemSwaps();
    
    protected:
        
        void LoadFromData(CreatureLoadDataPacket& load_data);
        
        // mesh and skeleton data
        glm::uint32 * global_indices;
        glm::float32 * global_pts, * global_uvs;
        glm::float32 * render_pts;
        glm::uint8 * render_colours;
        int total_num_pts, total_num_indices;
        meshRenderBoneComposition * render_composition;
        std::vector<std::string> animation_names;
		std::map<std::string, std::vector<CreatureUVSwapPacket> > uv_swap_packets;
		std::map<std::string, int> active_uv_swap_actions;
    };
    
    // Class for animating the creature character
    class CreatureAnimation {
    public:
        CreatureAnimation(CreatureLoadDataPacket& load_data,
                          const std::string& name_in);
        
        virtual ~CreatureAnimation();
        
        // Return the start time of the animation
        float getStartTime() const;
        
        // Return the end time of the animation
        float getEndTime() const;

		// Sets the start time of this animation
		void setStartTime(int value_in);

		// Sets the end time of this animation
		void setEndTime(int value_in);
        
        meshBoneCacheManager& getBonesCache();
        
        meshDisplacementCacheManager& getDisplacementCache();
        
        meshUVWarpCacheManager& getUVWarpCache();

		meshOpacityCacheManager& getOpacityCache();
        
        const std::string& getName() const;
        
        bool hasCachePts() const;
        
        std::vector<glm::float32 *>& getCachePts();

		void clearCachePts();
        
        void poseFromCachePts(float time_in, glm::float32 * target_pts, int num_pts);
        
    protected:
        
        void LoadFromData(const std::string& name_in,
                          CreatureLoadDataPacket& load_data);
        
        int getIndexByTime(int time_in) const;

        std::string name;
        float start_time, end_time;
        meshBoneCacheManager bones_cache;
        meshDisplacementCacheManager displacement_cache;
        meshUVWarpCacheManager uv_warp_cache;
		meshOpacityCacheManager opacity_cache;
		std::vector<glm::float32 *> cache_pts;
    };
    
    // Class for managing a collection of animations and a creature character
    class CreatureManager {
    public:
        CreatureManager(std::shared_ptr<CreatureModule::Creature> target_creature_in);
        
        virtual ~CreatureManager();
        
        // Create an animation
        void CreateAnimation(CreatureLoadDataPacket& load_data,
                             const std::string& name_in);
        
        // Add an animation
        void AddAnimation(std::shared_ptr<CreatureModule::CreatureAnimation> animation_in);
        
        // Return an animation
        CreatureModule::CreatureAnimation *
        GetAnimation(const std::string name_in);
        
        // Return the creature
        CreatureModule::Creature *
        GetCreature();
        
        // Sets the current animation to be active by name
        void SetActiveAnimationName(const std::string& name_in, bool check_already_active=false);
        
        // Returns the name of the currently active animation
        const std::string& GetActiveAnimationName() const;
        
        // Returns the table of all animations
        std::unordered_map<std::string, std::shared_ptr<CreatureModule::CreatureAnimation> >&
        GetAllAnimations();
        
        // Returns if animation is playing
        bool GetIsPlaying() const;
        
        // Sets whether the animation is playing
        void SetIsPlaying(bool flag_in);
        
        // Resets animation to start time
        void ResetToStartTimes();
        
        // Sets the run time of the animation
        void setRunTime(float time_in);
        
        // Increments the run time of the animation by a delta value
        void increRunTime(float delta_in);
        
        // Returns the current run time of the animation
        float getRunTime() const;

		// Returns the actual run time if auto blending is involved
		float getActualRunTime() const;
        
        // Runs a single step of the animation for a given delta timestep
        void Update(float delta);
        
        // Sets scaling for time
        void SetTimeScale(float scale_in);
        
        // Enables/Disables blending
        void SetBlending(bool flag_in);
        
        // Sets blending animation names
        void SetBlendingAnimations(const std::string& name_1, const std::string& name_2);
        
        // Sets the blending factor
        void SetBlendingFactor(float value_in);
        
        // Given a set of coordinates in local creature space,
        // see if any bone is in contact
        std::string IsContactBone(const glm::vec2& pt_in,
                                  const glm::mat4& creature_xform,
                                  float radius) const;
        
        // Mirrors the model along the Y-Axis
        void SetMirrorY(bool flag_in);
        
        // Decides whether to use a custom time range or the default
        // animation clip's time range
        void SetUseCustomTimeRange(bool flag_in);
        
        // Sets the values for the custom time range
        void SetCustomTimeRange(int start_time_in, int end_time_in);
        
        // Sets whether the animation loops or not
        void SetShouldLoop(bool flag_in);
        
        // Sets the callback to modify/override bone positions
        void SetBonesOverrideCallback(std::function<void (std::unordered_map<std::string, meshBone *>&) >& callback_in);
        
        // Creates point cache for animation
        void MakePointCache(const std::string& animation_name_in, int gap_step);

		// Clears point cache for animation
		void ClearPointCache(const std::string& animation_name_in);
        
        // Sets auto blending
        void SetAutoBlending(bool flag_in);
        
        // Uses auto blending to blend to the next animation
        void AutoBlendTo(const std::string& animation_name_in, float blend_delta);
        
    protected:

		float correctRunTime(float time_in, const std::string& animation_name);
        
        std::string ProcessContactBone(const glm::vec2& pt_in,
                                       float radius,
                                       meshBone * bone_in) const;

        
        void PoseCreature(const std::string& animation_name_in,
                          glm::float32 * target_pts,
						  float input_run_time);
        
        void ProcessAutoBlending();

		void increAutoBlendRuntimes(float delta_in);

		glm::float32 * interpFloatArray(glm::float32 * first_list, glm::float32 * second_list, float factor, int array_size);

		void ResetBlendTime(const std::string& name_in);

		void UpdateRegionSwitches(const std::string& animation_name_in);

		void PoseJustBones(const std::string& animation_name_in,
								glm::float32 * target_pts,
								float input_run_time);

		void JustRunUVWarps(const std::string& animation_name_in, float input_run_time);

		void RunUVItemSwap();
        
        std::unordered_map<std::string, std::shared_ptr<CreatureModule::CreatureAnimation> > animations;
        std::shared_ptr<CreatureModule::Creature> target_creature;
        std::string active_animation_name;
        bool is_playing;
        float run_time;
        float time_scale;
        glm::float32 * blend_render_pts[2];
        bool do_blending;
        float blending_factor;
        std::string active_blend_animation_names[2];
		std::unordered_map<std::string, float> active_blend_run_times;
        bool mirror_y;
        bool use_custom_time_range;
        int custom_start_time, custom_end_time;
        bool should_loop;
        bool do_auto_blending;
        std::string auto_blend_names[2];
        float auto_blend_delta;
        
        std::function<void (std::unordered_map<std::string, meshBone *>&) > bones_override_callback;
        
    };
};

#endif /* defined(__CocosEngineTest__CreatureModule__) */
