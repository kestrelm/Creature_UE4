/********************************************************************************
** auth£∫ God Of Pen
** desc£∫ ”√”⁄‘§¿¿AnimClipStoreµƒViewport
** Ver.:  V1.0.0
*********************************************************************************/
#include "Engine.h"
#include "UnrealEd.h"
#include "EditorUndoClient.h"
#include "SEditorViewport.h"
#pragma  once
class FCreatureAnimStoreEditorViewportClient;
class SCreatureAnimClipStoreEditorViewport : public SEditorViewport
{
public:
	SLATE_BEGIN_ARGS(SCreatureAnimClipStoreEditorViewport) {}
		SLATE_ARGUMENT(class UCreatureAnimationClipsStore*, ObjectToEdit)
	SLATE_END_ARGS()


	// SWidget interface
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	// End of SWidget interface
	
	void Construct(const FArguments& InArgs);

	void ChangePreviewAnimation(FString AnimationName);

	void ReConstructMesh();

protected:

	// SEditorViewport interface
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	// End of SEditorViewport interface

	virtual FText GetTitleText() const;
private:
	TSharedPtr<FCreatureAnimStoreEditorViewportClient> PreviewClient;

	class UCreatureAnimationClipsStore* EditStore;
	
};
