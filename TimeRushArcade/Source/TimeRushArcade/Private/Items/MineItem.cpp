// MineItem.cpp
#include "MineItem.h"

#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AMineItem::AMineItem()
{
	ExplosionDelay = 5.0f;		// 지뢰가 폭발하기까지의 지연 시간
	ExposionRadius = 300.0f;	// 폭발 반경
	ExplosionDamage = 30.0f;	// 폭발 피해량
	ItemType = "Mine";
	bHasExploded = false;

	ExplosionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ExplosionCollision"));
	ExplosionCollision->InitSphereRadius(ExposionRadius);
	ExplosionCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	ExplosionCollision->SetupAttachment(Scene);
}

void AMineItem::ActivateItem(AActor* Activator)
{
	if (bHasExploded) return;

	// 별도 추가 - 지뢰 Smoke 시간을 연장하고 싶어서 만든 변수
	// 먼저 DestroyParticle 시간 설정
	ParticleDestroyDelay = 5.0f; // 지뢰는 5초로 설정

	Super::ActivateItem(Activator);

	// 지뢰가 활성화되면 폭발 지연 시간을 설정하고, 폭발을 예약
	// 타이머 핸들러
	GetWorld()->GetTimerManager().SetTimer(
		ExplosionTimerHandle,	// 타이머를 설정
		this,					// 이 객체의 메서드
		&AMineItem::Explode,	// 폭발 함수
		ExplosionDelay,			// 지연 시간
		false					// 반복하지 않음
	);

	bHasExploded = true;
}

void AMineItem::Explode()
{
	UParticleSystemComponent* Particle = nullptr;

	if (ExplosionParticle)
	{
		Particle = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ExplosionParticle,
			GetActorLocation(),
			GetActorRotation(),
			false
		);
	}

	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			ExplosionSound,
			GetActorLocation()
		);
	}

	TArray<AActor*> OverlappingActors; // 겹치는 액터를 저장할 배열
	ExplosionCollision->GetOverlappingActors(OverlappingActors); // 폭발 범위 내의 액터를 가져옴
	for (AActor* Actor : OverlappingActors)
	{// 폭발 범위 내의 액터가 플레이어인지 확인
		//if (Actor && Actor != Activator)
		if (Actor && Actor->ActorHasTag("Player"))
		{// 폭발 처리 로직
			UGameplayStatics::ApplyDamage(
				Actor,
				ExplosionDamage,
				nullptr,
				this,
				UDamageType::StaticClass()
			);
		}
	}

	DestroyItem();

	//if (Particle)
	//{
	//	FTimerHandle DestroyParticleTimerHandle;

	//	GetWorld()->GetTimerManager().SetTimer(
	//		DestroyParticleTimerHandle,
	//		[Particle]()
	//		{
	//			//Particle->DestroyComponent();
	//			if (IsValid(Particle))
	//			{
	//				Particle->DestroyComponent();
	//			}
	//		},
	//		2.0f,
	//		false
	//	);
	//}

	if (Particle)
	{
		TWeakObjectPtr<UParticleSystemComponent> WeakParticle = Particle;

		FTimerHandle DestroyParticleTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			DestroyParticleTimerHandle,
			[WeakParticle]()
			{
				if (WeakParticle.IsValid())
				{
					WeakParticle->DestroyComponent();
				}
			},
			2.0f,
			false
		);
	}
}
