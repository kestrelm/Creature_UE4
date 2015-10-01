#include "CreatureEditorPCH.h"
#include "CreatureAnimStateMachineDetails.h"
#define LOCTEXT_NAMESPACE "CreatureAnimStateMachine"

TSharedRef<IDetailCustomization> FCreatureAnimStateMachineDetails::MakeInstance()
{
	return MakeShareable(new FCreatureAnimStateMachineDetails);
}

void FCreatureAnimStateMachineDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Create a category so this is displayed early in the properties
	IDetailCategoryBuilder& MyCategory = DetailBuilder.EditCategory("CategoryName", LOCTEXT("Extra info", "Test header name"), ECategoryPriority::Important);

	//You can get properties using the detailbuilder
	//MyProperty= DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(MyClass, MyClassPropertyName));

	MyCategory.AddCustomRow(LOCTEXT("Extra info", "Row header name"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Extra info", "Custom row header name"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
	.ValueContent().MinDesiredWidth(500)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Extra info", "Custom row content"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		];
}

#undef LOCTEXT_NAMESPACE