#include "Display.h"

#include "DisplayStylusLog.h"

#include <AntilatencyDeviceNetwork/Public/DeviceNetworkConstants.h>
#include <AntilatencyAltTracking/Public/MathLibrary.h>

#include "Kismet/GameplayStatics.h"


UDisplay::UDisplay() {
	PrimaryComponentTick.bCanEverTick = true;
}


void UDisplay::BeginPlay() {
	Super::BeginPlay();

	InitLibraries();

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

	_onDeviceNetworkChangedHandle = _deviceNetworkSubsystem->OnDeviceNetworkUpdated().AddUObject(this, &UDisplay::OnDeviceNetworkChanged);

	OnDeviceNetworkChanged();
}

// Called every frame
void UDisplay::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) {
	Super::TickComponent(deltaTime, tickType, thisTickFunction);
	
}

void UDisplay::EndPlay(const EEndPlayReason::Type endPlayReason) {
	if (_deviceNetworkSubsystem != nullptr && _onDeviceNetworkChangedHandle.IsValid()) {
		_deviceNetworkSubsystem->OnDeviceNetworkUpdated().Remove(_onDeviceNetworkChangedHandle);
	}

	_pceCotask = nullptr;
	_environment = nullptr;
	
	Super::EndPlay(endPlayReason);
}

bool UDisplay::GetDisplayProperties(FVector& screenPosition, FVector& screenX, FVector& screenY) const {
	if (_pceCotask == nullptr) {
		return false;
	}

	screenPosition = _screenPosition;
	screenX = _screenX;
	screenY = _screenY;

	return true;
}

bool UDisplay::GetEnvironment(UAltEnvironment*& environment) const {
	if (_environment == nullptr || !_environment->IsValid()) {
		return false;
	}
	
	environment = _environment;
	return true;
}

bool UDisplay::GetPhysicalDisplayRotation(FQuat& rotation) const {
	if (_environment == nullptr || !_environment->IsValid()) {
		return false;
	}

	if (_rectangleLibrary == nullptr) {
		return false;
	}

	const auto exception = _rectangleLibrary->GetRotation(_environment, rotation);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDisplay::GetPhysicalDisplayRotation: failed to get display rotation"));
		return false;
	}

	// TODO: remove after library update
	if (!rotation.IsNormalized()) {
		return false;
	}

	rotation = MathLibrary::AntilatencyQuatToUe(rotation);
	return true;
}

bool UDisplay::GetHalfScreenSize(FVector2D& result) const {
	if (_pceCotask == nullptr) {
		return false;
	}

	result = {_screenX.Size(), _screenY.Size()};
	return true;
}

void UDisplay::OnDeviceNetworkChanged() {
	if (_pceCotask != nullptr && _pceCotask->IsValid()) {
		bool taskFinished;
		auto exception = _pceCotask->IsTaskFinished(taskFinished);
		if (exception == Antilatency::InterfaceContract::ExceptionCode::Ok && !taskFinished) {
			return;
		} else {
			_environment = nullptr;
			_pceCotask = nullptr;
		}
	}

	FAdnNode pceNode;
	if (!FindPCENode(pceNode)) {
		return;
	}

	auto exception = _pceCotaskConstructor->StartTask(_deviceNetworkSubsystem->GetDeviceNetwork(), pceNode, _pceCotask);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDisplay::OnDeviceNetworkChanged: failed to start PCE cotask"));
		_pceCotask = nullptr;
		return;
	}

	exception = _pceCotask->GetScreenPosition(_screenPosition);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDisplay::OnDeviceNetworkChanged: failed to get screen position"));
		return;
	}
	_screenPosition = MathLibrary::AntilatencyVectorToUe(_screenPosition);

	exception = _pceCotask->GetScreenX(_screenX);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDisplay::OnDeviceNetworkChanged: failed to get screen X"));
		return;
	}
	_screenX = MathLibrary::AntilatencyVectorToUe(_screenX);

	exception = _pceCotask->GetScreenY(_screenY);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDisplay::OnDeviceNetworkChanged: failed to get screen Y"));
		return;
	}
	_screenY = MathLibrary::AntilatencyVectorToUe(_screenY);

	uint32_t configId;
	exception = _pceCotask->GetConfigId(configId);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDisplay::OnDeviceNetworkChanged: failed to get config ID"));
		return;
	}

	FString environmentCode;
	exception = _pceCotask->GetEnvironment(configId, environmentCode);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDisplay::OnDeviceNetworkChanged: failed to get environment code"));
		return;
	}

	if (_environmentSelectorLibrary == nullptr) {
		return;
	}

	exception = _environmentSelectorLibrary->CreateEnvironment(environmentCode, _environment);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDisplay::OnDeviceNetworkChanged: failed to create environment"));
		return;
	}

	_displayReadyEvent.Broadcast();

	UpdateDisplayMesh();
}

