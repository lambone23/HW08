// MineItem.h
#pragma once

#include "CoreMinimal.h"
#include "Items/BaseItem.h"
#include "MineItem.generated.h"

UCLASS()
class TIMERUSHARCADE_API AMineItem : public ABaseItem
{
	GENERATED_BODY()

public:
	AMineItem();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Component");
	USphereComponent* ExplosionCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effects")
	UParticleSystem* ExplosionParticle;	// 폭발 파티클
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effects")
	USoundBase* ExplosionSound;			// 폭발 사운드

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float ExplosionDelay;				// 지뢰가 폭발하기까지의 지연 시간

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float ExposionRadius;				// 폭발 반경

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 ExplosionDamage;				// 폭발 피해량

	bool bHasExploded;
	FTimerHandle ExplosionTimerHandle;	// 폭발 타이머 핸들

	virtual void ActivateItem(AActor* Activator) override;
	void Explode();						// 폭발 처리 함수
};
