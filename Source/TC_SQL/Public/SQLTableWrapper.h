
#pragma once

#include "CoreMinimal.h"
#include "SociLog.h"
#include "SQLTableWrapper.generated.h"

USTRUCT()
struct FSQLRow
{
	GENERATED_BODY();
};

template<typename T,typename = typename TEnableIf<TIsDerivedFrom<T,FSQLRow>::Value>::Type  >
//USTRUCT()
struct SQLRowWrapper 
{
	//GENERATED_BODY();
public:
	//SQLTableWrapper(UScriptStruct* tableStruct);
	
	SQLRowWrapper()
	{
		//TableStruct = T::StaticStruct();
		//TableName = T::StaticStruct()->GetName();
		//UScriptStruct* TableStruct= T::StaticStruct();
		//TableStruct->Link()
		Set_Change.Empty();
		//TSubclassOf<>
	}

	/*
	template<typename T2>
	void SetValue(FName name,const T2& t2 ,bool bchange = true)
	｛

		
		auto property =  T::StaticStruct()->FindPropertyByName(name);
		if (property==nullptr)
		{
			SOCI_ERROR(TEXT("SQLRowWrapper SetValue Not Found:%s"),*name.ToString());
			return;
		}
	｝*/

	void SetValue(FName name,const FString& InValue,bool bchange = true)
	{
		this->SetValue(name,&InValue,bchange);
	}
	void SetValue(FName name,const void* InValue ,bool bchange = true)
	{
		auto property =  T::StaticStruct()->FindPropertyByName(name);
		if (property==nullptr)
		{
			TC_SQL_ERROR(TEXT("SQLRowWrapper SetValue Not Found:%s"),*name.ToString());
			return;
		}
		bool bResult = property->Identical(&Data, InValue);
		if (!bResult)
		{
			property->SetValue_InContainer(&Data,InValue);
			Set_Change.Add(name);
		}
	}
	/*
	void SetValue(FName name,const void* InValue ,bool bchange = true)
	{
		auto property =  T::StaticStruct()->FindPropertyByName(name);
		if (property==nullptr)
		{
			SOCI_ERROR(TEXT("SQLRowWrapper SetValue Not Found:%s"),*name.ToString());
			return;
		}
		Cast<>()
		CastChecked<>()
		CastChec
		//Cast<FString>() InValue
		
		bool bResult = property->Identical(&Data, InValue);
		if (!bResult)
		{
			property->SetValue_InContainer(&Data,InValue);
			Set_Change.Add(name);
		}
	}*/
	//必须知道类型
	void GetValue(FName name,void* OutValue)
	{
		UScriptStruct* TableStruct= T::StaticStruct();
		auto property =  TableStruct->FindPropertyByName(name);
		if (property==nullptr)
        {
        	TC_SQL_ERROR(TEXT("SQLRowWrapper GetValue Not Found:%s"),*name.ToString());
        	return;
        }
		property->GetValue_InContainer(&Data,OutValue);
	}
	FString GetStrValue(FName key)
	{
		//UScriptStruct* TableStruct= T::StaticStruct();
		auto property =  T::StaticStruct()->FindPropertyByName(key);
		if (property==nullptr)
		{
			TC_SQL_ERROR(TEXT("SQLRowWrapper GetValue Not Found:%s"),*key.ToString());
			return FString();
		}
		FString str;
		if (property->IsA(FStrProperty::StaticClass()) )//字符串
		{
			property->GetValue_InContainer( &Data,&str);
			//str2 +=  FString::Format(TEXT("'{0}',"),{ *str});
		}
		else if (property->IsA(FIntProperty::StaticClass()))//int
		{
			int i32;
			property->GetValue_InContainer( &Data,&i32);
			str = FString::FromInt(i32);
		}
		else
		{
			TC_SQL_ERROR(TEXT("GetStrValue Not Supported:%s"),*key.ToString());
		}
		return str;
	}
	void ClearChange()
	{
		Set_Change.Empty();
	}
	/*
	~SQLRowWrapper()
	{
		SQL_LOG(TEXT("~SQLRowWrapper()"));
	}*/
public:
	//UScriptStruct* TableStruct;//
	//FName TableName;//表名
	T Data;
	TSet<FName> Set_Change;//数值变动表
	//SQLTable<T>*		OuterPrivate;
};