bool UDisplay::InitLibraries() {
	_pceLibrary = UPhysicalConfigurableEnvironmentLibrary::GetLibrary();
	if (_pceLibrary == nullptr) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDisplay: Failed to get UPhysicalConfigurableEnvironmentLibrary"));
		return false;
	}
	
	auto exception = _pceLibrary->CreateCotaskConstructor(_pceCotaskConstructor);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok)	{
		DISPLAYSTYLUS_LOG(Error, TEXT("UDisplay: Failed to create UPhysicalConfigurableEnvironmentCotaskConstructor"));
		return false;
	}

	_environmentSelectorLibrary = UEnvironmentSelectorLibrary::GetLibrary();
	if (_environmentSelectorLibrary == nullptr) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDisplay: Failed to get UEnvironmentSelectorLibrary"));
		return false;
	}

	_rectangleLibrary = UAltEnvironmentRectangleLibrary::GetLibrary();
	if (_rectangleLibrary == nullptr) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDisplay: Failed to get UAltEnvironmentRectangleLibrary"));
		return false;
	}

	return true;
}

bool UDisplay::FindPCENode(FAdnNode& result) const {
	if (_pceCotaskConstructor == nullptr || !_pceCotaskConstructor->IsValid()) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDisplay::FindPCENode: PCE cotask constructor is nullptr"));
		return false;
	}

	auto deviceNetwork = _deviceNetworkSubsystem->GetDeviceNetwork();

	TArray<FAdnNode> pceNodes;
	auto exception = _pceCotaskConstructor->FindSupportedNodes(deviceNetwork, pceNodes);
	if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
		DISPLAYSTYLUS_LOG(Error, TEXT("UDisplay::FindPCENode: failed to get PCE nodes"));
		return false;
	}

	for (auto node : pceNodes) {
		ENodeStatus status;
		exception = deviceNetwork->NodeGetStatus(node, status);
		if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
			continue;
		}

		if (status != ENodeStatus::Idle) {
			continue;
		}

		FString hardwareName;
		exception = deviceNetwork->NodeGetStringProperty(node, UDeviceNetworkConstants::HardwareNameKey, hardwareName);
		if (exception != Antilatency::InterfaceContract::ExceptionCode::Ok) {
			continue;
		}

		if (hardwareName != _displayNodeHardwareName) {
			continue;
		}

		result = node;
		
		return true;
	}

	return false;
}

bool UDisplay::FindDisplayMeshComponent() {
	TArray<UProceduralMeshComponent*, TInlineAllocator<1>> proceduralMeshes;
	GetOwner()->GetComponents(proceduralMeshes);

	for (auto component : proceduralMeshes) {
		auto componentParent = component->GetAttachParent();
		if (componentParent == this) {
			_displayMesh = component;
			return true;
		}
	}

	return false;
}

bool UDisplay::UpdateDisplayMesh() {
	if (BorderMaterial == nullptr) {
		return false;
	}

	if (_displayMesh == nullptr) {
		if (!FindDisplayMeshComponent()) {
			return false;
		}
	}

	FVector2D halfScreenSize;
	if (!GetHalfScreenSize(halfScreenSize)) {
		return false;
	}

	DISPLAYSTYLUS_LOG(Warning, TEXT("HSS: %f, %f, SP: %f, %f, %f"), halfScreenSize.X, halfScreenSize.Y, _screenPosition.X, _screenPosition.Y, _screenPosition.Z);
	
	_displayMesh->ClearAllMeshSections();

	FVector innerLeftBottom (0.0, -halfScreenSize.X, -halfScreenSize.Y);
	FVector innerLeftTop (0.0, -halfScreenSize.X, halfScreenSize.Y);
	FVector innerRightTop (0.0, halfScreenSize.X, halfScreenSize.Y);
	FVector innerRightBottom (0.0, halfScreenSize.X, -halfScreenSize.Y);

	innerLeftBottom += _screenPosition;
	innerLeftTop += _screenPosition;
	innerRightTop += _screenPosition;
	innerRightBottom += _screenPosition;

	FVector outerLeftBottom (innerLeftBottom.X, innerLeftBottom.Y - BorderWidth, innerLeftBottom.Z - BorderWidth);
	FVector outerLeftTop (innerLeftTop.X, innerLeftTop.Y - BorderWidth, innerLeftTop.Z + BorderWidth);
	FVector outerRightTop (innerRightTop.X, innerRightTop.Y + BorderWidth, innerRightTop.Z + BorderWidth);
	FVector outerRightBottom (innerRightBottom.X, innerRightBottom.Y + BorderWidth, innerRightBottom.Z - BorderWidth);

	TArray<FVector> verts = {
		innerLeftBottom, innerLeftTop, innerRightTop, innerRightBottom, outerLeftBottom, outerLeftTop, outerRightTop, outerRightBottom
	};

	TArray<int32> tris = {
		0, 1, 4,
		4, 1, 5,
		1, 2, 6,
		1, 6, 5,
		2, 3, 6,
		6, 3, 7,
		3, 0, 4,
		7, 3, 4
	};

	TArray<FVector> emptyVectorArray;
	TArray<FVector2D> emptyVector2dArray;
	TArray<FColor> emptyColorArray;
	TArray<FProcMeshTangent> emptyTangentsArray;

	_displayMesh->CreateMeshSection(0, verts, tris, emptyVectorArray, emptyVector2dArray, emptyColorArray, emptyTangentsArray, false);
	_displayMesh->SetMaterial(0, BorderMaterial);

	return true;
}
