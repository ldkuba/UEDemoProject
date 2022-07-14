// Copyright Promethean AI, LLC 2018

#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "GameFramework/Actor.h"
#include "Misc/ObjectThumbnail.h"

#include "UObject/ConstructorHelpers.h"
#include "UObject/Object.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/ObjectLibrary.h"
#include "Runtime/Launch/Resources/Version.h"  // engine version macros
#if ENGINE_MAJOR_VERSION >= 4  && ENGINE_MINOR_VERSION > 19  // MaterialStatsCommon missing on 4.19 so compiling
	#include "MaterialStatsCommon.h"
#endif

/**
 *	Go thorugh the entire asset library and save out all of the asset data to a temporary folder currently hardcoded to "C:\UE4AssetData"
 *	@param DataTypeName	- name of data type. "StaticMesh", "Material" and "MaterialInstanceConstant" are the only ones currently supported
 *	@return
 */
void GetAssetLibraryData(FString DataTypeName);
TSharedPtr<FJsonObject> getStaticMeshAssetData(FAssetData AssetData);
TSharedPtr<FJsonObject> getMaterialAssetData(FAssetData AssetData);
TSharedPtr<FJsonObject> getMaterialInstanceAssetData(FAssetData AssetData);
TSharedPtr<FJsonObject> getTextureAssetData(FAssetData AssetData);
TSharedPtr<FJsonObject> getLevelAssetData(FAssetData AssetData);
TSharedPtr<FJsonObject> getBlueprintAssetData(FAssetData AssetData);
TSharedPtr<FJsonObject> getUnknownAssetData();
TArray<FString> GetCurrentLevelArtAssets();

const FString OutFolderPath = "C:/PrometheanAITemp/";

/**
 *	Save FObjectThumbnail to a filename. TODO: there is seemingly a Gamma issue happening that could use some investigating
 *	@param DataTypeName	- name of data type. "StaticMesh", "Material" and "MaterialInstanceConstant" are the only ones currently supported
 *	@return
 */
FString SaveThumbnail(FObjectThumbnail* ObjectThumbnail, FString Filename);

// --------------------------------------------------------------------
// --- Working around necessary data being private. 
// --------------------------------------------------------------------
// structure used to store various statistics extracted from compiled shaders
// original definition is private in MaterialStats.h so copying it here for now
//TODO: future proof and find a way to tie it back to source


#if ENGINE_MAJOR_VERSION >= 4  && ENGINE_MINOR_VERSION > 19  // MaterialStatsCommon missing on 4.19 anyway so compiling out for now
/** structure used to store various statistics extracted from compiled shaders */
struct FShaderStatsInfo
{
	struct FContent
	{
		FString StrDescription;
		FString StrDescriptionLong;
	};

	TMap<ERepresentativeShader, FContent> ShaderInstructionCount;
	FContent SamplersCount;
	FContent InterpolatorsCount;
	FContent TextureSampleCount;
	FContent VirtualTextureLookupCount;
	FString StrShaderErrors;

	void Reset()
	{
		ShaderInstructionCount.Empty();

		SamplersCount.StrDescription = TEXT("Compiling...");
		SamplersCount.StrDescriptionLong = TEXT("Compiling...");

		InterpolatorsCount.StrDescription = TEXT("Compiling...");
		InterpolatorsCount.StrDescriptionLong = TEXT("Compiling...");

		TextureSampleCount.StrDescription = TEXT("Compiling...");
		TextureSampleCount.StrDescriptionLong = TEXT("Compiling...");

		VirtualTextureLookupCount.StrDescription = TEXT("Compiling...");
		VirtualTextureLookupCount.StrDescriptionLong = TEXT("Compiling...");

		StrShaderErrors.Empty();
	}

	void Empty()
	{
		ShaderInstructionCount.Empty();

		SamplersCount.StrDescription.Empty();
		SamplersCount.StrDescriptionLong.Empty();

		InterpolatorsCount.StrDescription.Empty();
		InterpolatorsCount.StrDescriptionLong.Empty();

		TextureSampleCount.StrDescription.Empty();
		TextureSampleCount.StrDescriptionLong.Empty();

		VirtualTextureLookupCount.StrDescription.Empty();
		VirtualTextureLookupCount.StrDescriptionLong.Empty();

		StrShaderErrors.Empty();
	}

	bool HasErrors()
	{
		return !StrShaderErrors.IsEmpty();
	}
};
# endif
