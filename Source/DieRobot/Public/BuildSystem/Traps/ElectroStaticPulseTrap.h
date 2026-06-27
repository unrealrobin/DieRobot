// Property of Paracosm Industries. 
#pragma once

#include "CoreMinimal.h"
#include "PlusTrapBase.h"
#include "ElectroStaticPulseTrap.generated.h"

class USphereComponent;

USTRUCT(BlueprintType)
struct FElectroPulseSphereData
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> SphereCapsule = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> SphereMesh = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGuid ElectroPulseSphereGuid;
	
	bool operator==(const FElectroPulseSphereData& Other) const
	{
		return ElectroPulseSphereGuid == Other.ElectroPulseSphereGuid;
	}
};


/*
 *Shoots A Web of Electro Static Energy that Damages and Leaves a Damage over Time effect on Enemy.
 *Slow Cooldown - 10 sec?
 */



UCLASS()
class DIEROBOT_API AElectroStaticPulseTrap : public APlusTrapBase
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AElectroStaticPulseTrap();

	//Interface Override
	virtual void SetIsAmplified(bool bIsAmplified) override;
	
	//Standard Electro Pulse Firing Loop Timer Handle
	FTimerHandle ElectroPulseFiringCooldownTimer;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void CreatePulseHitSphere();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TArray<FElectroPulseSphereData> ElectroPulseSphereInstances;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

	void MovePulseHitSphere(float DeltaTime);

	UFUNCTION()
	void FireElectroPulse();
	
	UFUNCTION()
	void HandlePulseSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float InitialHitDamage = 10.f;
	
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UBoxComponent* PulseHitBox = nullptr;*/
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* PulseSphereHitComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* PulseSphereMeshComponent = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	float SphereRadius = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMesh* PulseSphereMesh = nullptr;
	
private:
	
	void SetupElectroPulseLoopTimer(float InRate);
	
	//void DestroyPulseSphere();
	
	void DestroySphereComponents(FGuid InSphereGuid);
	
	void AddNewSphereToPulseArray(FElectroPulseSphereData NewSphereData);
	
	TArray<FGuid> DestroyedSphereGuids;
	
};