template<typename T,typename = typename TEnableIf<TIsDerivedFrom<T,FSQLRow>::Value>::Type  >
class SQLTableWrapper 
{
public:
	SQLTableWrapper()
	{
		Rows.Empty();
		TableName =	FName(T::StaticStruct()->GetName());
		PrimaryKey = FName("Id");
	}

	FString GetUpdateSQLString(TSharedRef<SQLRowWrapper<T>>  pt_Row)
	{
		FString str_result ;
		if (pt_Row->Set_Change.Num() == 0)
		{
			return str_result;
		}
		auto  key_str =GetConditionWithKey(pt_Row);
		FString str1 = FString::Format(TEXT("update {0} set "),{*TableName.ToString()});
		for (auto  str_prop : pt_Row->Set_Change)
		{
			auto property =  T::StaticStruct()->FindPropertyByName(str_prop);
			FString str_3  = pt_Row->GetStrValue(str_prop);
			if (property->IsA(FStrProperty::StaticClass()))//特殊处理
			{
				str_3 = FString::Format(TEXT("'{0}'"),{*str_3});
			}
			str1 += FString::Format( TEXT("{0}={1},"),{*str_prop.ToString(),*str_3});
		}
		str1.LeftInline(str1.Len()-1);//裁剪掉,
		str_result = FString::Format(TEXT("{0} {1}"),{*str1,*key_str}) ;
	    return 	str_result;
	}
	FString GetConditionWithKey(TSharedRef<SQLRowWrapper<T>>  pt_Row)
	{
		FString str_data = pt_Row->GetStrValue(PrimaryKey);
		return FString::Format(TEXT("where {0}={1}"),{*PrimaryKey.ToString(),*str_data});
	}
	
	FString GetDeleteSQLString(TSharedRef<SQLRowWrapper<T>>  pt_Row)
	{
		auto  key_str =GetConditionWithKey(pt_Row);
		return FString::Format(TEXT("delete from {0} {1}"),{*TableName.ToString(),*key_str});
	}
	FString GetInsertSQLString( TSharedRef<SQLRowWrapper<T>>  pt_Row)
	{

		//FString	str_tb = T::StaticStruct()->GetName();
		FString str1 = FString::Format(TEXT("insert into {0}("),{*TableName.ToString()});
		FString str2 = FString(TEXT("values("));
		for (TFieldIterator<FProperty>  it(T::StaticStruct()); it;++it )
		{
			FString key_str= it->GetNameCPP();
			FString data_str = pt_Row->GetStrValue(FName(key_str));
			//if (property->IsA(FIntProperty::StaticClass()))//int
			if ( (*it)->IsA(FStrProperty::StaticClass()) )//字符串特殊处理
			{
				str2 +=  FString::Format(TEXT("'{0}',"),{ *data_str});
			}
			else
			{
				str2 +=  FString::Format(TEXT("{0},"),{ *data_str});
			}
			str1 += FString::Format(TEXT("{0},"),{*key_str});
		}
		str1.LeftInline(str1.Len()-1);
		str2.LeftInline(str2.Len()-1);
		
		FString str = FString::Format(TEXT("{0}) {1})"),{*str1,*str2});
		return str;
	}
	/*
	~SQLTableWrapper()
	{
		SOCI_LOG(TEXT("~SQLTableWrapper()"));
	}*/
	
	
public:
	TArray<TSharedRef<SQLRowWrapper<T>>> Rows;
	//TMap<FName,TWeakPtr<FProperty>> Map_Propertys;
	FName TableName;
	FName PrimaryKey;
};
