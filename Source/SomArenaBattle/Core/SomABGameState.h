// Copyright (c) 2014-2019 Sombusta, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SomABGameState.generated.h"

UCLASS()
class SOMARENABATTLE_API ASomABGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	ASomABGameState();
	
	void AddGameScore();

private:
	UPROPERTY(Transient)
	int32 TotalGameScore;

public:
	FORCEINLINE int32 GetTotalGameScore() const { return TotalGameScore; }
};
