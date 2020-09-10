#pragma once

#include "CoreMinimal.h"
#include "Slate/SMeshWidget.h"  

class FProceduralMeshTriData;
class CreatureCore;

class SCreatureWidget : public SMeshWidget
{
public:
    SLATE_BEGIN_ARGS(SCreatureWidget)
        : _MeshData(nullptr)
    {
	}

        SLATE_ARGUMENT(USlateVectorArtData*, MeshData)
    SLATE_END_ARGS()

	void Construct(const FArguments& Args);

	void SetMeshScale(const FVector2D& scale_in) {
		mesh_scale = scale_in;
	}

	void SetCreatureCore(CreatureCore * creature_core_in);

	void SetWorldType(EWorldType::Type world_type_in)
	{
		world_type = world_type_in;
	}

	void SetBrush(FSlateBrush * render_brush_in);

protected:
	void UpdateMesh(const FVector2D& translation, const FVector2D& local_scale);

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;        

	CreatureCore * creature_core = nullptr;
	FSlateBrush * render_brush = nullptr;
	EWorldType::Type world_type;
	FVector2D mesh_scale;
	FRenderData render_data;
};