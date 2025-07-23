//TRAGameState.cpp
#include "TRAGameState.h"

#include "Core/TRAGameInstance.h"
#include "Controllers/TRAPlayerController.h"
#include "Items/SpawnVolume.h"
#include "Items/CoinItem.h"

#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "EngineUtils.h" // TActorIterator용
#include "UI/TRARoundProgressBar.h"

ATRAGameState::ATRAGameState()
{
	Score = 0;
	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;
	WaveDuration = 20.0f;
	CurrentWaveIndex = 0;
	WavesPerLevel = 3;
	MaxLevelCount = 3;
	MaxWaveCount = WavesPerLevel * MaxLevelCount;
}

void ATRAGameState::BeginPlay()
{
	Super::BeginPlay();
	StartWave();//StartLevel();

	GetWorldTimerManager().SetTimer(
		HUDUpdateTimerHandle,
		this,
		&ATRAGameState::UpdateHUD,
		0.1f,
		true
	);
}

int32 ATRAGameState::GetScore() const
{
	return Score;
}

void ATRAGameState::AddScore(int32 Amount)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UTRAGameInstance* TRAGameInstance = Cast<UTRAGameInstance>(GameInstance);
		if (TRAGameInstance)
		{
			TRAGameInstance->AddToScore(Amount);
		}
	}
}

void ATRAGameState::StartWave()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ATRAPlayerController* TRAPlayerController = Cast<ATRAPlayerController>(PlayerController))
		{
			TRAPlayerController->ShowGameHUD();
		}
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UTRAGameInstance* TRAGameInstance = Cast<UTRAGameInstance>(GameInstance);
		if (TRAGameInstance)
		{
			CurrentWaveIndex = TRAGameInstance->CurrentWaveIndex;
		}
	}

	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;

	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);

	const int32 ItemToSpawn = 20 + CurrentWaveIndex * 10; // ex: 20, 30, 40
	//const int32 ItemToSpawn = 40;

	if (FoundVolumes.Num() > 0)
	{
		ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]);
		if (SpawnVolume)
		{
			for (int32 i = 0; i < ItemToSpawn; i++)
			{
				AActor* SpawnedActor = SpawnVolume->SpawnRandomItem();
				if (SpawnedActor && SpawnedActor->IsA(ACoinItem::StaticClass()))
				{
					SpawnedCoinCount++;
				}
			}
		}
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
		FString::Printf(TEXT("Wave %d 시작!"), (CurrentWaveIndex % WavesPerLevel) + 1));

	GetWorldTimerManager().SetTimer(
		LevelTimerHandle,
		this,
		&ATRAGameState::OnLevelTimeUp,
		WaveDuration,
		false
	);

	//UE_LOG(LogTemp, Warning, TEXT("Level %d Start!, Spawned %d coin"),
	//	CurrentWaveIndex + 1,
	//	SpawnedCoinCount);
}

void ATRAGameState::OnLevelTimeUp()
{
	EndWave();
}

void ATRAGameState::OnCoinCollected()
{
	CollectedCoinCount++;
	UE_LOG(LogTemp, Warning, TEXT("Coin Colllected: %d / %d"),
		CollectedCoinCount, SpawnedCoinCount);

	if ((SpawnedCoinCount > 0) && (CollectedCoinCount >= SpawnedCoinCount))
	{
		EndWave();
	}
}

void ATRAGameState::EndWave()
{
	GetWorldTimerManager().ClearTimer(LevelTimerHandle);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UTRAGameInstance* TRAGameInstance = Cast<UTRAGameInstance>(GameInstance);
		if (TRAGameInstance)
		{
			AddScore(Score);
			CurrentWaveIndex++;
			TRAGameInstance->CurrentWaveIndex = CurrentWaveIndex;
		}
	}

	if (CurrentWaveIndex >= MaxWaveCount)
	{
		OnGameOver();
		return;
	}

	//if (LevelMapNames.IsValidIndex(CurrentWaveIndex))
	//{
	//	UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentWaveIndex]);
	//}
	//else
	//{
	//	OnGameOver();
	//}
	
	// 레벨 종료 시 맵 전환
	if (CurrentWaveIndex % WavesPerLevel == 0)
	{
		int32 NextLevelIndex = CurrentWaveIndex / WavesPerLevel;

		if (LevelMapNames.IsValidIndex(NextLevelIndex))
		{
			UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[NextLevelIndex]);
			return; // 맵 전환되므로 아래 StartWave는 실행하지 않음
		}
		else
		{
			OnGameOver();
			return;
		}
	}

	// 다음 웨이브 시작
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
		FString::Printf(TEXT("Wave %d 완료! 다음 웨이브 시작 준비 중..."), (CurrentWaveIndex % WavesPerLevel) + 1));

	// 잠시 후 다음 웨이브 시작
	GetWorldTimerManager().SetTimerForNextTick(this, &ATRAGameState::StartWave);
}

void ATRAGameState::OnGameOver()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ATRAPlayerController* TRAPlayerController = Cast<ATRAPlayerController>(PlayerController))
		{
			TRAPlayerController->SetPause(true);
			TRAPlayerController->ShowMainMenu(true);
		}
	}
}

void ATRAGameState::UpdateHUD()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ATRAPlayerController* TRAPlayerController = Cast<ATRAPlayerController>(PlayerController))
		{
			if (UUserWidget* HUDWidget = TRAPlayerController->GetHUDWidget())
			{
				if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time"))))
				{
					float RemainingTime = GetWorldTimerManager().GetTimerRemaining(LevelTimerHandle);
					TimeText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), RemainingTime)));
				}

				if (UTextBlock* ScoreText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Score"))))
				{
					if (UGameInstance* GameInstance = GetGameInstance())
					{
						UTRAGameInstance* TRAGameInstance = Cast<UTRAGameInstance>(GameInstance);
						if (TRAGameInstance)
						{
							ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), TRAGameInstance->TotalScore)));
						}
					}
				}

				if (UTextBlock* LevelIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Level"))))
				{
					//LevelIndexText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d"), CurrentWaveIndex + 1)));
					int32 CurrentLevel = (CurrentWaveIndex / WavesPerLevel) + 1;
					int32 CurrentWaveInLevel = (CurrentWaveIndex % WavesPerLevel) + 1;

					LevelIndexText->SetText(FText::FromString(FString::Printf(
						TEXT("Level: %d\nWave: %d"),
						CurrentLevel,
						CurrentWaveInLevel
					)));
				}

				// --- RoundProgressBar 업데이트 ---
				if (UWidget* RoundProgressWidget = HUDWidget->GetWidgetFromName(TEXT("WBP_RoundProgressBar")))
				{
					if (UTRARoundProgressBar* RoundProgressBar = Cast<UTRARoundProgressBar>(RoundProgressWidget))
					{
						float RemainingTime = GetWorldTimerManager().GetTimerRemaining(LevelTimerHandle);
						float TotalTime = WaveDuration;

						float ProgressPercent = 0.f;
						if (TotalTime > 0.f)
						{
							ProgressPercent = RemainingTime / TotalTime;
							ProgressPercent = FMath::Clamp(ProgressPercent, 0.f, 1.f);

							// 필요시 반전
							// ProgressPercent = 1.f - ProgressPercent;
						}

						UE_LOG(LogTemp, Warning, TEXT("SetProgressPercent Called - %.2f"), ProgressPercent);

						RoundProgressBar->SetProgressPercent(ProgressPercent);
					}
				}
			}
		}
	}
}