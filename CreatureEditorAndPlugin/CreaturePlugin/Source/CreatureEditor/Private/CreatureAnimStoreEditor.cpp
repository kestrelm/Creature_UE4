#include "CreatureAnimStoreEditor.h"
#include "CreatureAnimStateMachine.h"
#include "CreatureStateMachineGraph.h"
#include "Widgets/Docking/SDockTab.h"
#include "GraphEditorActions.h"
#include "GraphEditor.h"
#include "Toolkits/IToolkitHost.h"
#include "Framework/Commands/GenericCommands.h"
#include "CreatureAnimTransitionNode.h"
#include "SEditorViewport.h"
#include "SCreatureAnimClipStoreEditorViewport.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#define LOCTEXT_NAMESPACE "AssetTypeEditors"

#if __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
#endif

void FCreatureAnimStoreEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManagerIn)
{
	FAssetEditorToolkit::RegisterTabSpawners(TabManagerIn);
	TSharedPtr<FCreatureAnimStoreEditor> StoreEditorPtr = SharedThis(this);// Spawn the tab
	StorePanel = SNew(SStoreDetailPanel, StoreEditorPtr);
	//注册细节面板
	TabManagerIn->RegisterTabSpawner(FName(TEXT("Details")), FOnSpawnTab::CreateLambda(
		[&](const FSpawnTabArgs& Args){		
		return SNew(SDockTab)
			.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
			.Label(LOCTEXT("DetailsTab_Title", "Details"))
			[
				StorePanel.ToSharedRef()

			];}
		))
		.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
		
				
				ClipViewport = SNew(SCreatureAnimClipStoreEditorViewport)
					.ObjectToEdit(EditClipsStore);
//注册Viewport
TabManagerIn->RegisterTabSpawner(FName(TEXT("Preview")), FOnSpawnTab::CreateLambda(
		[&](const FSpawnTabArgs& Args){
		TSharedPtr<FCreatureAnimStoreEditor> StoreEditorPtr = SharedThis(this);// Spawn the tab
		return SNew(SDockTab)
			.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
			.Label(LOCTEXT("Preview_Title", "Preview"))
			[
				ClipViewport.ToSharedRef()

			];}
		))
		.SetDisplayName(LOCTEXT("ViewportLabel", "Preview"))
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
}
void FCreatureAnimStoreEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManagerIn)
{
	FAssetEditorToolkit::UnregisterTabSpawners(TabManagerIn);

}
FName FCreatureAnimStoreEditor::GetToolkitFName() const
{
	return FName("AnimStoreEditor");
}

FText FCreatureAnimStoreEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AnimStoreEditorAppLabelBase", "Anim Store Editor");
}

FText FCreatureAnimStoreEditor::GetToolkitName() const
{
	return FText::FromString(EditClipsStore->GetName());
}

FText FCreatureAnimStoreEditor::GetToolkitToolTipText() const
{
	return GetToolTipTextForObject(EditClipsStore);
}

FString FCreatureAnimStoreEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("Anim Store Editor");
}

void FCreatureAnimStoreEditor::OnToolkitHostingStarted(const TSharedRef< class IToolkit >& Toolkit)
{
	
}

void FCreatureAnimStoreEditor::OnToolkitHostingFinished(const TSharedRef< class IToolkit >& Toolkit)
{
	

}

FLinearColor FCreatureAnimStoreEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}
void FCreatureAnimStoreEditor::InitAnimStoreEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UCreatureAnimationClipsStore* Store){
	EditClipsStore = Store;

	// 定义了编辑器面板的默认显示样式
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_AnimClipStoreEditor_Layout")
		->AddArea
		(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Vertical)
		->Split
		(
		FTabManager::NewStack()
		->SetSizeCoefficient(0.1f)
		->SetHideTabWell(true)
		->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
		)
		->Split
		(
		FTabManager::NewSplitter()
		->SetOrientation(Orient_Horizontal)
		->SetSizeCoefficient(0.2f)
		->Split
		(
			
			FTabManager::NewStack()
			->SetHideTabWell(true)
			->AddTab(FName(TEXT("Details")), ETabState::OpenedTab)
			
		)
		->Split
		(
		FTabManager::NewStack()
		->SetSizeCoefficient(0.8f)
		->SetHideTabWell(true)
		->AddTab(FName(TEXT("Preview")), ETabState::OpenedTab)
		)


		)



		);

	// Initialize the asset editor and spawn the layout above
	InitAssetEditor(Mode, InitToolkitHost, FName(TEXT("StateMachineEditorApp")), StandaloneDefaultLayout, /*bCreateDefaultStandaloneMenu=*/ true, /*bCreateDefaultToolbar=*/ true, Store);
}

void FCreatureAnimStoreEditor::ChangePreviewAnimation(FString AnimationName)
{
	ClipViewport->ChangePreviewAnimation(AnimationName);
}

void FCreatureAnimStoreEditor::SaveAsset_Execute()
{
	FAssetEditorToolkit::SaveAsset_Execute();
	StorePanel->ConstructPreviewAnimationList();
	//重新构建Mesh
	ClipViewport->ReConstructMesh();
}

TSharedRef<SWidget> SStoreDetailPanel::PopulateSlot(TSharedRef<SWidget> PropertyEditorWidget)
{
	{

		return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
				.VAlign(VAlign_Top)
				.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
					.AutoWidth()
				[
					SNew(STextBlock)
						.Text(LOCTEXT("PreviewAnimation","PreviewAnimation"))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(STextComboBox)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.OptionsSource(&PreviewAnimationNameList)
						.OnSelectionChanged_Lambda(
						[&](TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
							{
								if (NewValue.IsValid())
								{
									EditorPtr.Pin()->ChangePreviewAnimation(*NewValue);
								}
							
							}
						
						
						)
				]

			]
		+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				PropertyEditorWidget
			]

		;
	}
}

void SStoreDetailPanel::ConstructPreviewAnimationList()
{
	if (EditorPtr.Pin()->GetEditingClipsStore()->ClipList.Num() != 0)
	{
		PreviewAnimationNameList.Empty();
		for (auto clip : EditorPtr.Pin()->GetEditingClipsStore()->ClipList)
		{
			TSharedPtr<FString> AnimName = MakeShareable(new FString(clip.ClipName.ToString()));
			PreviewAnimationNameList.AddUnique(AnimName);
		}
	}
}


#undef LOCTEXT_NAMESPACE  

#if __APPLE__
#pragma clang diagnostic pop
#endif


