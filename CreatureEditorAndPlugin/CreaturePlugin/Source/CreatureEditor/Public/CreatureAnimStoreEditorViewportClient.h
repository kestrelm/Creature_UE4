/********************************************************************************
** Author God Of Pen
** Ver.:  V1.0.0
*********************************************************************************/

#pragma  once
#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "PreviewScene.h"
#include "AdvancedPreviewScene.h"
class FCreatureAnimStoreEditorViewportClient : public FEditorViewportClient
{
public:
	/** Constructor */
	explicit FCreatureAnimStoreEditorViewportClient(const TWeakPtr<class SEditorViewport>& InEditorViewportWidget = nullptr,class UCreatureAnimationClipsStore* EditingAnimStore=nullptr);
	//~CreatureAnimStoreEditorViewportClient();

	// FViewportClient interface
	virtual void Tick(float DeltaSeconds) override;
	// End of FViewportClient interface

	// FEditorViewportClient interface
	virtual FLinearColor GetBackgroundColor() const override;
	// End of FEditorViewportClient interface

	// FSerializableObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End of FSerializableObject interface

	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;

	void ChangePreviewAnimation(FString AnimationName);

	void ReConstructMesh();
private:
	//”√”⁄‘§¿¿µƒScene
	//FPreviewScene OwnerScene;

	TSharedPtr<FAdvancedPreviewScene> OwnerScene;

	class UCreatureMeshComponent* EditingCreatureMesh;
	class UCreatureAnimationClipsStore* EditingStore;
private:
	void SetUpEditingCreatureMesh();
	void SetUpCamera();
};
