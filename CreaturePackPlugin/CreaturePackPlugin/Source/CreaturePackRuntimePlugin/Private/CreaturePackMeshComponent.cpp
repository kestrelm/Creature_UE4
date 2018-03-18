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

#include "CreaturePackMeshComponent.h"

static TMap<FString, CreaturePackLoader *> globalCreaturePackLoaders;
static std::mutex loadLock;

// UCreaturePackMeshComponent
UCreaturePackMeshComponent::UCreaturePackMeshComponent(const FObjectInitializer& ObjectInitializer)
	: UCustomPackProceduralMeshComponent(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bWantsInitializeComponent = true;
	packData = nullptr;
	animation_speed = 60.0f;
	animation_frame = 0.0f;
	creature_debug_draw = false;
	attach_vertex_id = -1;
	region_offset_z = 0.01f;
	updateLock = TSharedPtr<FCriticalSection, ESPMode::ThreadSafe>(new FCriticalSection());
}

void UCreaturePackMeshComponent::SetActiveAnimation(FString name_in)
{
	if (!isPlayerValid())
	{
		return;
	}

	playerObj->setActiveAnimation(ConvertToString(name_in));
}

void UCreaturePackMeshComponent::SetBlendActiveAnimation(FString name_in, float factor)
{
	if (!isPlayerValid())
	{
		return;
	}

	playerObj->blendToAnimation(ConvertToString(name_in), factor);
}

void 
UCreaturePackMeshComponent::SetShouldLoop(bool flagIn)
{
	if (!isPlayerValid())
	{
		return;
	}

	playerObj->isLooping = flagIn;
}

FVector 
UCreaturePackMeshComponent::GetAttachmentPosition(int32 vertex_id) const
{
	if (!isPlayerValid())
	{
		return FVector(0, 0, 0);
	}

	if (vertex_id < 0)
	{
		vertex_id = attach_vertex_id;
	}

	if ((vertex_id < 0)
		|| (vertex_id >= (int32)playerObj->getRenderPointsLength()))
	{
		return FVector(0, 0, 0);
	}

	auto pts_array = playerObj->render_points.get();
	auto cur_point = FVector(pts_array[vertex_id * 3],
		pts_array[vertex_id * 3 + 2],
		pts_array[vertex_id * 3 + 1]
		);

	FVector ret_point = GetComponentToWorld().TransformPosition(cur_point);

	return ret_point;
}

void UCreaturePackMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if ((GetOwner() && GetOwner()->bHidden) || bHiddenInGame)
	{
		return;
	}

	runTick(DeltaTime);
}

void UCreaturePackMeshComponent::OnRegister()
{
	Super::OnRegister();
	initCreatureRender();
	runTick(0);
}

void UCreaturePackMeshComponent::InitializeComponent()
{
	Super::InitializeComponent();
	initCreatureRender();
	runTick(0);
}

FPrimitiveSceneProxy * UCreaturePackMeshComponent::CreateSceneProxy()
{
	return UCustomPackProceduralMeshComponent::CreateSceneProxy();
}

bool
UCreaturePackMeshComponent::loadPackData(const FString& filenameIn, 
	const TArray<uint8>& fileData,
	bool overwrite)
{
	if (globalCreaturePackLoaders.Contains(filenameIn) && (overwrite == false))
	{
		return false;
	}

	std::vector<uint8_t> raw_data(fileData.Num());
	for (size_t i = 0; i < (size_t)raw_data.size(); i++)
	{
		raw_data[i] = fileData[i];
	}

	globalCreaturePackLoaders.Add(filenameIn, new CreaturePackLoader(raw_data));

	return true;
}

CreaturePackLoader *
UCreaturePackMeshComponent::getPackData(const FString& filenameIn)
{
	if (globalCreaturePackLoaders.Contains(filenameIn))
	{
		return globalCreaturePackLoaders[filenameIn];
	}

	return nullptr;
}

bool 
UCreaturePackMeshComponent::isPlayerValid() const
{
	return (packData != nullptr) && (playerObj != nullptr);
}

