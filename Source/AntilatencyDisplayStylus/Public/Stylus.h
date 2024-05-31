#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Display.h"

#include <AntilatencyAltTracking/Public/AltTrackingCotask.h>
#include <AntilatencyHardwareExtensionInterface/Public/HardwareExtensionInterfaceCotask.h>
#include <AntilatencyHardwareExtensionInterface/Public/InputPin.h>

#include "Stylus.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStylusPoseUpdated, FVector, stylusLocation, FRotator, stylusRotation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStylusButtonStateUpdated, bool, stylusButtonPressed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStylusDestroy);

UCLASS(ClassGroup=(Antilatency))
class ANTILATENCYDISPLAYSTYLUS_API AStylus : public AActor {
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStylus();

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ExtrapolationTime = 0.06f;

	UPROPERTY(BlueprintAssignable)
	FOnStylusPoseUpdated OnStylusPoseUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnStylusButtonStateUpdated OnStylusButtonStateUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnStylusDestroy OnStylusDestroy;

protected:
	UPROPERTY()
	bool _isInitialized = false;

	UPROPERTY()
	UDisplay* _display = nullptr;

	UPROPERTY()
	UAltTrackingCotask* _trackingCotask = nullptr;

	UPROPERTY()
	UHardwareExtensionInterfaceCotask* _heiCotask = nullptr;

	UPROPERTY()
	UInputPin* _inputPin = nullptr;

	UPROPERTY()
	FVector _extrapolatedLocation = FVector::ZeroVector;

	UPROPERTY()
	FRotator _extrapolatedRotation = FRotator::ZeroRotator;

	// Self interface
public:
	bool Initialize(UAltTrackingCotask* trackingCotask, UHardwareExtensionInterfaceCotask* heiCotask, UInputPin* inputPin, UDisplay* display);
	bool IsActive() const;
	bool GetExtrapolatedPose(FVector& location, FRotator& rotation) const;
	
	// AActor interface
public:	
	// Called every frame
	virtual void Tick(float deltaTime) override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
	
};
