// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "ASQLSubsystem.h"
#include "GameFramework/Actor.h"
#include "TestSociActor.generated.h"

class USQLSubsystem;
UCLASS()
class TC_SQL_API ATestSociActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestSociActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Ping_SQL();
	UFUNCTION(BlueprintCallable)
	void Test1();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY()
	TObjectPtr<USQLSubsystem> PSQLSubsys;
};
