// Fill out your copyright notice in the Description page of Project Settings.

#include "PZGameMode.h"
#include "PZCharacter.h"

APZGameMode::APZGameMode()
{
	// Set default pawn class to our character
	DefaultPawnClass = APZCharacter::StaticClass();
}
