#include "StylusCreator.h"

#include "DisplayStylusLog.h"

#include <AntilatencyAltTracking/Public/AltEnvironment.h>
#include <AntilatencyDeviceNetwork/Public/DeviceNetworkConstants.h>

#include "Kismet/GameplayStatics.h"


UStylusCreator::UStylusCreator() {	
	PrimaryComponentTick.bCanEverTick = true;
}


void UStylusCreator::BeginPlay() {
	Super::BeginPlay();

	InitLibraries();
	
	FindDisplay();

	const auto gameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	if (gameInstance == nullptr) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDisplay::BeginPlay: failed to get game instance"));
		return;
	}

	_deviceNetworkSubsystem = gameInstance->GetSubsystem<UDeviceNetworkSubsystem>();
	if (_deviceNetworkSubsystem == nullptr) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDisplay::BeginPlay: failed to get UDeviceNetworkSubsystem"));
		return;
	}

	if (!_deviceNetworkSubsystem->IsRunning()) {
		_deviceNetworkSubsystem->StartDeviceNetwork();
	}

	_deviceNetworkSubsystem->OnDeviceNetworkUpdated.AddUniqueDynamic(this, &UStylusCreator::OnDeviceNetworkChanged);

	OnDeviceNetworkChanged();
}


void UStylusCreator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}


void UStylusCreator::EndPlay(const EEndPlayReason::Type endPlayReason) {
	Super::EndPlay(endPlayReason);

	if (_deviceNetworkSubsystem != nullptr) {
		_deviceNetworkSubsystem->OnDeviceNetworkUpdated.RemoveDynamic(this, &UStylusCreator::OnDeviceNetworkChanged);
	}

	if (_display != nullptr) {
		_display->OnDisplayReady.RemoveDynamic(this, &UStylusCreator::OnDeviceNetworkChanged);
	}
}


bool UStylusCreator::InitLibraries() {
	_trackingLibrary = UAltTrackingLibrary::GetLibrary();
	if (_trackingLibrary == nullptr || !_trackingLibrary->IsValid()) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UStylusCreator::UStylusCreator: failed to get tracking library"));
		return false;
	}

	auto exception = _trackingLibrary->CreateTrackingCotaskConstructor(_trackingCotaskConstructor);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UStylusCreator::UStylusCreator: failed to create tracking cotask constructor"));
		return false;
	}

	_heiLibrary = UHardwareExtensionInterfaceLibrary::GetLibrary();
	if (_heiLibrary == nullptr || !_heiLibrary->IsValid()) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UStylusCreator::UStylusCreator: failed to get HEI library"));
		return false;
	}

	exception = _heiLibrary->GetCotaskConstructor(_heiCotaskConstructor);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UStylusCreator::UStylusCreator: failed to create HEI cotask constructor"));
		return false;
	}

	return true;
}

void UStylusCreator::OnDeviceNetworkChanged() {
	if (_display == nullptr) {
		return;
	}

	UAltEnvironment* environment;
	if (!_display->GetEnvironment(environment)) {
		return;
	}

	if (_trackingCotaskConstructor == nullptr || !_trackingCotaskConstructor->IsValid()) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UStylusCreator::OnDeviceNetworkChanged: tracking cotask constructor is null"));
		return;
	}

	auto network = _deviceNetworkSubsystem->GetDeviceNetwork();

	TArray<FAdnNode> altNodes;
	auto exception = _trackingCotaskConstructor->FindSupportedNodes(network, altNodes);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UStylusCreator::OnDeviceNetworkChanged: failed to get alt nodes"));
		return;
	}

	for (auto& altNode : altNodes) {
		ENodeStatus altStatus;
		exception = network->NodeGetStatus(altNode, altStatus);
		if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
			continue;
		}

		if (altStatus != ENodeStatus::Idle) {
			continue;
		}

		FAdnNode altParent;
		exception = network->NodeGetParent(altNode, altParent);
		if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
			continue;
		}

		if (altParent == UDeviceNetworkConstants::NullNode) {
			continue;
		}

		if (_heiCotaskConstructor == nullptr || !_heiCotaskConstructor->IsValid()) {
			DISPLAYSTYLUS_LOG(Error, TEXT("UStylusCreator::OnDeviceNetworkChanged: HEI cotask constructor is null"));
			return;
		}

		bool heiTaskIsSupported;
		exception = _heiCotaskConstructor->IsSupported(network, altParent, heiTaskIsSupported);
		if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
			continue;
		}

		if (!heiTaskIsSupported) {
			continue;
		}

		ENodeStatus stylusStatus;
		exception = network->NodeGetStatus(altParent, stylusStatus);
		if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
			continue;
		}

		if (stylusStatus != ENodeStatus::Idle) {
			return;
		}

		UAltTrackingCotask* trackingCotask;
		exception = _trackingCotaskConstructor->StartTask(network, altNode, environment, trackingCotask);
		if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
			DISPLAYSTYLUS_LOG(Warning, TEXT("UStylusCreator::OnDeviceNetworkChanged: failed to start tracking task"));
			continue;
		}

		UHardwareExtensionInterfaceCotask* heiCotask;
		exception = _heiCotaskConstructor->StartTask(network, altParent, heiCotask);
		if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
			DISPLAYSTYLUS_LOG(Warning, TEXT("UStylusCreator::OnDeviceNetworkChanged: failed to start HEI task"));
			continue;
		}

		UInputPin* inputPin;
		exception = heiCotask->CreateInputPin(_buttonPin, inputPin);
		if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
			DISPLAYSTYLUS_LOG(Warning, TEXT("UStylusCreator::OnDeviceNetworkChanged: failed to create input pin"));
			continue;
		}

		exception = heiCotask->Run();
		if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
			DISPLAYSTYLUS_LOG(Warning, TEXT("UStylusCreator::OnDeviceNetworkChanged: failed to run HEI task"));
			continue;
		}

		const auto displayTransform = _display->GetComponentTransform();
		FActorSpawnParameters spawnInfo;
		spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		auto stylus = GetWorld()->SpawnActor<AStylus>(StylusTemplate, displayTransform.GetLocation(), displayTransform.GetRotation().Rotator(), spawnInfo);

		if (stylus == nullptr) {
			DISPLAYSTYLUS_LOG(Warning, TEXT("UStylusCreator::OnDeviceNetworkChanged: failed to spawn stylus"));
			continue;
		}

		FAttachmentTransformRules attachmentRules(EAttachmentRule::SnapToTarget, false);
		stylus->AttachToComponent(_display, attachmentRules);

		if (!stylus->Initialize(trackingCotask, heiCotask, inputPin, _display)) {
			stylus->Destroy();
		}
	}
}

bool UStylusCreator::FindDisplay() {
	TArray<UDisplay*, TInlineAllocator<1>> displays;
	GetOwner()->GetComponents(displays);

	const auto displaysCount = displays.Num();

	if (displaysCount == 1) {
		_display = displays[0];
		_display->OnDisplayReady.AddUniqueDynamic(this, &UStylusCreator::OnDeviceNetworkChanged);
		return true;
	}

	if (displaysCount == 0) {
		DISPLAYSTYLUS_LOG(Warning, TEXT("UStylusCreator::FindDisplay: Display Component must be set as a child of the DisplayHandle"))
		return false;
	}
	
	DISPLAYSTYLUS_LOG(Warning, TEXT("UStylusCreator::FindDisplay: Found multiple Display components, only one is expected"))
	return false;
}

