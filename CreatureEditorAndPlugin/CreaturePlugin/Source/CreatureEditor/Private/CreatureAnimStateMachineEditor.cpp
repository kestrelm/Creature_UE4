#include "CreatureEditorPCH.h"
#include "CreatureAnimStateMachineEditor.h"
#include "CreatureAnimStateMachine.h"
#include "CreatureStateMachineGraph.h"
#include "SDockTab.h"
#include "GraphEditorActions.h"
#include "GraphEditor.h"
#include "IToolkitHost.h"
#include "GenericCommands.h"
#include "CreatureAnimTransitionNode.h"
#include "STextComboBox.h"
#include "CreatureAnimStateMachineInstance.h"

#define LOCTEXT_NAMESPACE "AnimAssetEditors"

void FCreatureAnimStateMachineEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager)
{
	GraphEditorCommands = MakeShareable(new FUICommandList());
	FAssetEditorToolkit::RegisterTabSpawners(TabManager);

	
	//蓝图编辑器命令绑定
	GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
		FExecuteAction::CreateSP(this,&FCreatureAnimStateMachineEditor::OnDeleteNode)
		);

	/*NodeAction.OnSelectionChanged.BindLambda(
		[&](const FGraphPanelSelectionSet& selection){
				for (auto obj : selection)
				{
					if (UCreatureAnimStateNode* Node =Cast<UCreatureAnimStateNode>(obj))
					{
						EditGraph->NotifyGraphChanged();
					}
				}
			}
	);*/
	EditGraph =SNew(SGraphEditor)
		.GraphToEdit(EditingStateMachine->StateMachineGraph)
		.IsEditable(true)
		.AdditionalCommands(GraphEditorCommands)
		.GraphEvents(NodeAction)
		;
	//将细节面板放入左侧面板
	TabManager->RegisterTabSpawner(FName(TEXT("Details")), FOnSpawnTab::CreateLambda(
		[&](const FSpawnTabArgs& Args){
		TSharedPtr<FCreatureAnimStateMachineEditor> StateMechineEditorPtr = SharedThis(this);

			// Spawn the tab
			return SNew(SDockTab)
				.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
				.Label(LOCTEXT("DetailsTab_Title", "Details"))
				[
					SNew(SCreatureAnimStateMachinePropertiesTabBody,StateMechineEditorPtr)

				];
			}
		))
		.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

		TabManager->RegisterTabSpawner(FName(TEXT("BluePrint")), FOnSpawnTab::CreateLambda(
			//用于产生布局的Lambda表达式
			[&](const FSpawnTabArgs& Args){
			return SNew(SDockTab)
				.Icon(FEditorStyle::GetBrush("LevelEditor.Tabs.Details"))
				.Label(LOCTEXT("BlueprintTab_Title", "Blueprint"))
				[
					EditGraph.ToSharedRef()//Slate允许提供一个sharedRef，不需要原地构建控件
				];

			}
		))
		.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));


}
void FCreatureAnimStateMachineEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(TabManager);

}
FName FCreatureAnimStateMachineEditor::GetToolkitFName() const
{
	return FName("TileMapEditor");
}

FText FCreatureAnimStateMachineEditor::GetBaseToolkitName() const
{
	return LOCTEXT("TileMapEditorAppLabelBase", "Tile Map Editor");
}

FText FCreatureAnimStateMachineEditor::GetToolkitName() const
{
	return FText::FromString(EditingStateMachine->GetName());
}

FText FCreatureAnimStateMachineEditor::GetToolkitToolTipText() const
{
	return GetToolTipTextForObject(EditingStateMachine);
}

FString FCreatureAnimStateMachineEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("StateMachineEditor");
}

FString FCreatureAnimStateMachineEditor::GetDocumentationLink() const
{
	return TEXT("Engine/Paper2D/TileMapEditor");
}

void FCreatureAnimStateMachineEditor::OnToolkitHostingStarted(const TSharedRef< class IToolkit >& Toolkit)
{
	
}

void FCreatureAnimStateMachineEditor::OnToolkitHostingFinished(const TSharedRef< class IToolkit >& Toolkit)
{
	

}

FLinearColor FCreatureAnimStateMachineEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

void FCreatureAnimStateMachineEditor::InitAnimStateMachineEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UCreatureAnimStateMachine* StateMachine){
	
	EditingStateMachine = StateMachine;
	
	// 定义了编辑器面板的默认显示样式
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_StateMachineEditor_Layout")
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
				->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.1f)
					->SetHideTabWell(true)
					->AddTab(FName(TEXT("Details")), ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.1f)
					->SetHideTabWell(true)
					->AddTab(FName(TEXT("BluePrint")), ETabState::OpenedTab)
				)


			)
		
			

		);

	GenerateDebugObjectNames(false);

	DebugObjectsComboBox = SNew(STextComboBox)
		.ButtonStyle(FEditorStyle::Get(), "FlatButton.Light")
		.OptionsSource(&DebugObjectNames)
		.InitiallySelectedItem(GetDebugObjectName())
		.OnComboBoxOpening(this, &FCreatureAnimStateMachineEditor::GenerateDebugObjectNames, true)
		.OnSelectionChanged(this, &FCreatureAnimStateMachineEditor::DebugObjectSelectionChanged)
		.AddMetaData<FTagMetaData>(TEXT("SelectDebugObjectCombo"));

	TSharedRef<SWidget> DebugObjectSelectionWidget =
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			DebugObjectsComboBox.ToSharedRef()
		];
	AddToolbarWidget(DebugObjectSelectionWidget);

	// Initialize the asset editor and spawn the layout above
	InitAssetEditor(Mode, InitToolkitHost, FName(TEXT("StateMachineEditorApp")), StandaloneDefaultLayout, /*bCreateDefaultStandaloneMenu=*/ true, /*bCreateDefaultToolbar=*/ true, StateMachine);
}

