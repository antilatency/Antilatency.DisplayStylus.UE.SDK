#include "DisplayHandle.h"

#include "DisplayStylusLog.h"


UDisplayHandle::UDisplayHandle() {
	PrimaryComponentTick.bCanEverTick = true;
}

void UDisplayHandle::BeginPlay() {
	Super::BeginPlay();

	FindDisplay();
}

void UDisplayHandle::TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) {
	Super::TickComponent(deltaTime, tickType, thisTickFunction);

	if (_display == nullptr) {
		if (!FindDisplay()) {
			return;
		}
	}

	FVector screenPosition, screenX, screenY;
	if (!_display->GetDisplayProperties(screenPosition, screenX, screenY)) {
		return;
	}

	auto position = -screenPosition - screenX * OriginX - screenY * OriginY;
	
	auto scale = FVector::OneVector;
	
	if (ScaleMode == EScaleMode::WidthIsOne) {
		scale /= screenX.Size();
	} else if (ScaleMode == EScaleMode::HeightIsOne) {
		scale /= screenY.Size();
	}

	_display->SetRelativeScale3D(scale);
	_display->SetRelativeLocation(FVector(position.X * scale.X, position.Y * scale.Y, position.Z * scale.Z));

	if (_display->SyncWithPhysicalDisplayRotation) {
		FQuat displayRotation;
		if (_display->GetPhysicalDisplayRotation(displayRotation)) {
			_display->SetRelativeRotation(displayRotation);
			return;
		}
	}

	_display->SetRelativeRotation(FQuat::Identity);
}

void UDisplayHandle::EndPlay(const EEndPlayReason::Type endPlayReason) {
	Super::EndPlay(endPlayReason);
}

bool UDisplayHandle::FindDisplay() {
	TArray<UDisplay*, TInlineAllocator<1>> displays;
	GetOwner()->GetComponents(displays);

	const auto displaysCount = displays.Num();

	if (displaysCount == 1) {
		_display = displays[0];
		return true;
	}

	if (displaysCount == 0) {
		DISPLAYSTYLUS_LOG(Warning, TEXT("UDisplayHandle::FindDisplay: Display Component must be set as a child of the DisplayHandle"))
		return false;
	}
	
	DISPLAYSTYLUS_LOG(Warning, TEXT("UDisplayHandle::FindDisplay: Found multiple Display components, only one is expected"))
	return false;
}