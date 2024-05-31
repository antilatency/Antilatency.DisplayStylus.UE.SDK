// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "DeviceNetworkLibrary.h"
#include "DeviceNetwork.h"
#include "DeviceFilter.h"


#include "DeviceNetworkSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ANTILATENCYDISPLAYSTYLUS_API UDeviceNetworkSubsystem : public UGameInstanceSubsystem, public FTickableGameObject {
	GENERATED_BODY()

public:
	// --------------------------------------------------
	// UGameInstanceSubsystem interface 
	// --------------------------------------------------
	virtual void Initialize(FSubsystemCollectionBase& collection) override;
	virtual void Deinitialize() override;

	// --------------------------------------------------
	// FTickableObjectBase interface 
	// --------------------------------------------------
	virtual void Tick(float deltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return HasAnyFlags(RF_ClassDefaultObject) ? ETickableTickType::Never : ETickableTickType::Always; }
	virtual TStatId GetStatId() const override { return TStatId(); }
	virtual bool IsTickableWhenPaused() const override { return false; }
	virtual bool IsTickableInEditor() const override { return false; }
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }

	// --------------------------------------------------
	// UDeviceNetworkSubsystem interface 
	// --------------------------------------------------
	bool StartDeviceNetwork(UDeviceFilter* deviceFilter = nullptr);
	bool IsRunning() const;
	UDeviceNetwork* GetDeviceNetwork() const;
	void StopDeviceNetwork();

	DECLARE_EVENT(UDeviceNetworkSubsystem, FDeviceNetworkUpdated)
	FDeviceNetworkUpdated& OnDeviceNetworkUpdated(){ return _deviceNetworkUpdatedEvent; }

private:
	FDeviceNetworkUpdated _deviceNetworkUpdatedEvent;
	
	UPROPERTY()
	UDeviceNetworkLibrary* _deviceNetworkLibrary;

	EDeviceNetworkLogLevel _adnLogLevel = EDeviceNetworkLogLevel::Info;
	
	UPROPERTY()
	UDeviceNetwork* _deviceNetwork;

	uint32 _lastUpdateId = 0;
};