TSharedPtr<FString> FCreatureAnimStateMachineEditor::GetDebugObjectName() const
{
	check(EditingStateMachine);
	check(DebugObjects.Num() == DebugObjectNames.Num());
	if (EditingStateMachine->InstanceBeingDebugged)
	{
		for (int32 ObjectIndex = 0; ObjectIndex < DebugObjects.Num(); ++ObjectIndex)
		{
			if (DebugObjects[ObjectIndex].IsValid() && (DebugObjects[ObjectIndex].Get() == EditingStateMachine->InstanceBeingDebugged))
			{
				return DebugObjectNames[ObjectIndex];
			}
		}
	}

	return DebugObjectNames[0];
}

void FCreatureAnimStateMachineEditor::GenerateDebugObjectNames(bool bRestoreSelection)
{
	TSharedPtr<FString> OldSelection;

	// Store off the old selection
	if (bRestoreSelection && DebugObjectsComboBox.IsValid())
	{
		OldSelection = DebugObjectsComboBox->GetSelectedItem();
	}

	DebugObjectNames.Reset();
	DebugObjects.Reset();

	DebugObjectNames.Add(MakeShareable(new FString(TEXT("No debug object selected"))));
	DebugObjects.Add(nullptr);

	for (TObjectIterator<UObject> It; It; ++It)
	{
		UObject *obj = *It;
		if (UCreatureAnimStateMachineInstance* instance = Cast<UCreatureAnimStateMachineInstance>(obj))
		{
			if (instance->GetTargetStateMachine() == EditingStateMachine)
			{
				AddDebugObject(instance);
			}
		}
	}

	// Attempt to restore the old selection
	if (bRestoreSelection && DebugObjectsComboBox.IsValid())
	{
		bool bMatchFound = false;
		for (int32 ObjectIndex = 0; ObjectIndex < DebugObjectNames.Num(); ++ObjectIndex)
		{
			if (*DebugObjectNames[ObjectIndex] == *OldSelection)
			{
				DebugObjectsComboBox->SetSelectedItem(DebugObjectNames[ObjectIndex]);
				bMatchFound = true;
				break;
			}
		}

		// No match found, use the default option
		if (!bMatchFound)
		{
			DebugObjectsComboBox->SetSelectedItem(DebugObjectNames[0]);
		}
	}

	// Finally ensure we have a valid selection
	if (DebugObjectsComboBox.IsValid())
	{
		TSharedPtr<FString> CurrentSelection = DebugObjectsComboBox->GetSelectedItem();
		if (DebugObjectNames.Find(CurrentSelection) == INDEX_NONE)
		{
			if (DebugObjectNames.Num() > 0)
			{
				DebugObjectsComboBox->SetSelectedItem(DebugObjectNames[0]);
			}
			else
			{
				DebugObjectsComboBox->ClearSelection();
			}
		}

		DebugObjectsComboBox->RefreshOptions();
	}
}

void FCreatureAnimStateMachineEditor::AddDebugObject(class UCreatureAnimStateMachineInstance* TestObject)
{
	FString Label;
	if (AActor* ParentActor = TestObject->GetTypedOuter<AActor>())
	{
		// This gives the most precision, but is pretty long for the combo box
		//const FString RelativePath = TestObject->GetPathName(/*StopOuter=*/ ParentActor);
		const FString RelativePath = TestObject->GetName();
		Label = FString::Printf(TEXT("%s in %s"), *RelativePath, *ParentActor->GetActorLabel());
	}
	else
	{
		Label = TestObject->GetName();
	}

	DebugObjects.Add(TestObject);
	DebugObjectNames.Add(MakeShareable(new FString(Label)));
}

void FCreatureAnimStateMachineEditor::DebugObjectSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	check(DebugObjects.Num() == DebugObjectNames.Num());
	for (int32 ObjectIndex = 0; ObjectIndex < DebugObjectNames.Num(); ++ObjectIndex)
	{
		if (*DebugObjectNames[ObjectIndex] == *NewSelection)
		{
			UCreatureAnimStateMachineInstance* DebugObj = DebugObjects[ObjectIndex].IsValid() ? DebugObjects[ObjectIndex].Get() : nullptr;
			
			EditingStateMachine->InstanceBeingDebugged = DebugObj;
		}
	}
}

UCreatureAnimStateMachine* FCreatureAnimStateMachineEditor::GetEditingStateMachine(){
	return EditingStateMachine;

}

void FCreatureAnimStateMachineEditor::OnDeleteNode()
{
	if (EditGraph.IsValid())
	{
		for (auto node:EditGraph->GetSelectedNodes())
		{
			if (UEdGraphNode* GraphNode=Cast<UEdGraphNode>(node))
			{
				GraphNode->DestroyNode();
			}
		}
	}
}

UEdGraphNode* FCreatureAnimStateMachineEditor::GetSelectNode()
{
	if (EditGraph.IsValid() && EditGraph->GetSelectedNodes().Num()!=0)
	{
		auto Node = EditGraph->GetSelectedNodes().Array()[0];
		if (Node!=nullptr)
		{
			return Cast<UEdGraphNode>(Node);
		}
		
		
	}
	return nullptr;

}

void FCreatureAnimStateMachineEditor::SaveAsset_Execute()
{
	Cast<UCreatureStateMachineGraph>(EditingStateMachine->StateMachineGraph)->CompileNodes();
	FAssetEditorToolkit::SaveAsset_Execute();
}

#undef LOCTEXT_NAMESPACE  
