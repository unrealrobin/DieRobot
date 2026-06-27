// Property of Paracosm Industries.

#pragma once

#include "CoreMinimal.h"
#include "TrapBase.h"
#include "FrostTrap.generated.h"

class UNiagaraComponent;

UCLASS()
class DIEROBOT_API AFrostTrap : public ATrapBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFrostTrap();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* FrostTrapFanMesh = nullptr;

	//The Amount of degrees to Spin over Time.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	float FanSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UNiagaraComponent* FrostTrapNiagaraComponent = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void HandleFrostTrapBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
	

};
