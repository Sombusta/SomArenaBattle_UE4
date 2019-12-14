// Copyright (c) 2014-2019 Sombusta, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SomABPlayerController.generated.h"

UCLASS()
class SOMARENABATTLE_API ASomABPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ASomABPlayerController();
	
	// Called when initialize complete Session
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when the game ends or when quit
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Allows the PlayerController to set up custom input bindings
	virtual void SetupInputComponent() override;

	// Called when this controller possesses a pawn
	virtual void OnPossess(APawn* InPawn) override;

	void NPCKill(class ASomAB_TPCharacter* KilledNPC) const;

	void AddGameScore() const;

private:
	UPROPERTY()
	class USomABHUDWidget* HUDWidget;

	UPROPERTY()
	class ASomABPlayerState* SomABPlayerState;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SomABWorks|Variables", meta = (BlueprintProtected = "true"))
	TSubclassOf<class USomABHUDWidget> HUDWidgetClass;
	
public:
	FORCEINLINE class USomABHUDWidget* GetHUDWidget() const { return HUDWidget; }
};