bool UCreaturePackMeshComponent::initCreatureRender()
{
	std::lock_guard<std::mutex> scope_lock(loadLock);

	if (creature_animation_asset == nullptr)
	{
		return false;
	}

	auto realFilename = creature_animation_asset->GetCreatureFilename();
	auto binaryBytes = creature_animation_asset->GetFileData();
	loadPackData(realFilename, binaryBytes);
	packData = getPackData(realFilename);

	if (packData == nullptr)
	{
		return false;
	}

	playerObj = std::shared_ptr<CreaturePackPlayer>(new CreaturePackPlayer(*packData));
	prepareRenderData();

	return true;
}

void 
UCreaturePackMeshComponent::prepareRenderData()
{
	RecreateRenderProxy(true);
	SetProceduralMeshTriData(GetProcMeshData());
}

FProceduralPackMeshTriData
UCreaturePackMeshComponent::GetProcMeshData()
{
	if (!isPlayerValid())
	{
		FProceduralPackMeshTriData ret_data(nullptr,
			nullptr, nullptr,
			0, 0,
			&regionAlphas,
			updateLock);

		return ret_data;
	}

	int32 num_points = playerObj->getRenderPointsLength() / 3;
	int32 num_indices = playerObj->data.getNumIndices();
	uint32 * cur_idx = playerObj->data.indices.get();
	float * cur_pts = playerObj->render_points.get();
	float * cur_uvs = playerObj->render_uvs.get();

	if (regionAlphas.Num() != num_points)
	{
		regionAlphas.SetNum(num_points);
		for (size_t i = 0; i < (size_t)regionAlphas.Num(); i++)
		{
			regionAlphas[i] = 255;
		}
	}

	FProceduralPackMeshTriData ret_data(
		cur_idx,
		cur_pts, cur_uvs,
		num_points, num_indices,
		&regionAlphas,
		updateLock);

	return ret_data;
}

void 
UCreaturePackMeshComponent::runTick(float deltaTime)
{
	FScopeLock lock(&tickLock);

	if (!isPlayerValid())
	{
		return;
	}

	playerObj->stepTime(deltaTime * animation_speed);
	playerObj->syncRenderData();
	runRegionOffsetZs();

	animation_frame = playerObj->getRunTime();

	doCreatureMeshUpdate();
}

void 
UCreaturePackMeshComponent::doCreatureMeshUpdate(int render_packet_idx)
{
	FCProceduralPackMeshSceneProxy *localRenderProxy = GetLocalRenderProxy();
	SetBoundsScale(1.0f);
	SetBoundsOffset(FVector(0, 0, 0));
	ForceAnUpdate(render_packet_idx);

	if (creature_debug_draw) {
		FSphere tmpDebugSphere = GetDebugBoundsSphere();
		DrawDebugSphere(
			GetWorld(),
			tmpDebugSphere.Center,
			tmpDebugSphere.W,
			32,
			FColor(255, 0, 0)
			);

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Sphere pos is: (%f, %f, %f)"), tmpDebugSphere.Center.X, tmpDebugSphere.Center.Y, tmpDebugSphere.Center.Z));
		FTransform wTransform = GetComponentToWorld();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Walk pos is: (%f, %f, %f)"), wTransform.GetLocation().X,
			wTransform.GetLocation().Y,
			wTransform.GetLocation().Z));
	}
}

void 
UCreaturePackMeshComponent::runRegionOffsetZs()
{
	if (!packData)
	{
		return;
	}

	auto& mesh_regions_list = packData->meshRegionsList;
	float set_region_z = 0.0f;
	for (auto cur_region : mesh_regions_list)
	{
		auto start_idx = cur_region.first;
		auto end_idx = cur_region.second;

		float * cur_pts = playerObj->render_points.get();
		for (auto i = start_idx; i <= end_idx; i++)
		{
			cur_pts[i * 3 + 2] = set_region_z;
		}

		set_region_z += region_offset_z;
	}
}




