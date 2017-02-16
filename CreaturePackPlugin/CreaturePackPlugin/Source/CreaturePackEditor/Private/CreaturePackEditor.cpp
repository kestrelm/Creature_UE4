#include "CreaturePackEditorPCH.h"
#include "CreaturePackEditor.h"
#include "CreaturePackAnimationAssetTypeActions.h"
#include "CreaturePackMeshComponent.h"
#include "PropertyEditorModule.h"
#define LOCTEXT_NAMESPACE "CreaturePackEditor"

class UCreaturePackMeshVisualizer : public FComponentVisualizer
{
public:
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override
	{
		const UCreaturePackMeshComponent * creature_mesh = dynamic_cast<const UCreaturePackMeshComponent *>(Component);
		if (!creature_mesh)
		{
			return;
		}

		if (creature_mesh->attach_vertex_id > 0)
		{
			FVector point_pos = creature_mesh->GetAttachmentPosition(-1);
			DrawWireSphere(PDI, FTransform(point_pos), FLinearColor(1.0f, 1.0f, 0), 6.0f, 20, SDPG_World);
		}
	}
};

void CreaturePackEditor::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	//Custom detail views

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	EAssetTypeCategories::Type CreaturePackAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("CreatureAssetCategory")), LOCTEXT("CreatureAssetCategory", "Creature"));

	AssetTools.RegisterAssetTypeActions(MakeShareable(new FCreaturePackAnimationAssetTypeActions(CreaturePackAssetCategoryBit)));

	if (GUnrealEd)
	{
		GUnrealEd->RegisterComponentVisualizer(UCreaturePackMeshComponent::StaticClass()->GetFName(),
			MakeShareable(new UCreaturePackMeshVisualizer()));
	}
}

void CreaturePackEditor::ShutdownModule()
{
}

IMPLEMENT_MODULE(CreaturePackEditor, CreaturePackEditor)

#undef LOCTEXT_NAMESPACE
