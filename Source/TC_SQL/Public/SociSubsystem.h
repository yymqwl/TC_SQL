// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SociSetting.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SociSubsystem.generated.h"

class USQLSubsystem;


/**
 *
 数据库相关操作
 */
UCLASS(Config=Game,Defaultconfig)
class TC_SQL_API USociSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	
	/** Implement this for initialization of instances of the system */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Implement this for deinitialization of instances of the system */
	virtual void Deinitialize()override;


	TObjectPtr<USQLSubsystem> Find_SQLSubsystem(FName name);
private:
	void ClearArray();

protected:
	UPROPERTY(Config,transient)
	TArray<FSociDefinition> SociDefinitions;

	/*
	UPROPERTY()
	TMap<ESocil_SQLType,UClass*> SQLSubsystemClass_Map;
	*/
	//TMap<ESocil_SQLType,UASQLSubsystem*> SQLSubsystem_Map;
	UPROPERTY()
	TMap<FName,TObjectPtr<USQLSubsystem>> SQLSubsystem_Map;//可能会有多个连接
};
