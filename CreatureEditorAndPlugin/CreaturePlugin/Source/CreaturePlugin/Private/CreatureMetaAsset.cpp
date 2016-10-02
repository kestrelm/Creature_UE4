
#include "CreaturePluginPCH.h"
#include "CreatureMetaAsset.h"
#include "CreatureCore.h"

FString& UCreatureMetaAsset::GetJsonString()
{
	return jsonString;
}

void UCreatureMetaAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}

void 
UCreatureMetaAsset::BuildMetaData()
{
	TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
	TSharedRef< TJsonReader<> > reader = TJsonReaderFactory<>::Create(jsonString);
	meta_data.clear();

	if (FJsonSerializer::Deserialize(reader, jsonObject))
	{
		// Fill mesh data
		if (jsonObject->HasField(TEXT("meshes"))) {
			auto meshes_obj = jsonObject->GetObjectField(TEXT("meshes"));
			for (auto cur_data : meshes_obj->Values)
			{
				auto cur_json = cur_data.Value->AsObject();
				auto cur_id = cur_json->GetIntegerField(TEXT("id"));
				auto cur_start_index = cur_json->GetIntegerField(TEXT("startIndex"));
				auto cur_end_index = cur_json->GetIntegerField(TEXT("endIndex"));

				meta_data.mesh_map.Add(cur_id, TTuple<int32, int32>(cur_start_index, cur_end_index));
			}
		}

		// Fill switch order data
		if (jsonObject->HasField(TEXT("regionOrders"))) {
			auto orders_obj = jsonObject->GetObjectField(TEXT("regionOrders"));
			for (auto cur_data : orders_obj->Values)
			{
				auto cur_anim_name = cur_data.Key;
				TMap<int32, TArray<int32> > cur_switch_order_map;

				auto cur_obj_array = cur_data.Value->AsArray();
				for (auto cur_switch_json : cur_obj_array)
				{
					auto switch_obj = cur_switch_json->AsObject();
					auto cur_switch_list = switch_obj->GetArrayField(TEXT("switch_order"));

					TArray<int32> cur_switch_ints;
					for (auto cur_switch_val : cur_switch_list)
					{
						cur_switch_ints.Add((int32)cur_switch_val->AsNumber());
					}

					auto cur_switch_time = switch_obj->GetIntegerField(TEXT("switch_time"));
					
					cur_switch_order_map.Add(cur_switch_time, cur_switch_ints);
				}


				meta_data.anim_order_map.Add(cur_anim_name, cur_switch_order_map);
			}
		}

		// Fill event triggers
		if (jsonObject->HasField(TEXT("eventTriggers"))) {
			auto events_obj = jsonObject->GetObjectField(TEXT("eventTriggers"));
			for (auto cur_data : events_obj->Values)
			{
				auto cur_anim_name = cur_data.Key;
				TMap<int32, FString> cur_events_map;
				auto cur_obj_array = cur_data.Value->AsArray();
				for (auto cur_events_json : cur_obj_array)
				{
					auto events_obj = cur_events_json->AsObject();
					auto event_name = events_obj->GetStringField(TEXT("event_name"));
					auto switch_time = events_obj->GetIntegerField(TEXT("switch_time"));

					cur_events_map.Add(switch_time, event_name);
				}

				meta_data.anim_events_map.Add(cur_anim_name, cur_events_map);
			}
		}
	}
}

CreatureMetaData *
UCreatureMetaAsset::GetMetaData()
{
	return &meta_data;
}
