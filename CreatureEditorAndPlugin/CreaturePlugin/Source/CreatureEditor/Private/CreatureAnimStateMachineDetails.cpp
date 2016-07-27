#include "CreatureEditorPCH.h"
#include "CreatureAnimStateMachineDetails.h"
#include "CreatureAnimStateMachine.h"
#include "CreatureAnimTransition.h"
#define LOCTEXT_NAMESPACE "CreatureAnimStateMachine"

TSharedRef<IDetailCustomization> FCreatureAnimStateMachineDetails::MakeInstance()
{
	return MakeShareable(new FCreatureAnimStateMachineDetails);
}

void FCreatureAnimStateMachineDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

	if (ObjectsBeingCustomized.Num() == 0)
	{
		return;
	}

	UCreatureAnimStateMachine *stateMachine = Cast<UCreatureAnimStateMachine>(ObjectsBeingCustomized[0].Get());


	TArray<UCreatureAnimState *> toVisit;
	TArray<UCreatureAnimTransition *> transitions;
	toVisit.Add(stateMachine->RootState);
	while (toVisit.Num() > 0)
	{
		UCreatureAnimState *state = toVisit.Pop();
		if (state == nullptr)
		{
			continue;
		}
		for (UCreatureAnimTransition *transition : state->TransitionList)
		{
			if (transitions.Contains(transition) == false)
			{
				transitions.Add(transition);
				toVisit.Add(transition->TargetState);
			}
		}
	}

	// Create a category so this is displayed early in the properties
	IDetailCategoryBuilder& MyCategory = DetailBuilder.EditCategory("CategoryName", LOCTEXT("Extra info", "State Transition Conditions"), ECategoryPriority::Important);

	for (UCreatureAnimTransition *transition : transitions)
	{
		for (FCreatureTransitionCondition &condition : transition->TransitionConditions)
		{
			MyCategory.AddCustomRow(FText::FromString(TEXT("Condition")))
				.NameContent()
				[
					SNew(STextBlock)
					.Text(FText::FromName(condition.TransitionName))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			.ValueContent().MinDesiredWidth(100)
				[
					SNew(STextBlock)
					.Text(FText::FromString((condition.TransitionFlag) ? TEXT("true") : TEXT("false")))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				];

		}


	}
}

#undef LOCTEXT_NAMESPACE