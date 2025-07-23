// TRACharacter.cpp
#include "TRACharacter.h"

#include "GameModes/TRAGameState.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "EnhancedInputComponent.h"
#include "Controllers/TRAPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Components/WidgetComponent.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"

ATRACharacter::ATRACharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);	// Root 컴포넌트에 연결
	SpringArmComp->TargetArmLength = 300.0f;		// 스프링 암의 기본 길이 설정
	SpringArmComp->bUsePawnControlRotation = true;	// Pawn의 제어 회전을 사용하도록 설정

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);	// SpringArm 컴포넌트에 연결
	CameraComp->bUsePawnControlRotation = false;									// 카메라의 제어 회전을 비활성화

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(GetMesh());				// StaticMeshComponent에 붙여주겠다는 의미
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);	// 스크린 모드로 설정

	NormalSpeed = 600.0f;								// 일반 이동 속도
	SprintSpeedMultiplier = 2.0f;//1.7f;				// 스프린트 속도 배수
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;	// 스프린트 속도 계산

	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed; // 기본 이동 속도 설정

	MaxHealth = 100.0f; // 최대 체력
	Health = MaxHealth; // 현재 체력

	// 캐릭터가 컨트롤러의 Yaw 회전을 따라가지 않도록 설정
	// -> 이동 방향을 기준으로 캐릭터가 회전하게 하려면 false여야 함
	bUseControllerRotationYaw = false;

	// 캐릭터가 이동 방향을 기준으로 자연스럽게 회전하도록 설정
	// -> 예: 앞으로 이동하면 캐릭터가 자동으로 앞을 보게 됨
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// 캐릭터가 회전할 때의 속도를 설정 (Pitch, Yaw, Roll 순서)
	// 이동 방향 전환 시 부드럽지만 빠르게 회전하는 느낌을 줌
	// 아래 예시는 Yaw(좌우 회전)만 설정하여 초당 420도 속도로 회전하도록 함
	// ex) GetCharacterMovement()->RotationRate = FRotator(0.f, 420.f, 0.f); // 초당 420도 Yaw 회전
	// -> 블루프린트에서 직접 수정
}

float ATRACharacter::GetHealth() const
{
	return Health;
}

void ATRACharacter::AddHealth(float Amount)
{
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	//UE_LOG(LogTemp, Warning, TEXT("Health increasd to : %f"), Health);
	UpdateOverheadHP();
}

void ATRACharacter::BeginPlay()
{
	Super::BeginPlay();
	UpdateOverheadHP();
}

void ATRACharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (OverheadWidget)
	{
		FVector CameraLocation = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation();
		FVector ToCamera = CameraLocation - OverheadWidget->GetComponentLocation();
		FRotator LookAtRotation = FRotationMatrix::MakeFromX(ToCamera).Rotator();
		OverheadWidget->SetWorldRotation(LookAtRotation);
	}
}

void ATRACharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput
		= Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ATRAPlayerController* PlayerController
			= Cast<ATRAPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&ATRACharacter::Move
				);
			}

			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Triggered,
					this,
					&ATRACharacter::StartJump
				);
			}

			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Completed,
					this,
					&ATRACharacter::StopJump
				);
			}

			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&ATRACharacter::Look
				);
			}

			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Triggered,
					this,
					&ATRACharacter::StartSprint
				);
			}

			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Completed,
					this,
					&ATRACharacter::StopSprint
				);
			}
		}
	}
}

float ATRACharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	// 부모 클래스 내용부터 호출을 해야 기본적인 로직이 일단 실행된다.
	// 반환값은 데미지 입은 값
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
	//UE_LOG(LogTemp, Warning, TEXT("Health decresed to : %f"), Health);
	UpdateOverheadHP();

	if (Health <= 0.0f)
	{
		OnDeath();
	}

	return ActualDamage;
}

