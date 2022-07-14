// Copyright Promethean AI, LLC 2018

#pragma once

#include "CoreMinimal.h"

#include "UObject/ConstructorHelpers.h"
#include "UObject/Object.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "Json.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/ObjectLibrary.h"
#include "EditorFramework/AssetImportData.h"


void PrometheanLearn(UWorld* World, FString OutputPath);
void PrometheanLearnSelection(FString SceneName, FString OutputPath);
void PrometheanLearnView(FString SceneName, FString OutputPath);

/**
 *	Output file with Learned Data
 *	@param
 *	@return
 */
void GenerateLearnData(TArray<AActor*> StaticMeshActors, FString SceneName, FString OutputPath);

/**
 *	Go thorugh the scene and output data on all objects in the scene
 *	@param 
 *	@return A json string for output to file
 */
FString GenerateLearnFileString(TArray<AActor*> LearnActors, FString SceneName);


/**
 *	Centralized function that controls the coordinate system for both learn and update transform commands
 *	@param
 *	@return A json array of shraed ptr float values
 */
TArray<TSharedPtr<FJsonValue>> GetVectorJSONData(FVector UEVector);
TArray<TSharedPtr<FJsonValue>> GetRotationJSONData(FRotator ActorRotation);
