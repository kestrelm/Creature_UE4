#pragma once
 
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Components/Widget.h"
#include "CreatureAnimationAsset.h"
#include "CreatureMetaAsset.h"
#include "CreatureCore.h"

#include "CreatureWidget.generated.h"

class SCreatureWidget;
 
UCLASS()
class UCreatureWidget : public UWidget
{
	GENERATED_UCLASS_BODY()
 
protected:
	TSharedPtr<SCreatureWidget> creature_draw;
	CreatureCore creature_core;

public:
   
	UPROPERTY(EditDefaultsOnly, Category=Events)
	FOnPointerEvent OnMouseButtonDownEvent;
  
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Creature")
	FSlateBrush creature_brush;

	/** Points to a Creature Animation Asset containing the JSON filename of the character. Use this instead of creature_filename if you want to use an asset based system. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Creature")
	UCreatureAnimationAsset * creature_animation_asset;

	/** Points to a Creature Meta Asset containing the JSON of the mdata file exported out from Creature. This file contains extra data like animation data for region ordering for example. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Creature")
	UCreatureMetaAsset * creature_meta_asset;

	/** Playback speed of the animation, 2.0 is the default */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	float animation_speed = 2.0f;

	/** Activates/Deactivates anchor points in the character if it was setup in the Creature Animation Editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	bool use_anchor_points;

	/** Scaling factor on character mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Creature")
	FVector2D creature_scale = FVector2D(1, 1);

	// Ticks the character by a delta time. Call this function at every Tick Event to animate
	// the character from your game's Tick event
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void CreatureTick(float DeltaTime);

	// Setting the active animation name
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetActiveAnimation(FName name_in);

	// Setting the blended active animation name
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void SetBlendActiveAnimation(FName name_in, float factor);

	// Enable Skin Swap for a particular Skin Swap Name
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void EnableSkinSwap(FString swap_name);

	// Turns off Skin Swapping
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void DisableSkinSwap();

	// Adds a new Skin Swap
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void AddSkinSwap(FString new_swap_name, TArray<FString> new_swap);

	// Enables Region Color Animation
	UFUNCTION(BlueprintCallable, Category = "Components|Creature")
	void EnableRegionColors();

	// UWidget interface
	virtual void SynchronizeProperties() override;
	// End of UWidget interface
 
	// UVisual interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	// End of UVisual interface
 
#if WITH_EDITOR
	// UWidget interface
	virtual const FText GetPaletteCategory() override;
	// End UWidget interface
#endif

	//virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
 
protected:

	virtual TSharedRef<SWidget> RebuildWidget() override;
  
	FReply HandleMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent);

	void InitCreatureCore();
};
