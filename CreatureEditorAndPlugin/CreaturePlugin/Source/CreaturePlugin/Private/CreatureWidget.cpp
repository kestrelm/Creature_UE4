#include "CreatureWidget.h"
#include "CreaturePluginPCH.h"
#include "SCreatureWidget.h"

//LOCTEXT
#define LOCTEXT_NAMESPACE "UMG"

// UCreatureWidget
UCreatureWidget::UCreatureWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//bCanEverTick = true;
}

TSharedRef<SWidget> UCreatureWidget::RebuildWidget()
{
	creature_draw = SNew(SCreatureWidget);
	return creature_draw.ToSharedRef();
}

void UCreatureWidget::InitCreatureCore()
{
	creature_core.ClearMemory();
	creature_core = CreatureCore();

	if (creature_animation_asset
		&& creature_core.creature_asset_filename != creature_animation_asset->GetCreatureFilename())
	{
		creature_core.pJsonData = &creature_animation_asset->GetJsonString();
		creature_core.creature_asset_filename = creature_animation_asset->GetCreatureFilename();

		creature_animation_asset->LoadPointCacheForAllClips(&creature_core);

		creature_core.InitCreatureRender();
		creature_core.InitValues();
		creature_core.SetUseAnchorPoints(use_anchor_points);

		if (creature_meta_asset)
		{
			creature_meta_asset->BuildMetaData();
			creature_core.meta_data = creature_meta_asset->GetMetaData();
		}
	}
}

void UCreatureWidget::SynchronizeProperties()
{
    // Property update(s)
	Super::SynchronizeProperties();
	
	InitCreatureCore();
	if (creature_draw.IsValid())
	{
		creature_draw->SetBrush(&creature_brush);

		creature_draw->SetCreatureCore(&creature_core);
		creature_core.RunTick(0);

		auto cur_world_type = EWorldType::Type::Editor;
		if (GetWorld())
		{
			cur_world_type = GetWorld()->WorldType;
		}
		
		creature_draw->SetWorldType(cur_world_type);
		creature_draw->SetMeshScale(creature_scale);
	}
}

void UCreatureWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
 
	creature_draw.Reset();
}

FReply UCreatureWidget::HandleMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
	if ( OnMouseButtonDownEvent.IsBound() )
	{
		return OnMouseButtonDownEvent.Execute(Geometry, MouseEvent).NativeReply;
	}
 
	return FReply::Unhandled();
}

void UCreatureWidget::CreatureTick(float DeltaTime)
{
	creature_core.RunTick(DeltaTime * animation_speed);
}

void UCreatureWidget::SetActiveAnimation(FName name_in)
{
	creature_core.SetBluePrintActiveAnimation(name_in);
}

void UCreatureWidget::SetBlendActiveAnimation(FName name_in, float factor)
{
	creature_core.SetBluePrintBlendActiveAnimation(name_in, factor);
}

void UCreatureWidget::EnableSkinSwap(FString swap_name)
{
	creature_core.enableSkinSwap(swap_name, true);
}

void UCreatureWidget::DisableSkinSwap()
{
	creature_core.enableSkinSwap("", true);
}

void UCreatureWidget::AddSkinSwap(FString new_swap_name, TArray<FString> new_swap)
{
	if (creature_core.meta_data)
	{
		TSet<FString> new_set;
		for (auto& cur_str : new_swap)
		{
			new_set.Add(cur_str);
		}

		creature_core.meta_data->addSkinSwap(new_swap_name, new_set);
	}
}

#if WITH_EDITOR

const FText UCreatureWidget::GetPaletteCategory()
{
	return LOCTEXT("Common", "Common");
}
 
#endif

#undef LOCTEXT_NAMESPACE