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

#include <mutex>
#include <vector>
#include <map>
#include "CustomPackProceduralMeshComponent.h"
#include "CreaturePackModule.hpp"
#include "CreaturePackAnimationAsset.h"
#include "UnrealEd.h"
#include "CreaturePackMeshComponent.generated.h"

// UCreaturePackMeshComponent
UCLASS(editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup=Rendering)
class CREATUREPACKRUNTIMEPLUGIN_API UCreaturePackMeshComponent : public UCustomPackProceduralMeshComponent //, public IInterface_CollisionDataProvider
{
	GENERATED_UCLASS_BODY()

public:
	/** Points to a Creature Pack Animation Asset containing the binary data of the character.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|CreaturePack")
	UCreaturePackAnimationAsset * creature_animation_asset;

	/** Playback speed of the animation, 2.0 is the default */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|CreaturePack")
	float animation_speed;

	/** Current frame of the animation during playback */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|CreaturePack")
	float animation_frame;

	/** Displays the bouding box */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	bool creature_debug_draw;

	/** An attachment vertex, the id of the vertex you want to attach by index */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	int32 attach_vertex_id;

	/** The z value offset of each mesh region */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	float region_offset_z;

	// Blueprint version of setting the active animation name
	UFUNCTION(BlueprintCallable, Category = "Components|CreaturePack")
	void SetActiveAnimation(FString name_in);

	// Blueprint version of setting the blended active animation name
	UFUNCTION(BlueprintCallable, Category = "Components|CreaturePack")
	void SetBlendActiveAnimation(FString name_in, float factor);

	// Blueprint function to set whether the animation loops or not
	UFUNCTION(BlueprintCallable, Category = "Components|CreaturePack")
	void SetShouldLoop(bool flagIn);

	// Blueprint function that returns the world space position of a vertex by id. A value of -1 will result in the use of
	// the attach_vertex_id property value.
	UFUNCTION(BlueprintCallable, Category = "Components|CreaturePack")
	FVector GetAttachmentPosition(int32 vertex_id = -1) const;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void OnRegister() override;

	virtual void InitializeComponent() override;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;


	static bool loadPackData(const FString& filenameIn, 
		const TArray<uint8>& fileData, 
		bool overwrite = false);

	static CreaturePackLoader * getPackData(const FString& filenameIn);

	bool isPlayerValid() const;

protected:

	static std::string ConvertToString(const FString &str)
	{
		std::string t = TCHAR_TO_UTF8(*str);
		return t;
	}

	bool initCreatureRender();

	void prepareRenderData();

	void runTick(float deltaTime);

	FProceduralPackMeshTriData GetProcMeshData();

	void doCreatureMeshUpdate(int render_packet_idx = -1);
	
	void runRegionOffsetZs();

	CreaturePackLoader * packData;
	std::mutex updateLock, tickLock;
	TArray<uint8> regionAlphas;
	std::shared_ptr<CreaturePackPlayer> playerObj;
};