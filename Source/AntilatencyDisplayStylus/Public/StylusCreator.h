// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "DeviceNetworkSubsystem.h"
#include "Display.h"

#include <AntilatencyAltTracking/Public/AltTrackingLibrary.h>
#include <AntilatencyAltTracking/Public/TrackingCotaskConstructor.h>
#include <AntilatencyHardwareExtensionInterface/Public/HardwareExtensionInterfaceLibrary.h>
#include <AntilatencyHardwareExtensionInterface/Public/HardwareExtensionInterfaceCotaskConstructor.h>
#include <AntilatencyHardwareExtensionInterface/Public/HardwareExtensionInterfaceTypes.h>

#include "Stylus.h"
#include "StylusCreator.generated.h"


UCLASS( ClassGroup=(Antilatency), meta=(BlueprintSpawnableComponent) )
class ANTILATENCYDISPLAYSTYLUS_API UStylusCreator : public UActorComponent {
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStylusCreator();

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> StylusTags = {"Stylus"};

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<AStylus> StylusTemplate;

protected:
	UPROPERTY(BlueprintReadOnly)
	UAltTrackingLibrary* _trackingLibrary = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UTrackingCotaskConstructor* _trackingCotaskConstructor = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UHardwareExtensionInterfaceLibrary* _heiLibrary = nullptr;

	UPROPERTY(BlueprintReadOnly)
	UHardwareExtensionInterfaceCotaskConstructor* _heiCotaskConstructor = nullptr;
	
	UPROPERTY(EditAnywhere)
	EPins _buttonPin = EPins::IO1;

	UPROPERTY()
	UDeviceNetworkSubsystem* _deviceNetworkSubsystem = nullptr;

	UPROPERTY()
	UDisplay* _display = nullptr;

	UPROPERTY()
	bool _isInitialized = false;

	FDelegateHandle _onDeviceNetworkChangedHandle;
	FDelegateHandle _onDisplayReadyHandle;

	// Self interface
protected:
	bool InitLibraries();
	void OnDeviceNetworkChanged();
	bool FindDisplay();

	// UActorComponent interface
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;


		
};
