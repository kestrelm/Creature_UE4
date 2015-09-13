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

// This is the Mesh Component version of the Creature Runtime. You can use this class if you want to add the Creature Runtime as a 
// mesh component instead of a regular CreatureActor object.

#pragma once

#include <mutex>
#include <vector>
#include "CustomProceduralMeshComponent.h"
#include "CreatureCore.h"
#include "CreatureMeshComponent.generated.h"

USTRUCT()
struct FCreatureMeshCollectionToken
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	FString animation_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	int32 collection_data_index;
};

USTRUCT()
struct FCreatureMeshCollectionClip
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	FString collection_name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	TArray<FCreatureMeshCollectionToken> sequence_clips;

	int32 active_index;
};

USTRUCT()
struct FCreatureMeshCollection
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	FString creature_filename;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	float animation_speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	UMaterialInterface * collection_material;

	CreatureCore creature_core;
	TArray<FProceduralMeshTriangle> ProceduralMeshTris;
};

// Blueprint event delegates event declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreatureMeshAnimationStartEvent, float, frame);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreatureMeshAnimationEndEvent, float, frame);

/** Component that allows you to specify custom triangle mesh geometry */
UCLASS(editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup=Rendering)
class UCreatureMeshComponent : public UCustomProceduralMeshComponent //, public IInterface_CollisionDataProvider
{
	GENERATED_UCLASS_BODY()

public:
	/** Path/Filename to the Creature JSON. Will accept .zip archives, make sure the file is with a .zip extension */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	FString creature_filename;

	/** Playback speed of the animation, 2.0 is the default */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	float animation_speed;

	/** Size of the returned bone data xform, for colliders */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	float bone_data_size;

	/** Size of the bounding box, used for culling during rendering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	float creature_bounds_scale;

	/** Offset of the bounding box, used for culling during rendering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	FVector creature_bounds_offset;

	/** Displays the bouding box */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	bool creature_debug_draw;

	/** Size of the returned bone data xform, for colliders */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	float bone_data_length_factor;

	/** How much each region is offset by z, useful if you are encountering z fighting rendering artifacts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	float region_overlap_z_delta;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	bool smooth_transitions;

	/** Starting animation clip */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	FString start_animation_name;

	/** Current frame of the animation during playback */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Creature")
	float animation_frame;

	/** A collection of Creature JSONs to load when the game starts, you should fill in this information if you want to playback a collection of Creature JSONs as a single animation clip */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	TArray<FCreatureMeshCollection> collectionData;

	/** Use this in conjunction with the collectionData property. This defines how the collection of JSONs are played back and in what order they are displayed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	TArray<FCreatureMeshCollectionClip> collectionClips;

	/** This enables/disables collection clip playback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	bool enable_collection_playback;

	/** Event that is triggered when the animation starts */
	UPROPERTY(BlueprintAssignable, Category = "Components|Creature")
	FCreatureMeshAnimationStartEvent CreatureAnimationStartEvent;

	/** Event that is triggered when the animation ends */
	UPROPERTY(BlueprintAssignable, Category = "Components|Creature")
	FCreatureMeshAnimationEndEvent CreatureAnimationEndEvent;

	// Returns the CreatureManager associated with this actor
	CreatureModule::CreatureManager * GetCreatureManager();

	// Blueprint version of setting the active animation name
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintActiveAnimation(FString name_in);

	// Blueprint version of setting the blended active animation name
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintBlendActiveAnimation(FString name_in, float factor);

	// Blueprint version of setting a custom time range for a given animation
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintAnimationCustomTimeRange(FString name_in, int32 start_time, int32 end_time);

	// Blueprint function to create a point cache for the creature character. This speeds up the playback performance.
	// A small amount of time will be spent precomputing the point cache. You can reduce this time by increasing the approximation level.
	// name_in is the name of the animation to cache, approximation_level is the approximation level. The higher the approximation level
	// the faster the cache generation but lower the quality. 1 means no approximation, with 10 being the maximum value allowed.
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void MakeBluePrintPointCache(FString name_in, int32 approximation_level);

	// Blueprint function to clear the point cache of a given animation
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void ClearBluePrintPointCache(FString name_in, int32 approximation_level);

	// Blueprint function that returns the transform given a bone name, position_slide_factor
	// determines how far left or right the transform is placed. The default value of 0 places it
	// in the center of the bone, positve values places it to the right, negative to the left
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	FTransform GetBluePrintBoneXform(FString name_in, bool world_transform, float position_slide_factor);

	// Blueprint function that decides whether the animation will loop or not
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintAnimationLoop(bool flag_in);

	// Blueprint function that decides whether to play the animation or not
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintAnimationPlay(bool flag_in);

	// Blueprint function that plays the animation from the start
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintAnimationPlayFromStart();

	// Blueprint function that resets the animation to the start time
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintAnimationResetToStart();

	// Blueprint function that returns the current animation frame
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	float GetBluePrintAnimationFrame();

	// Blueprint function that sets the alpha(opacity value) of a region
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintRegionAlpha(FString region_name_in, uint8 alpha_in);

	// Blueprint function that sets up a custom z order for the various regions
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintRegionCustomOrder(TArray<FString> order_in);

	// Blueprint function that clears the custom z order for the various regions
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void ClearBluePrintRegionCustomOrder();

	// Blueprint function that turns on/turns off internal updates of this object
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetIsDisabled(bool flag_in);

	// Blueprint function that sets the active collection clip
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBluePrintActiveCollectionClip(FString name_in);

	CreatureCore& GetCore();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void OnRegister() override;

	virtual void InitializeComponent() override;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

protected:
	CreatureCore creature_core;
	FString active_collection_clip_name;
	FCreatureMeshCollectionClip * active_collection_clip;
	bool active_collection_loop;

	void InitStandardValues();

	void UpdateCoreValues();

	void PrepareRenderData();

	void RunTick(float DeltaTime);

	void RunCollectionTick(float DeltaTime);

	void StandardInit();

	void CollectionInit();

	void SwitchToCollectionClip(FCreatureMeshCollectionClip * clip_in);

	void SetActiveCollectionAnimation(FCreatureMeshCollectionClip * clip_in);

	FCreatureMeshCollection *
	GetCollectionDataFromClip(FCreatureMeshCollectionClip * clip_in);

	int GetCollectionDataIndexFromClip(FCreatureMeshCollectionClip * clip_in);

	void DoCreatureMeshUpdate(int render_packet_idx = -1);
};
