// Copyright (c) 2014-2019 Sombusta, All Rights Reserved.

#include "SomAB_TPCharacter.h"
#include "SomArenaBattle.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimInstance.h"

ASomAB_TPCharacter::ASomAB_TPCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_CardBoardMan(TEXT("SkeletalMesh'/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_Cardboard.SK_CharM_Cardboard'"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBP_Warrior(TEXT("AnimBlueprint'/Game/Book/Animations/AnimBP_Warrior.AnimBP_Warrior_C'"));

	MainCameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("MainCameraArm"));
	MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Set mesh location and rotation
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
	if (SK_CardBoardMan.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SK_CardBoardMan.Object);
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		if (AnimBP_Warrior.Succeeded()) {
			GetMesh()->SetAnimInstanceClass(AnimBP_Warrior.Class);
		}
	}

	// Create a camera boom (pulls in towards the player if there is a collision)
	MainCameraArm->SetupAttachment(RootComponent);
	MainCameraArm->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	MainCameraArm->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera	
	MainCamera->SetupAttachment(MainCameraArm, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	MainCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	
	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	SetControlMode(EABControlType::GTA);

	ArmLengthSpeed = 3.0f;
	ArmRotationSpeed = 10.0f;
}

void ASomAB_TPCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

}

void ASomAB_TPCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void ASomAB_TPCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

}

void ASomAB_TPCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CurrentControlType != EABControlType::None)
	{
		MainCameraArm->TargetArmLength = FMath::FInterpTo(MainCameraArm->TargetArmLength, ArmLengthTo, DeltaSeconds, ArmLengthSpeed);

		if (CurrentControlType == EABControlType::Diablo)
		{
			MainCameraArm->RelativeRotation = FMath::RInterpTo(MainCameraArm->RelativeRotation, ArmRotationTo, DeltaSeconds, ArmRotationSpeed);

			if (DirectionToMove.SizeSquared() > 0.0f)
			{
				GetController()->SetControlRotation(FRotationMatrix::MakeFromX(DirectionToMove).Rotator());
				AddMovementInput(DirectionToMove);
			}
		}
	}

}

//////////////////////////////////////////////////////////////////////////
// Input

void ASomAB_TPCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASomAB_TPCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASomAB_TPCharacter::MoveRight);

	PlayerInputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &ASomAB_TPCharacter::ViewChange);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	if (CurrentControlType != EABControlType::Diablo)
	{
		PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
		PlayerInputComponent->BindAxis("TurnRate", this, &ASomAB_TPCharacter::TurnAtRate);
		PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
		PlayerInputComponent->BindAxis("LookUpRate", this, &ASomAB_TPCharacter::LookUpAtRate);
	}
}

void ASomAB_TPCharacter::SetControlMode(EABControlType NewControlMode)
{
	CurrentControlType = NewControlMode;

	switch (CurrentControlType)
	{
	case EABControlType::None:
		break;
	case EABControlType::GTA:
		// MainCameraArm->TargetArmLength = 450.0f;
		// MainCameraArm->SetRelativeRotation(FRotator::ZeroRotator);
		ArmLengthTo = 450.0f;
		MainCameraArm->bUsePawnControlRotation = true;
		MainCameraArm->bInheritPitch = true;
		MainCameraArm->bInheritRoll = true;
		MainCameraArm->bInheritYaw = true;
		MainCameraArm->bDoCollisionTest = true;
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.f, 0.0f);
		break;
	case EABControlType::Diablo:
		// MainCameraArm->TargetArmLength = 800.0f;
		// MainCameraArm->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
		ArmLengthTo = 800.0f;
		ArmRotationTo = FRotator(-45.0f, 0.0f, 0.0f);
		MainCameraArm->bUsePawnControlRotation = false;
		MainCameraArm->bInheritPitch = false;
		MainCameraArm->bInheritRoll = false;
		MainCameraArm->bInheritYaw = false;
		MainCameraArm->bDoCollisionTest = false;
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.f, 0.0f);
		break;
	}
}

void ASomAB_TPCharacter::ViewChange()
{
	switch (CurrentControlType)
	{
	case EABControlType::None:
		SetControlMode(EABControlType::GTA);
		break;
	case EABControlType::GTA:
		GetController()->SetControlRotation(GetActorRotation());
		SetControlMode(EABControlType::Diablo);
		break;
	case EABControlType::Diablo:
		GetController()->SetControlRotation(MainCameraArm->RelativeRotation);
		SetControlMode(EABControlType::GTA);
		break;
	}
}

void ASomAB_TPCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASomAB_TPCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ASomAB_TPCharacter::MoveForward(float Value)
{
	if (CurrentControlType == EABControlType::Diablo)
	{
		DirectionToMove.X = Value;
	}
	else
	{
		if ((Controller != NULL) && (Value != 0.0f))
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, Value);
		}
	}	
}

void ASomAB_TPCharacter::MoveRight(float Value)
{
	if (CurrentControlType == EABControlType::Diablo)
	{
		DirectionToMove.Y = Value;
	}
	else
	{
		if ((Controller != NULL) && (Value != 0.0f))
		{
			// find out which way is right
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get right vector 
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			// add movement in that direction
			AddMovementInput(Direction, Value);
		}
	}
}
