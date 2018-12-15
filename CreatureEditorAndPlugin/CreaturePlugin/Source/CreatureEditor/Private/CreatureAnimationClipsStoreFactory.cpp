#include "CreatureAnimationClipsStoreFactory.h"
#include "CreatureAnimationClipsStore.h"
#define LOCTEXT_NAMESPACE "CreatureAnimationClipsStoreFactory"
UCreatureAnimationClipsStoreFactory::UCreatureAnimationClipsStoreFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UCreatureAnimationClipsStore::StaticClass();
}
UObject* UCreatureAnimationClipsStoreFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UCreatureAnimationClipsStore>(InParent, Class, Name, Flags);

}

#undef LOCTEXT_NAMESPACE