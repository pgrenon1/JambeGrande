// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnferCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Public/JGChunk.h"
#include "Public/JGLevelGenerator.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AEnferCharacter

AEnferCharacter::AEnferCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh);
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

	// SetGeneratorAndIndex chunk tracking
	CurrentChunkIndex = 0;
	LevelGenerator = nullptr;
}

void AEnferCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Set up overlap events for chunk detection
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AEnferCharacter::OnChunkOverlapBegin);

	// Get the level generator from the GameMode
	if (AActor* gameMode = GetWorld()->GetAuthGameMode())
	{
		LevelGenerator = Cast<UJGLevelGenerator>(gameMode->GetComponentByClass(UJGLevelGenerator::StaticClass()));
		if (LevelGenerator.IsValid())
		{
			// Register the initial chunk index
			CurrentChunkIndex = 0;
			LevelGenerator->OnPlayerEnteredChunk(CurrentChunkIndex, -1);
		}
		else
		{
			UE_LOG(LogTemplateCharacter, Warning, TEXT("No valid Level Generator found in GameMode!"));
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("No valid GameMode found!"));
	}

}

void AEnferCharacter::OnChunkOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if we overlapped with a chunk
	if (AJGChunk* chunk = Cast<AJGChunk>(OtherActor))
	{
		// Only process if we have a valid level generator and the chunk index is different
		if (LevelGenerator.IsValid() && CurrentChunkIndex != chunk->ChunkLogicalIndex)
		{
			int32 previousChunkIndex = CurrentChunkIndex;
			CurrentChunkIndex = chunk->ChunkLogicalIndex;
			LevelGenerator->OnPlayerEnteredChunk(CurrentChunkIndex, previousChunkIndex);
		}
	}
}

void AEnferCharacter::SetupPlayerInputComponent(UInputComponent* playerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(playerInputComponent))
	{
		// Start
		enhancedInputComponent->BindAction(StartGameAction, ETriggerEvent::Triggered, this, &AEnferCharacter::StartGame, enhancedInputComponent);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AEnferCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AEnferCharacter::WalkForward()
{
	DoMove(0.0f, 1.0f);
}

void AEnferCharacter::WalkBackward()
{
	DoMove(0.0f, -1.0f);
}

void AEnferCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);
}

void AEnferCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw * 0.25f);
		AddControllerPitchInput(Pitch * 0.25f);
	}
}

void AEnferCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AEnferCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AEnferCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

void AEnferCharacter::OnPausePressed()
{
	if (!GetWorld()->GetAuthGameMode()->IsPaused())
		GetWorld()->GetAuthGameMode()->SetPause(GetLocalViewingPlayerController());
	else
		GetWorld()->GetAuthGameMode()->ClearPause();
}

void AEnferCharacter::StartGame(UEnhancedInputComponent* playerInputComponent)
{
	if (HasGameStarted)
		return;

	// Pause
	playerInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &AEnferCharacter::OnPausePressed);

	// Jumping
	playerInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AEnferCharacter::DoJumpStart);
	playerInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AEnferCharacter::DoJumpEnd);

	// Moving
	playerInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AEnferCharacter::MoveInput);
	playerInputComponent->BindAction(WalkForwardAction, ETriggerEvent::Triggered, this, &AEnferCharacter::WalkForward);
	playerInputComponent->BindAction(WalkBackwardAction, ETriggerEvent::Triggered, this, &AEnferCharacter::WalkBackward);

	// Looking/Aiming
	playerInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AEnferCharacter::LookInput);
	playerInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AEnferCharacter::LookInput);
	
	GetLocalViewingPlayerController()->SetInputMode(FInputModeGameOnly());
	
	OnGameStarted.Broadcast();

	HasGameStarted = true;
}
