// Property of Paracosm.

#pragma once

#include "CoreMinimal.h"
#include "TrapBase.h"
#include "CorrosionTrap.generated.h"

UCLASS()
class DIEROBOT_API ACorrosionTrap : public ATrapBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACorrosionTrap();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMeshComponent* CorrosionTrapMeshFront = nullptr;


protected:
	UFUNCTION()
	void HandleFrostTrapBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,const FHitResult& SweepResult);	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
