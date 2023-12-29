// Fill out your copyright notice in the Description page of Project Settings.


#include "SociSubsystem.h"
#include "SQLSubsystem.h"
#include "SociLog.h"
#include "SociSetting.h"

void USociSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	//auto sociSetting =GetDefault<USociSetting>();
	
	//SOCI_LOG(TEXT("sociSetting:%d"),this->SociDefinitions.Num());

	//GetGameInstance()->GetTimerManager().SetTimer()
	ClearArray();

	///
	/*
	TArray<UClass*> SubsystemClasses;
	GetDerivedClasses(USQLSubsystem::StaticClass(), SubsystemClasses, true);
	for (UClass* SubsystemClass : SubsystemClasses)
	{
		
		SOCI_LOG(TEXT("Soci Add Classes:%s"),*SubsystemClass->GetName())
		if (SubsystemClass->HasAnyClassFlags(CLASS_Abstract))
		{
			continue;
		}
		auto p_sqlsubsys = NewObject<UASQLSubsystem>(this,SubsystemClass);

		auto sqltype = p_sqlsubsys->Get_SQLType();
		if (SQLSubsystem_Map.Contains(sqltype))
		{
			SQL_ERROR(TEXT("SQLSubsystem_Map Has Same Type %s"),sqltype);
		}
		
		auto p_sqlsubsys = SubsystemClass->GetDefaultObject<UASQLSubsystem>();
		check(p_sqlsubsys);
		auto sqltype = p_sqlsubsys->Get_SQLType();
		
		if ( SQLSubsystemClass_Map.Contains(sqltype))
		{
			SQL_ERROR(TEXT("SQLSubsystem_Map Has Same Type %d"),sqltype);
			continue;
		}
		SQLSubsystemClass_Map.Add(sqltype,SubsystemClass);
	}
	*/
	//设置

	for (const FSociDefinition& socidef : this->SociDefinitions)
	{
		/*
		if (!SQLSubsystemClass_Map.Contains(socidef.SQLType))
		{
			SOCI_LOG(TEXT("Null Adaptater:%d"),socidef.SQLType)
			continue;
		}
		auto sqlclass =SQLSubsystemClass_Map[socidef.SQLType];
		*/
		
		auto sql_subsys= NewObject<USQLSubsystem>(this);//存储泄漏问题
		sql_subsys->Initialize(&socidef);
		
		if (SQLSubsystem_Map.Contains(socidef.DefName))
		{
			//TC_SQL_ERROR(TEXT("SQLSubsystem"));
			TC_SQL_ERROR(TEXT("SQLSubsystem_Map Same Name %s"), *socidef.DefName.ToString());
			//SQL_ERROR(TEXT("GetStrValue Not Supported:%s"));
			continue;
		}
		
		SQLSubsystem_Map.Add(socidef.DefName,sql_subsys);
	}

	/*SOCI_LOG(TEXT("所有创建的类:%d"),SQLSubsystem_Map.Num())
	
	for (const FSociDefinition& socidef : sociSetting->SociDefinitions)
	{
		
		NewObject<>()
	}*/
	
	/*
	foreach(auto element in sociSetting->SociDefinitions)
	{
		
	}*/
	//this->GetGameInstance()->GetSubsystem<>()

	
}


TObjectPtr<USQLSubsystem> USociSubsystem::Find_SQLSubsystem(FName name)
{
	return  SQLSubsystem_Map[name];
}

void USociSubsystem::ClearArray()
{
	//SQLSubsystemClass_Map.Empty();//清空所有
	SQLSubsystem_Map.Empty();
}

void USociSubsystem::Deinitialize()
{

	TC_SQL_LOG(TEXT("USociSubsystem Deinitialize"));
	for (TTuple<FName, TObjectPtr<USQLSubsystem>>&  sqlsubsys : SQLSubsystem_Map)
	{
		sqlsubsys.Value->Deinitialize();
	}
	/*
	for (TTuple<ESocil_SQLType, UClass* >&  socil_sqltype : SQLSubsystemClass_Map)
	{
		SOCI_LOG(TEXT("析构:%s"),*socil_sqltype.Value->GetName())
	}*/
	ClearArray();
}
