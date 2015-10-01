/********************************************************************************
** auth： God Of Pen
** desc： 用于定义当打开这种Asset类型时调用什么样的类与函数
** Ver.:  V1.0.0
*********************************************************************************/

#pragma once

#include "AssetTypeActions_Base.h"

class FCreatureAnimStoreAssetTypeActions : public FAssetTypeActions_Base
{
public:
	FCreatureAnimStoreAssetTypeActions(EAssetTypeCategories::Type InAssetCategory);

	// IAssetTypeActions interface
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	virtual uint32 GetCategories() override;
	virtual FColor GetTypeColor() const override;
	// End of IAssetTypeActions interface

private:
	EAssetTypeCategories::Type MyAssetCategory;
};
