// TRAGameInstance.cpp
#include "TRAGameInstance.h"

UTRAGameInstance::UTRAGameInstance()
{
	TotalScore = 0;
	CurrentWaveIndex = 0;
}

void UTRAGameInstance::AddToScore(int32 Amount)
{
	TotalScore += Amount;
	UE_LOG(LogTemp, Warning, TEXT("Total Score Updated: %d"), TotalScore);
}
