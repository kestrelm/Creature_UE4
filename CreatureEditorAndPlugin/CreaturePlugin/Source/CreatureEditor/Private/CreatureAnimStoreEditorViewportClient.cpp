
#include "CreatureEditorPCH.h"
#include "Runtime/Engine/Public/Slate/SceneViewport.h"
#include "SEditorViewport.h"
#include "AssetEditorModeManager.h"
#include "CreatureAnimStoreEditorViewportClient.h"
#include "CreatureMeshComponent.h"
#include "CreatureAnimationClipsStore.h"
FCreatureAnimStoreEditorViewportClient::FCreatureAnimStoreEditorViewportClient(const TWeakPtr<class SEditorViewport>& InEditorViewportWidget /*= nullptr*/, UCreatureAnimationClipsStore* EditingAnimStore)
	:FEditorViewportClient(nullptr, &OwnerScene, InEditorViewportWidget)
	, EditingStore(EditingAnimStore)
{
	PreviewScene = &OwnerScene;
	((FAssetEditorModeManager*)ModeTools)->SetPreviewScene(PreviewScene);
	DrawHelper.bDrawGrid = true;
	DrawHelper.bDrawPivot = true;
	
	/*SetViewLocation(FVector(0, 30, 0));
	SetLookAtLocation(FVector(0, -30, 0));*/
	

	SetRealtime(true);
	//向PreviewScene添加Component
	EditingCreatureMesh = NewObject<UCreatureMeshComponent>();
	//设置CreatureMesh组件
	SetUpEditingCreatureMesh();
	PreviewScene->AddComponent(EditingCreatureMesh, FTransform::Identity);
	EditingCreatureMesh->SetRelativeRotation(FRotator(0, 90, 0));

	//设置Camera
	SetUpCamera();
	
	if (EditingStore->ClipList.Num()!=0)
	{
		//默认播放第一个Clip
		EditingCreatureMesh->SetBluePrintActiveCollectionClip(EditingStore->ClipList[0].ClipName);
	}

}

void FCreatureAnimStoreEditorViewportClient::Tick(float DeltaSeconds)
{
	
	FEditorViewportClient::Tick(DeltaSeconds);
	OwnerScene.GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
	//EditingCreatureMesh->UpdateBounds();
	//FBox Box = EditingCreatureMesh->Bounds.GetBox();
	//
	//FocusViewportOnBox(Box);
}

FLinearColor FCreatureAnimStoreEditorViewportClient::GetBackgroundColor() const
{
	return FLinearColor::Black;
}

void FCreatureAnimStoreEditorViewportClient::AddReferencedObjects(FReferenceCollector& Collector)
{
	
}

void FCreatureAnimStoreEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FEditorViewportClient::Draw(View, PDI);
}

void FCreatureAnimStoreEditorViewportClient::SetUpEditingCreatureMesh()
{
	if (EditingCreatureMesh==nullptr||EditingStore==nullptr)
	{
		return;
	}
	EditingCreatureMesh->ClipStore = EditingStore;
	//开启多Clip播放
	EditingCreatureMesh->enable_collection_playback = true;
	
}

void FCreatureAnimStoreEditorViewportClient::SetUpCamera()
{
	SetViewLocation(FVector(-100, 0, 0));
	
}

void FCreatureAnimStoreEditorViewportClient::ChangePreviewAnimation(FString AnimationName)
{
	EditingCreatureMesh->SetBluePrintActiveCollectionClip(AnimationName);
}

void FCreatureAnimStoreEditorViewportClient::ReConstructMesh()
{
	if (EditingCreatureMesh!=nullptr)
	{
		EditingCreatureMesh->DestroyComponent(true);
		EditingCreatureMesh = NewObject<UCreatureMeshComponent>();
		//设置CreatureMesh组件
		SetUpEditingCreatureMesh();
		PreviewScene->AddComponent(EditingCreatureMesh, FTransform::Identity);
		EditingCreatureMesh->SetRelativeRotation(FRotator(0, 90, 0));

		//设置Camera
		SetUpCamera();

		if (EditingStore->ClipList.Num() != 0)
		{
			//默认播放第一个Clip
			EditingCreatureMesh->SetBluePrintActiveCollectionClip(EditingStore->ClipList[0].ClipName);
		}
	}
}


