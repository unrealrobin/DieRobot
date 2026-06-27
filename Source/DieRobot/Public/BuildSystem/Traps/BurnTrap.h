// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "TrapBase.h"
#include "BurnTrap.generated.h"

class UNiagaraComponent;

UCLASS()
class DIEROBOT_API ABurnTrap : public ATrapBase
{
	GENERATED_BODY()

public:

	// Sets default values for this actor's properties
	ABurnTrap();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* BurnTrapInternalsMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	float InternalsSpinSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	float InternalsSpinTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UNiagaraComponent* BurnTrapInternalsNiagara = nullptr;

	//The amount of time to wait before checking if need to Deactivate Niagara System.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float NiagaraCheckTimer = 3.0f;

protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleTrapBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleTrapEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 INT32);

private:

	//Checks if Enemies in Hitbox Array is Empty and Deactivates Niagara if its Still Active.
	void CheckNiagaraActive();
};
