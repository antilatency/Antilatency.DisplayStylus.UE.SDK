// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ProceduralMeshComponent.h"
#include "Materials/Material.h"

#include "DeviceNetworkSubsystem.h"

#include <AntilatencyAltEnvironmentSelector/Public/EnvironmentSelectorLibrary.h>
#include <AntilatencyAltTracking/Public/AltEnvironment.h>
#include <AntilatencyAltEnvironmentRectangle/Public/AltEnvironmentRectangleLibrary.h>
#include <AntilatencyPhysicalConfigurableEnvironment/Public/PhysicalConfigurableEnvironmentLibrary.h>
#include <AntilatencyPhysicalConfigurableEnvironment/Public/PhysicalConfigurableEnvironmentCotaskConstructor.h>
#include <AntilatencyPhysicalConfigurableEnvironment/Public/PhysicalConfigurableEnvironmentCotask.h>

#include "Display.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDisplayReady);

UCLASS( ClassGroup=(Antilatency), meta=(BlueprintSpawnableComponent) )
class ANTILATENCYDISPLAYSTYLUS_API UDisplay : public USceneComponent {
	GENERATED_BODY()

public:	
	UDisplay();

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool SyncWithPhysicalDisplayRotation = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool ShowDisplayBorder = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float BorderWidth = 2.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UMaterial* BorderMaterial = nullptr;

	UPROPERTY(BlueprintAssignable)
	FOnDisplayReady OnDisplayReady;

protected:
	UPROPERTY()
	FVector _screenPosition = {};

	UPROPERTY()
	FVector _screenX = {};

	UPROPERTY()
	FVector _screenY = {};

	UPROPERTY()
	UDeviceNetworkSubsystem* _deviceNetworkSubsystem = nullptr;

	UPROPERTY()
	UPhysicalConfigurableEnvironmentLibrary* _pceLibrary = nullptr;

	UPROPERTY()
	UPhysicalConfigurableEnvironmentCotaskConstructor* _pceCotaskConstructor = nullptr;

	UPROPERTY()
	UPhysicalConfigurableEnvironmentCotask* _pceCotask = nullptr;

	UPROPERTY()
	UEnvironmentSelectorLibrary* _environmentSelectorLibrary = nullptr;

	UPROPERTY()
	UAltEnvironmentRectangleLibrary* _rectangleLibrary = nullptr;

	UPROPERTY()
	UAltEnvironment* _environment = nullptr;

	UPROPERTY()
	UProceduralMeshComponent* _displayMesh = nullptr;
	
	const FString _displayNodeHardwareName = "AntilatencyPhysicalConfigurableEnvironment";

	// Self interface
public:
	UFUNCTION(BlueprintPure)
	bool GetDisplayProperties(FVector& screenPosition, FVector& screenX, FVector& screenY) const;

	UFUNCTION(BlueprintPure)
	bool GetEnvironment(UAltEnvironment*& environment) const;

	UFUNCTION(BlueprintPure)
	bool GetPhysicalDisplayRotation(FQuat& rotation) const;

	UFUNCTION(BlueprintPure)
	bool GetHalfScreenSize(FVector2D& result) const;

protected:
	UFUNCTION()
	void OnDeviceNetworkChanged();
	bool InitLibraries();
	bool FindPCENode(FAdnNode& result) const;
	bool FindDisplayMeshComponent();
	bool UpdateDisplayMesh();

	// UActorComponent interface
public:	
	virtual void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
	
protected:
	virtual void BeginPlay() override;
};
