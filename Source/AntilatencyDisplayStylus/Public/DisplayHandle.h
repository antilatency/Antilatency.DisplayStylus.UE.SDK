#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "Display.h"

#include "DisplayHandle.generated.h"

UENUM(BlueprintType)
enum class EScaleMode : uint8 {
	RealSize,
	WidthIsOne,
	HeightIsOne
};

UCLASS( ClassGroup=(Antilatency), meta=(BlueprintSpawnableComponent) )
class ANTILATENCYDISPLAYSTYLUS_API UDisplayHandle : public USceneComponent {
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDisplayHandle();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = -1, ClampMax = 1, UIMin = -1, UIMax = 1))
	int OriginX = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ClampMin = -1, ClampMax = 1, UIMin = -1, UIMax = 1))
	int OriginY = -1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EScaleMode ScaleMode = EScaleMode::RealSize;

protected:
	UPROPERTY()
	UDisplay* _display = nullptr;

	// Self interface
protected:
	bool FindDisplay();
	
	// UActorComponent interface
public:	
	virtual void TickComponent(float deltaTime, ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
};
