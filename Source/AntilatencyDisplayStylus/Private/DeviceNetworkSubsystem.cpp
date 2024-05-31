// Fill out your copyright notice in the Description page of Project Settings.

#include "DeviceNetworkSubsystem.h"
#include "DisplayStylusLog.h"

#include <DeviceNetworkConstants.h>

void UDeviceNetworkSubsystem::Initialize(FSubsystemCollectionBase& collection) {
	Super::Initialize(collection);

	DISPLAYSTYLUS_LOG(Log, TEXT("UDeviceManagerSubsystem::Initialize"));

	_deviceNetworkLibrary = UDeviceNetworkLibrary::GetLibrary();
	if (_deviceNetworkLibrary == nullptr || !_deviceNetworkLibrary->IsValid()) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDeviceManagerSubsystem::Initialize: failed to get Device Network library"));
		return;
	}

	// _onWorldAddedDelegate = GEngine->OnWorldAdded().AddUObject(this, &UDeviceNetworkSubsystem::OnWorldAdded);
	// _onWorldDestroyedDelegate = GEngine->OnWorldDestroyed().AddUObject(this, &UDeviceNetworkSubsystem::OnWorldDestroyed);
}

void UDeviceNetworkSubsystem::Deinitialize() {
	Super::Deinitialize();

	StopDeviceNetwork();

	DISPLAYSTYLUS_LOG(Log, TEXT("UDeviceManagerSubsystem::Deinitialize"));
}

void UDeviceNetworkSubsystem::Tick(float deltaTime) {
	if (_deviceNetwork == nullptr) {
		return;
	}

	uint32 updateId;
	auto exceptionCode = _deviceNetwork->GetUpdateId(updateId);
	if (exceptionCode != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDeviceManagerSubsystem::Tick: failed to get update id"));
		return;
	}

	if (updateId == _lastUpdateId) {
		return;
	}

	_lastUpdateId = updateId;

	_deviceNetworkUpdatedEvent.Broadcast();
}

bool UDeviceNetworkSubsystem::StartDeviceNetwork(UDeviceFilter* deviceFilter) {
	if (_deviceNetworkLibrary == nullptr) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDeviceManagerSubsystem::StartDeviceNetwork: Device Network library is null"));
		return false;
	}

	if (_deviceNetwork != nullptr) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDeviceManagerSubsystem::StartDeviceNetwork: Device Network already initialized"));
		return false;
	}

	if (deviceFilter == nullptr) {
		DISPLAYSTYLUS_LOG(Log, TEXT("UDeviceManagerSubsystem::StartDeviceNetwork: initialize ADN with default device filter"));

		auto exceptionCode = _deviceNetworkLibrary->CreateFilter(deviceFilter);
		if (exceptionCode != Antilatency::InterfaceContract::ExceptionCode::Ok) {
			DISPLAYSTYLUS_LOG(Log, TEXT("UDeviceManagerSubsystem::StartDeviceNetwork: failed to create default device filter"));
			return false;
		}

		const auto allUsbDevices = UDeviceNetworkConstants::AllUsbDevices;
		exceptionCode = deviceFilter->AddUsbDevice(allUsbDevices);
		if (exceptionCode != Antilatency::InterfaceContract::ExceptionCode::Ok) {
			DISPLAYSTYLUS_LOG(Log, TEXT("UDeviceManagerSubsystem::StartDeviceNetwork: failed to add usb device filter"));
			return false;
		}
	}

	const auto exceptionCode = _deviceNetworkLibrary->CreateNetwork(deviceFilter, _deviceNetwork);
	if (exceptionCode != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDeviceManagerSubsystem::StartDeviceNetwork: failed to create device network"));
		return false;
	}

	return true;
}

bool UDeviceNetworkSubsystem::IsRunning() const {
	return _deviceNetwork != nullptr && _deviceNetwork->IsValid();
}

UDeviceNetwork* UDeviceNetworkSubsystem::GetDeviceNetwork() const {
	return _deviceNetwork;
}

void UDeviceNetworkSubsystem::StopDeviceNetwork() {
	if (_deviceNetwork == nullptr) {
		return;
	}

	_deviceNetwork = nullptr;
}
