
#include "Toolkits/AssetEditorToolkit.h"
#include "Editor/KismetWidgets/Public/SSingleObjectDetailsPanel.h"
#include "CreatureAnimationClipsStore.h"
#include "GraphEditor.h"
#include "Widgets/Input/STextComboBox.h"
//这个类负责定义编辑器的外观
#pragma once
class SCreatureAnimClipStoreEditorViewport;
class SStoreDetailPanel;
class FCreatureAnimStoreEditor :public FAssetEditorToolkit{
public:
	//IToolKitInterface
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	//EndIToolKitInterface

	// FAssetEditorToolkit
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual void OnToolkitHostingStarted(const TSharedRef< class IToolkit >& Toolkit) override;
	virtual void OnToolkitHostingFinished(const TSharedRef< class IToolkit >& Toolkit) override;
	// End of FAssetEditorToolkit
	void InitAnimStoreEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class UCreatureAnimationClipsStore* Store);
	
	
	TSharedPtr<FUICommandList> GraphEditorCommands;

	UCreatureAnimationClipsStore* GetEditingClipsStore(){
		return EditClipsStore;
	}

	/** Called when "Save" is clicked for this asset */
	virtual void SaveAsset_Execute() override;

	void ChangePreviewAnimation(FString AnimationName);
private:
	UCreatureAnimationClipsStore* EditClipsStore;

	 TSharedPtr<SCreatureAnimClipStoreEditorViewport> ClipViewport;
	 TSharedPtr<SStoreDetailPanel> StorePanel;
};
class SStoreDetailPanel : public SSingleObjectDetailsPanel
{
public:
	SLATE_BEGIN_ARGS(SStoreDetailPanel) {}
	SLATE_END_ARGS()

private:

	TWeakPtr<FCreatureAnimStoreEditor> EditorPtr;
	TArray<TSharedPtr<FString>> PreviewAnimationNameList;

public:
	void Construct(const FArguments& InArgs, TSharedPtr<FCreatureAnimStoreEditor> Editor)
	{
		EditorPtr = Editor;
		
		ConstructPreviewAnimationList();
		SSingleObjectDetailsPanel::Construct(SSingleObjectDetailsPanel::FArguments().HostCommandList(Editor->GetToolkitCommands()), /*bAutoObserve=*/ true, /*bAllowSearch=*/ true);
	}

	// SSingleObjectDetailsPanel interface
	virtual UObject* GetObjectToObserve() const override
	{
		return EditorPtr.Pin()->GetEditingClipsStore();
	}

	virtual TSharedRef<SWidget> PopulateSlot(TSharedRef<SWidget> PropertyEditorWidget) override;
	// End of SSingleObjectDetailsPanel interface
	void ConstructPreviewAnimationList();
};