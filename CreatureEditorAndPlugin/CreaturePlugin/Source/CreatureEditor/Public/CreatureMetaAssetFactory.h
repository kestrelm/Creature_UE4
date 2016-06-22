/********************************************************************************
** auth�� God Of Pen
** desc�� ���ڲ���CreatureAnimationAsset��ͬʱ��������JSon�ļ��������Ĺ���
** Ver.:  V1.0.0
*********************************************************************************/
#include "UnrealEd.h"
#include "CreatureMetaAssetFactory.generated.h"
#pragma once
UCLASS()
class CREATUREEDITOR_API UCreatureMetaAssetFactory : public UFactory, public FReimportHandler
{
	GENERATED_UCLASS_BODY()

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface

	// Begin FReimportHandler interface
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
    virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	virtual int32 GetPriority() const override;
	// End FReimportHandler interface

	bool ImportSourceFile(class UCreatureMetaAsset *forAsset, FString importFilename) const;
};