void ATRACharacter::Move(const FInputActionValue& value)
{
	/*
		기준: 캐릭터 방향
		캐릭터가 바라보는 방향을 기준으로 앞/뒤, 좌/우를 판단
		
		만약 카메라가 캐릭터 뒤에 고정되어 있고
		캐릭터가 그쪽을 바라보는 형태라면, 입력 방향과 이동 방향이 일치.
		
		하지만 카메라가 자유롭게 회전하거나 조작자가 보는 방향으로 이동하기 원한다면
		직관적이지 않을 수 있다.
	*/
	//if (!Controller) return;

	//const FVector2D MoveInput = value.Get<FVector2D>();
	//if (!FMath::IsNearlyZero(MoveInput.X))
	//{
	//	AddMovementInput(GetActorForwardVector(), MoveInput.X);
	//}
	//if (!FMath::IsNearlyZero(MoveInput.Y))
	//{
	//	AddMovementInput(GetActorRightVector(), MoveInput.Y);
	//}

	/*
		기준: 카메라 회전(Yaw)
		Controller->GetControlRotation().Yaw: 일반적으로 카메라의 회전(Yaw)을 의미
		따라서 이 방식은 카메라가 바라보는 방향을 기준으로 이동 방향을 계산
		예를 들어, 카메라가 오른쪽을 바라보면 W 키(앞)는 캐릭터가 오른쪽으로 움직인다.
		이는 대부분의 3인칭 액션 게임, 어드벤처 게임 등에서 기대하는 이동 방식

		유저가 보는 방향을 기준으로 캐릭터가 움직이는 것이 더 직관적이고 조작감이 좋다.
		특히 자유 카메라가 있는 3인칭 게임에서는, 카메라 회전과 별개로 캐릭터가 고정된 방향으로 움직이면 불편하다.
		예) 유저가 왼쪽을 보고 있는데 W키 누르면 캐릭터는 "자기 기준 앞"으로 가버려서, 화면 기준으로는 엉뚱한 방향으로 이동
	*/
	if (!Controller) return;

	const FVector2D MoveInput = value.Get<FVector2D>();
	const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0); // Yaw만 사용
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (!FMath::IsNearlyZero(MoveInput.X))
	{
		AddMovementInput(ForwardDir, MoveInput.X);
	}
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(RightDir, MoveInput.Y);
	}
}

void ATRACharacter::StartJump(const FInputActionValue& value)
{
	if (value.Get<bool>())
	{
		Jump();
	}
}

void ATRACharacter::StopJump(const FInputActionValue& value)
{
	if (!value.Get<bool>())
	{
		StopJumping();
	}
}

void ATRACharacter::Look(const FInputActionValue& value)
{
	FVector2D LookInput = value.Get<FVector2D>();

	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

void ATRACharacter::StartSprint(const FInputActionValue& value)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed; // 스프린트 속도로 변경
	}
}

void ATRACharacter::StopSprint(const FInputActionValue& value)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed; // 일반 속도로 변경
	}
}

void ATRACharacter::OnDeath()
{
	ATRAGameState* TRAGameState = GetWorld() ? GetWorld()->GetGameState<ATRAGameState>() : nullptr;
	if (TRAGameState)
	{
		TRAGameState->OnGameOver();
	}
}

void ATRACharacter::UpdateOverheadHP()
{
	if (!OverheadWidget) return;

	UUserWidget* OverheadWidgetInstance = OverheadWidget->GetUserWidgetObject();
	if (!OverheadWidgetInstance) return;

	if (UTextBlock* HPText = Cast<UTextBlock>(OverheadWidgetInstance->GetWidgetFromName(TEXT("OverHeadHPText"))))
	{
		HPText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth)));
	}

	// ProgressBar로 HP 표시
	if (UProgressBar* HPBar = Cast<UProgressBar>(OverheadWidgetInstance->GetWidgetFromName(TEXT("OverHeadHPBar"))))
	{
		HPBar->SetPercent(FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f));
	}
}

