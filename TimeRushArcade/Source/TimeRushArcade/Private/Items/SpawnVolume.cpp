// SpawnVolume.cpp
#include "SpawnVolume.h"

#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

ASpawnVolume::ASpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	SpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
	SpawningBox->SetupAttachment(Scene);

	ItemDataTable = nullptr; // 아이템 데이터 테이블 초기화
}

AActor* ASpawnVolume::SpawnRandomItem()
{
	if (FItemSpawnRow* SelectedRow = GetRandomItem())
	{// 어떤 아이템이 뽑혔을 때 최종 선택된 아이템을 가져온다.
		//if(UClass* ActualClass = SelectedRow->ItemClass.Get())
		if (UClass* ActualClass = SelectedRow->ItemClass) //TODO: .Get() 빼기
		{
			/*
				ItemClass.Get()
				ItemClass라는 컬럼을 Get() 함수를 통해 가져오면
				TsoftclassPtr 타입으로 가져오게 되어 UClass로 반환
				//TODO: TSubclassOf<AActor> 타입인데 문제 없나?
			*/
			return SpawnItem(ActualClass);
		}
	}

	return nullptr;
}

FItemSpawnRow* ASpawnVolume::GetRandomItem() const
{
	/*
		데이터 테이블에서 랜덤으로 아이템을 선택하는 함수
		ItemDataTable은 UDataTable 타입의 변수로, 아이템 데이터가 저장된 데이터 테이블을 참조한다.
	*/

	// 데이터 테이블이 없거나 비어있으면, 이 함수는 행을 리턴해야 하므로 반환값이 필수이므로 nullptr 반환
	if (!ItemDataTable) return nullptr;

	// 데이터 테이블에서 모든 행을 가져오기 위한 배열
	TArray<FItemSpawnRow*> AllRows;

	/*
		GetAllRows 함수는 데이터 테이블의 모든 행을 가져오는 함수로,
		첫 번째 인자는 컨텍스트 문자열로, 디버깅 시 어떤 데이터 테이블에서 가져온 것인지 알 수 있게 해준다.
		두 번째 인자는 가져온 행들을 저장할 배열이다.
		컨텍스트 문자열은 디버깅을 위해 사용되며, 데이터 테이블의 이름을 명시적으로 지정하는 것이 좋다.
		이 예제에서는 "ItemSpawnContext"라는 문자열을 사용한다.
	*/
	static const FString ContextString(TEXT("ItemSpawnContext"));
	ItemDataTable->GetAllRows(ContextString, AllRows);

	// 데이터 테이블에서 행이 없으면 nullptr 반환
	if (AllRows.IsEmpty()) return nullptr;

	// 아이템의 총 확률을 계산하기 위한 변수
	float Totalchance = 0.0f;

	// 모든 행을 순회하면서 확률을 합산
	for (const FItemSpawnRow* Row : AllRows)
	{
		if (Row && Row->SpawnChance > 0.0f)
		{
			Totalchance += Row->SpawnChance;
		}
	}

	// 총 확률값이 적용된 범위 내에서 랜덤 값을 생성
	const float RandomValue = FMath::FRandRange(0.0f, Totalchance);

	// 누적 확률을 초기화
	float AccumulatedChance = 0.0f;

	// 랜덤 값을 기반으로 아이템을 선택하는 과정
	for (FItemSpawnRow* Row : AllRows)
	{// 각 행을 순회하면서
		if (Row && Row->SpawnChance > 0.0f)
		{// 각 행의 SpawnChance가 0보다 큰 경우에만 처리
			// 현재 행의 확률을 누적 확률에 더한다.
			AccumulatedChance += Row->SpawnChance;

			if (RandomValue <= AccumulatedChance)
			{// 생성된 랜덤 값이 누적 확률보다 작거나 같으면 해당 행을 선택
				return Row;  // 선택된 아이템 반환
			}
		}
	}

	// 예외적으로 여기에 도달하면 nullptr (정상적이라면 도달하지 않음)
	return nullptr;
}

FVector ASpawnVolume::GetRandomPointInVolume() const
{
	FVector BoxExtent = SpawningBox->GetScaledBoxExtent();		// Box의 반지름
	FVector BoxOrigin = SpawningBox->GetComponentLocation();	// Box의 중심 위치

	return BoxOrigin + FVector(
		FMath::RandRange(-BoxExtent.X, BoxExtent.X),
		FMath::RandRange(-BoxExtent.Y, BoxExtent.Y),
		FMath::RandRange(-BoxExtent.Z, BoxExtent.Z)
	);
}

AActor* ASpawnVolume::SpawnItem(TSubclassOf<AActor> ItemClass)
{
	if (!ItemClass) return nullptr;

	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
		ItemClass,
		GetRandomPointInVolume(),
		FRotator::ZeroRotator
	);

	return SpawnedActor;
}
