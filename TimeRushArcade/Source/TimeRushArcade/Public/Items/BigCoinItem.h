// BigCoinItem.h
#pragma once

#include "CoreMinimal.h"
#include "Items/CoinItem.h"
#include "BigCoinItem.generated.h"


UCLASS()
class TIMERUSHARCADE_API ABigCoinItem : public ACoinItem
{
	GENERATED_BODY()

public:
	ABigCoinItem();

	virtual void ActivateItem(AActor* Activator) override;
};
