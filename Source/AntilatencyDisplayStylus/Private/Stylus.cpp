#include "Stylus.h"

#include "AntilatencyAltTracking/Public/MathFunctionLibrary.h"

#include "DisplayStylusLog.h"

AStylus::AStylus() {
	PrimaryActorTick.bCanEverTick = true;

}

void AStylus::BeginPlay() {
	Super::BeginPlay();
	
}

void AStylus::EndPlay(const EEndPlayReason::Type endPlayReason) {
	Super::EndPlay(endPlayReason);

	OnStylusDestroy.Broadcast();

	_heiCotask = nullptr;
	_inputPin = nullptr;
	_trackingCotask = nullptr;
	_display = nullptr;
}

void AStylus::Tick(float deltaTime) {
	Super::Tick(deltaTime);

	if (!_isInitialized) {
		return;
	}

	if (!IsActive()) {
		Destroy();
		return;
	}

	FTrackingState trackingState;
	const FVector placementLocation = FVector::ZeroVector;
	const FQuat placementRotation = FQuat::Identity;
	auto exception = _trackingCotask->GetExtrapolatedState(FAltPose(placementLocation, placementRotation), ExtrapolationTime, trackingState);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Error, TEXT("AStylus::Tick: failed to get extrapolated tracking state"));
		return;
	}

	FQuat displayRotation;
	if (!_display->GetPhysicalDisplayRotation(displayRotation)) {
		return;
	}
	displayRotation = displayRotation.Inverse();
	
	const auto location = displayRotation.RotateVector(MathLibrary::AntilatencyVectorToUe(trackingState.Pose.Position));
	const auto rotation = displayRotation * MathLibrary::AntilatencyQuatToUe(trackingState.Pose.Rotation);
	const FTransform transform(rotation, location);
	SetActorRelativeTransform(transform);

	_extrapolatedLocation = GetActorLocation();
	_extrapolatedRotation = GetActorRotation();

	OnStylusPoseUpdated.Broadcast(_extrapolatedLocation, _extrapolatedRotation);

	EPinState pinState;
	exception = _inputPin->GetState(pinState);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Error, TEXT("AStylus::Tick: failed to get input pin state"));
		return;
	}

	OnStylusButtonStateUpdated.Broadcast(pinState == EPinState::Low);
}

bool AStylus::Initialize(UAltTrackingCotask* trackingCotask, UHardwareExtensionInterfaceCotask* heiCotask, UInputPin* inputPin, UDisplay* display) {
	if (trackingCotask == nullptr || !trackingCotask->IsValid()) {
		DISPLAYSTYLUS_LOG(Error, TEXT("AStylus::Initialize: tracking cotask is null"));
		return false;
	}

	if (heiCotask == nullptr || !heiCotask->IsValid()) {
		DISPLAYSTYLUS_LOG(Error, TEXT("AStylus::Initialize: HEI cotask is null"));
		return false;
	}

	if (inputPin == nullptr || !inputPin->IsValid()) {
		DISPLAYSTYLUS_LOG(Error, TEXT("AStylus::Initialize: input pin is null"));
		return false;
	}

	if (display == nullptr) {
		DISPLAYSTYLUS_LOG(Error, TEXT("AStylus::Initialize: display is null"));
		return false;
	}

	_trackingCotask = trackingCotask;
	_heiCotask = heiCotask;
	_inputPin = inputPin;
	_display = display;

	_isInitialized = true;

	return true;
}

bool AStylus::IsActive() const {
	if (_trackingCotask == nullptr || !_trackingCotask->IsValid()) {
		return false;
	}

	bool trackingTaskFinished;
	auto exception = _trackingCotask->IsTaskFinished(trackingTaskFinished);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Warning, TEXT("AStylus::IsActive: IsTaskFinished check for tracking is failed"));
		return false;
	}

	if (trackingTaskFinished) {
		return false;
	}

	if (_heiCotask == nullptr || !_heiCotask->IsValid()) {
		return false;
	}

	bool heiTaskFinished;
	exception = _heiCotask->IsTaskFinished(heiTaskFinished);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Warning, TEXT("AStylus::IsActive: IsTaskFinished check for HEI is failed"));
		return false;
	}

	if (heiTaskFinished) {
		return false;
	}

	return true;
}

bool AStylus::GetExtrapolatedPose(FVector& location, FRotator& rotation) const {
	if (!_isInitialized) {
		return false;
	}

	location = _extrapolatedLocation;
	rotation = _extrapolatedRotation;

	return true;
}

