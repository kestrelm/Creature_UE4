#include "CreatureEditorPCH.h"
#include "EditorViewportClient.h"
#include "SCreatureAnimClipStoreEditorViewport.h"
#include "CreatureAnimStoreEditorViewportClient.h"
#define LOCTEXT_NAMESPACE "CreatureAnimClipStoreEditorViewport"
void SCreatureAnimClipStoreEditorViewport::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SEditorViewport::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	if (PreviewClient.IsValid())
	{
		//更新和渲染viewport
		PreviewClient->Tick(InDeltaTime);
		GEditor->UpdateSingleViewportClient(PreviewClient.Get(),true,false);
	}
}

FReply SCreatureAnimClipStoreEditorViewport::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}

FReply SCreatureAnimClipStoreEditorViewport::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}

FReply SCreatureAnimClipStoreEditorViewport::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}

FReply SCreatureAnimClipStoreEditorViewport::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}

FCursorReply SCreatureAnimClipStoreEditorViewport::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	return FCursorReply::Unhandled();
}
//////////////////////////////////////////////////////////////////////////
//必须调用父项的OnPaint才能绘制出Viewport界面
int32 SCreatureAnimClipStoreEditorViewport::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	return SEditorViewport::OnPaint(Args,AllottedGeometry,MyClippingRect,OutDrawElements,LayerId,InWidgetStyle,bParentEnabled);
}

void SCreatureAnimClipStoreEditorViewport::Construct(const FArguments& InArgs)
{
	EditStore = InArgs._ObjectToEdit;
	SEditorViewport::Construct(SEditorViewport::FArguments());

}

TSharedRef<FEditorViewportClient> SCreatureAnimClipStoreEditorViewport::MakeEditorViewportClient()
{
	TSharedPtr<FCreatureAnimStoreEditorViewportClient> TargetClient(new FCreatureAnimStoreEditorViewportClient(SharedThis(this), EditStore));
	PreviewClient = TargetClient;
	return TargetClient.ToSharedRef();
}

FText SCreatureAnimClipStoreEditorViewport::GetTitleText() const
{
	return LOCTEXT("TitleTEXT","ClipStorePreview");
}

void SCreatureAnimClipStoreEditorViewport::ChangePreviewAnimation(FString AnimationName)
{
	if (AnimationName==TEXT("None"))
	{
		return;
	}
	PreviewClient->ChangePreviewAnimation(AnimationName);
}

void SCreatureAnimClipStoreEditorViewport::ReConstructMesh()
{
	PreviewClient->ReConstructMesh();
}

#undef LOCTEXT_NAMESPACE
