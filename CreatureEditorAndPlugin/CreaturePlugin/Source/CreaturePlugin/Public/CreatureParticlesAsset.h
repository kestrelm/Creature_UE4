#pragma  once
#include "CoreMinimal.h"
#include "mpLib.h"
#include "Runtime/Core/Public/Templates/UniquePtr.h"
#include "CreatureParticlesAsset.generated.h"

UCLASS()
class CREATUREPLUGIN_API UCreatureParticlesAsset :public UObject
{
	GENERATED_BODY()
public:

    UPROPERTY()
    TArray<uint8> m_ParticlesBytes;

	// Names of all animation clips for this particles asset
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Creature")
	TArray<FString> animation_clipnames;

protected:

	class particlesOffsetsData
	{
	public:
		particlesOffsetsData(int offset_in, int num_particles_in)
			: offset(offset_in), num_particles(num_particles_in)
		{}

		particlesOffsetsData()
		{}

		int offset = -1;
		int num_particles = 0;
	};

	class clipOffsetsData
	{
	public:
		int32 base_idx = -1;
		int32 num_frames;
		FString name;

		// <Frame, <layer idx, particlesOffsetsData>>
		TMap<int, TMap<int, particlesOffsetsData>> particles_lookup;
	};

	TUniquePtr<mpMiniLib::msg_mini> m_PackData;
	TMap<FString, clipOffsetsData> m_ClipDataOffsets;
	int32 m_maxParticlesNum = 0;
	int32 m_maxIndicesNum = 0;

protected:

	void computeDataInfo(const std::vector<mpMiniLib::msg_mini_generic_data>& pack_objs);

	int32 getVersion(const std::vector<mpMiniLib::msg_mini_generic_data>& pack_objs) const;

	int32 getGapStep(const std::vector<mpMiniLib::msg_mini_generic_data>& pack_objs) const;

	int32 getAnimClipsNum(const std::vector<mpMiniLib::msg_mini_generic_data>& pack_objs) const;

	TArray<FString> getAnimClipNames(const std::vector<mpMiniLib::msg_mini_generic_data>& pack_objs) const;

public:

	void setData(const TArray<uint8>& data_in);

	mpMiniLib::msg_mini * getPackData();
};