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
* Running Software on Licensee�s Website[s] and Server[s];
* Allowing 3rd Parties to run Software on Licensee�s Website[s] and Server[s];
* Publishing Software�s output to Licensee and 3rd Parties;
* Distribute verbatim copies of Software�s output (including compiled binaries);
* Modify Software to suit Licensee�s needs and specifications.
* Binary Restricted: Licensee may sublicense Software as a part of a larger work containing more than Software,
* distributed solely in Object or Binary form under a personal, non-sublicensable, limited license. Such redistribution shall be limited to unlimited codebases.
* Non Assignable & Non-Transferable: Licensee may not assign or transfer his rights and duties under this license.
* Commercial, Royalty Free: Licensee may use Software for any purpose, including paid-services, without any royalties
* Including the Right to Create Derivative Works: Licensee may create derivative works based on Software,
* including amending Software�s source code, modifying it, integrating it into a larger work or removing portions of Software,
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

#pragma once

#include "CustomProceduralMeshComponent.h"
#include "CreatureModule.h"
#include <map>
#include <mutex>
#include <memory>

// Creature Core is a thin wrapper between the Creature Runtime and any UE4 Creature Object(s)
// The variables of this class are all made public for easy access since it really just functions as a simple
// interface between the Creature Runtime world and the UE4 environment. You can use CreatureCore in various
// UE4 objects that require Creature Playback functionality. The purpose of this class is to enable you to easily
// slow in Creature Runtime features into whatever UE4 object you need it in, like an Actor, Mesh Component or something 
// else altogether. CreatureActor for example contains a CreatureCore object and performs all its playback functionlity
// using the CreatureCore object.

struct FCreatureBoneData
{
	FVector point1;
	FVector point2;
	FTransform xform;
	FTransform startXform;
	FTransform endXform;
	FName name;
};

class CREATUREPLUGIN_API CreatureCore {
public:
	CreatureCore();

	bool GetAndClearShouldAnimStart();

	bool GetAndClearShouldAnimEnd();

	void UpdateCreatureRender();

	bool InitCreatureRender();

	void FillBoneData();

	void ParseEvents(float deltaTime);

	void ProcessRenderRegions();

	FProceduralMeshTriData GetProcMeshData();

	// Loads a data packet from a file
	static bool LoadDataPacket(const std::string& filename_in);

	// Loads a data packet from a string in memory
	static bool LoadDataPacket(const std::string& filename_in,FString* pSourceData);

	//////////////////////////////////////////////////////////////////////////
	// Loads an animation from a file
	static void LoadAnimation(const std::string& filename_in, const std::string& name_in);

	// Loads the creature character from a file
	TArray<FProceduralMeshTriangle>& LoadCreature(const std::string& filename_in);

	// Adds a loaded animation onto the creature character
	bool AddLoadedAnimation(const std::string& filename_in, const std::string& name_in);

	// Returns the CreatureManager associated with this actor
	CreatureModule::CreatureManager * GetCreatureManager();

	void SetBluePrintActiveAnimation(FName name_in);

	void SetBluePrintBlendActiveAnimation(FName name_in, float factor);

	void SetBluePrintAnimationCustomTimeRange(FName name_in, int32 start_time, int32 end_time);

	void MakeBluePrintPointCache(FName name_in, int32 approximation_level);

	void ClearBluePrintPointCache(FName name_in, int32 approximation_level);

	FTransform GetBluePrintBoneXform(FName name_in, bool world_transform, float position_slide_factor, FTransform base_transform);

	bool IsBluePrintBonesCollide(FVector test_point, float bone_size, FTransform base_transform);

	void SetBluePrintAnimationLoop(bool flag_in);

	void SetBluePrintAnimationPlay(bool flag_in);

	void SetBluePrintAnimationPlayFromStart();

	void SetBluePrintAnimationResetToStart();
	void SetBluePrintAnimationResetToEnd();

	float GetBluePrintAnimationFrame();

	void SetBluePrintAnimationFrame(float time_in);

	void SetBluePrintRegionAlpha(FName region_name_in, uint8 alpha_in);

	void RemoveBluePrintRegionAlpha(FName region_name_in);

	void SetBluePrintRegionCustomOrder(TArray<FName> order_in);

	void ClearBluePrintRegionCustomOrder();

	void SetBluePrintRegionItemSwap(FName region_name_in, int32 tag);

	void RemoveBluePrintRegionItemSwap(FName region_name_in);

	void SetUseAnchorPoints(bool flag_in);

	bool GetUseAnchorPoints() const;
	
	void RunBeginPlay();

	bool RunTick(float delta_time);

	// Sets the an active animation by name
	void SetActiveAnimation(const std::string& name_in);

	// Sets the active animation by smoothly blending, factor is a range of ( 0 < factor < 1 )
	void SetAutoBlendActiveAnimation(const std::string& name_in, float factor);

	void SetIsDisabled(bool flag_in);

	void SetDriven(bool flag_in);

	bool GetIsReadyPlay() const;

	void SetGlobalEnablePointCache(bool flag_in);

	bool GetGlobalEnablePointCache();

	glm::uint32 * GetIndicesCopy(int init_size);


	// properties
	FString creature_filename, creature_asset_filename;

	float bone_data_size;

	float bone_data_length_factor;

	float region_overlap_z_delta;

	bool smooth_transitions;

	FString start_animation_name;

	float animation_frame;

	TArray<FProceduralMeshTriangle> draw_triangles;

	std::shared_ptr<CreatureModule::CreatureManager> creature_manager;

	TArray<FCreatureBoneData> bone_data;

	TArray<uint8> region_alphas;

	TMap<FName, uint8> region_alpha_map;

	TArray<FName> region_custom_order;

	FString absolute_creature_filename;

	bool should_play, is_looping;

	bool play_start_done, play_end_done;

	bool is_disabled;

	bool is_driven;

	bool is_ready_play;

	bool should_process_animation_start, should_process_animation_end;

	bool do_file_warning;

	bool should_update_render_indices;

	std::mutex * update_lock;

	//////////////////////////////////////////////////////////////////////////
	//Add by God of Pen
	//////////////////////////////////////////////////////////////////////////

	bool bUsingCreatureAnimatinAsset=false;//���ʹ��CreatureAnimationAsset�Ļ�������Ϊ�棬���ٴ�Ӳ�̶�ȡ��ֱ�Ӵ�Asset��ȡ������Ϣ
	//����AnimationAsset��ȡ��ʱ��ֱ�Ӵ�pJsonData�����룬���ٴ�Ӳ��������
	FString* pJsonData;
	std::shared_ptr<glm::uint32> global_indices_copy;
};

std::string ConvertToString(const FString &str);
std::string ConvertToString(FName name);
