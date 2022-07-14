// Fill out your copyright notice in the Description page of Project Settings.

#include "PrometheanLibraryData.h"  // matching header has to go first
#include "PrometheanGeneric.h"
#include "PrometheanLearning.h"
#include "PrometheanGeneric.h"
#include "promethean.h" // has stuff necessary for custom logging


#include "UObject/Object.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "Json.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/ObjectLibrary.h"

#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#if ENGINE_MAJOR_VERSION >= 4  && ENGINE_MINOR_VERSION > 19  // MaterialStatsCommon missing on 4.19 so compiling
	#include "MaterialStatsCommon.h"
#endif
#if ENGINE_MAJOR_VERSION >= 4  && ENGINE_MINOR_VERSION >= 21
	#include "Misc/FileHelper.h" // - to output images on 4.21 and higher
#endif
#include "HighResScreenshot.h"  // - to output asset thumbnails on 4.20 and below
#include "ObjectTools.h"  // - has ThumbnailTools manager!
#include "AutoReimport/AssetSourceFilenameCache.h"  // - getting source asset without having to load it
//#include "MaterialStats.h" // can't import, private but has declarations for material stats info that we need

// --------------------------------------------------------------------
// --- ASSET LIBRARY FUNCTIONS
// --------------------------------------------------------------------

// start of this is copied from https://forums.unrealengine.com/development-discussion/c-gameplay-programming/76797-how-to-save-utexture2d-to-png-file
// but repurposed to work with object thumbnail
FString SaveThumbnail(FObjectThumbnail* ObjectThumbnail, FString Filename)
{
	FString ResultPath;
	TArray<FColor> OutBMP;
	int w = ObjectThumbnail->GetImageWidth();
	int h = ObjectThumbnail->GetImageHeight();	
	const TArray<uint8>& ImageData = ObjectThumbnail->AccessImageData();
	if (ImageData.Num() < (w * h * 4)) // In some extreme cases, ImageData may be empty. No idea why.
	{
		return ResultPath;
	}

	OutBMP.InsertZeroed(0, w*h);

	for (int i = 0; i < (w*h); ++i)
	{
		int j = i * 4;		
		//FColor* Color = new FColor(ImageData[j+2], ImageData[j + 1], ImageData[j], ImageData[j + 3]);  // RED and BLUE are switched
		FColor* Color = new FColor(ImageData[j+2], ImageData[j + 1], ImageData[j], 255);  // RED and BLUE are switched
		OutBMP[i] = *Color;		
	}
	
	FIntPoint DestSize(w, h);	

	TArray<FString> FileList;
	Filename.ParseIntoArray(FileList, TEXT("."), true);  // need to make sure we don't repeat the name second time after the '.' i.e. --Props--SM_Chair.SM_Chair.bmp
	ResultPath = FString::Printf(TEXT("%s.bmp"), *FileList[0]);  // need to add bmp extension to force CreateBitmap not ue4 add numbers at the end		
	bool bSaved = FFileHelper::CreateBitmap(*ResultPath, w, h, OutBMP.GetData());

	UE_LOG(LogPromethean, Display, TEXT("ThumbnailSize: %d %d"), w, h);
	UE_LOG(LogPromethean, Display, TEXT("SaveThumbnail: %s %d"), *ResultPath, bSaved == true ? 1 : 0);
	return ResultPath;
}

void GetAssetLibraryData(FString DataTypeName)
{
	UE_LOG(LogPromethean, Display, TEXT("Getting asset library data..."));
	
	UClass* cls = TSubclassOf<class UObject>();
	
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(cls, false, GIsEditor);
	
	if (ObjectLibrary != nullptr)
	{
		ObjectLibrary->AddToRoot();

		FString NewPath = TEXT("/Game");  // "/Game/StarterContent/Architecture"
		int32 NumOfAssetDatas = ObjectLibrary->LoadAssetDataFromPath(NewPath);

		TArray<FAssetData> AssetDatas;
		ObjectLibrary->GetAssetDataList(AssetDatas);

		UE_LOG(LogPromethean, Display, TEXT("About to go through all asset datas... Datas Num: %i"), AssetDatas.Num());
		
		TArray<TSharedPtr<FJsonValue>> JSONDictArray;  // for output
		for (int32 i = 0; i < AssetDatas.Num(); ++i)  // can't print AssetDatas.Num() - crashes!
		{
			FAssetData& AssetData = AssetDatas[i];
			if (AssetData.IsValid())
			{
				// UE_LOG(LogPromethean, Display, TEXT("Data Type: %s"), *AssetData.AssetClass.ToString());
				if (AssetData.AssetClass.ToString() == DataTypeName)  // currently get only all static meshes
				{									
					TSharedPtr<FJsonObject> JSONDict = MakeShareable(new FJsonObject);
					if (DataTypeName == "MaterialInstanceConstant")
					{
						JSONDict = getMaterialInstanceAssetData(AssetData);
					}
					else if (DataTypeName == "Material")
					{
						JSONDict = getMaterialAssetData(AssetData);
					}
					else if (DataTypeName == "StaticMesh")
					{
						JSONDict = getStaticMeshAssetData(AssetData);
					}
					else if (DataTypeName == "Texture2D")
					{
						JSONDict = getTextureAssetData(AssetData);
					}
					else if (DataTypeName == "World")
					{
						// JSONDict = getLevelAssetData(AssetData);
					}
					TSharedRef<FJsonValueObject> JSONDictValue = MakeShareable(new FJsonValueObject(JSONDict));
					UE_LOG(LogPromethean, Display, TEXT("================================================"));
					// -- Add To Array
					JSONDictArray.Add(JSONDictValue);
				}				
			}
		}
		// output json data to string
		TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
		RootObject->SetArrayField(TEXT("asset_data"), JSONDictArray);
		FString OutputString;
		TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
		SaveToFile(OutFolderPath + "/ue4_asset_data_" + DataTypeName + ".json", OutputString);
	}
}

TSharedPtr<FJsonObject> getStaticMeshAssetData(FAssetData AssetData)
{
	TSharedPtr<FJsonObject> JSONDict = MakeShareable(new FJsonObject);
	if (AssetData.AssetClass.ToString() == "StaticMesh")
	{
		for (const auto& TagAndValuePair : AssetData.TagsAndValues)
		{
			// UE_LOG(LogPromethean, Display, TEXT("Tag NameValue Pair: %s : %s"), *TagAndValuePair.Key.ToString(), *TagAndValuePair.Value);			
			// ---- Material Count
			if (TagAndValuePair.Key.ToString() == "Materials")
			{
				UE_LOG(LogPromethean, Display, TEXT("Material Count: %i"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));  // gets material slots on the static mesh
				JSONDict->SetNumberField(TEXT("material_count"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
			}
			// ---- Face Count
			else if (TagAndValuePair.Key.ToString() == "Triangles")
			{
				UE_LOG(LogPromethean, Display, TEXT("Faces: %i"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
				JSONDict->SetNumberField(TEXT("face_count"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
			}
			// ---- Vertex Count
			else if (TagAndValuePair.Key.ToString() == "Vertices")
			{
				UE_LOG(LogPromethean, Display, TEXT("Vertices: %i"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
				JSONDict->SetNumberField(TEXT("vertex_count"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
			}
			// ---- UV Channels
			else if (TagAndValuePair.Key.ToString() == "UVChannels")
			{
				UE_LOG(LogPromethean, Display, TEXT("UV Channels: %i"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
				JSONDict->SetNumberField(TEXT("uv_sets"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
			}			
			// ---- Size
			/*  // Get precise size from the static mesh since we have to load it anyway
			else if (TagAndValuePair.Key.ToString() == "ApproxSize")
			{				
				TArray<FString> DimensionsList;
				TagAndValuePair.Value.ParseIntoArray(DimensionsList, TEXT("x"), true);  // value is "8x16x4"
				if (DimensionsList.Num() == 3)  // make sure we split correctly into 3 items. unreal crashes if index is out of range for whatever unlikely reason
				{
					UE_LOG(LogPromethean, Display, TEXT("Size: %s"), *TagAndValuePair.Value);
					TArray<TSharedPtr<FJsonValue>> MeshScaleArray;
					MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(FCString::Atoi(*DimensionsList[0]))));
					MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(FCString::Atoi(*DimensionsList[2]))));  // swizzle
					MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(FCString::Atoi(*DimensionsList[1]))));  // swizzle
					JSONDict->SetArrayField(TEXT("bounding_box"), MeshScaleArray);
				}
			}
			*/
			// ---- LODs
			else if (TagAndValuePair.Key.ToString() == "LODs")
			{				
				UE_LOG(LogPromethean, Display, TEXT("Lod Count: %i"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
				JSONDict->SetNumberField(TEXT("lod_count"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
			}
			
		}		
		// ---- Unique ID - Name
		JSONDict->SetStringField(TEXT("path"), AssetData.ObjectPath.ToString());
		UE_LOG(LogPromethean, Display, TEXT("Asset Path: %s"), *AssetData.ObjectPath.ToString());
		// ---- Name
		JSONDict->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		// ---- Type
		JSONDict->SetStringField(TEXT("type"), "mesh");
		// ---- Vertex Color Channels   // TODO?
		JSONDict->SetNumberField(TEXT("vertex_color_channels"), 1);
		// ---- Source Files
		TOptional<FAssetImportInfo> ImportInfo = FAssetSourceFilenameCache::ExtractAssetImportInfo(AssetData);
		if (ImportInfo.IsSet())
		{
			FString SourcePaths;
			for (const auto& File : ImportInfo->SourceFiles)
			{
				SourcePaths += " " + File.RelativeFilename;
			}
			// JSONDict->SetStringField(TEXT("source_path"), SourcePaths);  // TODO: non english characters prevent json from being written :( 
			UE_LOG(LogPromethean, Display, TEXT("Source Assets: %s"), *SourcePaths);
		}

		// ---- WARNING! Have to load the actual StaticMesh here. Need it to get material paths and pivot offset
		UStaticMesh* StaticMesh = Cast<UStaticMesh>(AssetData.GetAsset());
		// ---- Material Paths
		TArray<TSharedPtr<FJsonValue>> MaterialPathArray;
		for (FStaticMaterial StaticMaterial : StaticMesh->GetStaticMaterials())
		{
			UE_LOG(LogPromethean, Display, TEXT("Material Used: %s"), *StaticMaterial.MaterialInterface->GetPathName());  // gets material slots on the static mesh
			MaterialPathArray.Add(MakeShareable(new FJsonValueString(StaticMaterial.MaterialInterface->GetPathName())));
		}
		JSONDict->SetArrayField(TEXT("material_paths"), MaterialPathArray);
		// ---- Size
		UE_LOG(LogPromethean, Display, TEXT("Size: %s"), *StaticMesh->GetBoundingBox().GetSize().ToString());
		FVector Size = StaticMesh->GetBoundingBox().GetSize();
		TArray<TSharedPtr<FJsonValue>> MeshScaleArray;
		MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(Size.X)));
		MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(Size.Y)));
		MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(Size.Z)));
		JSONDict->SetArrayField(TEXT("bounding_box"), MeshScaleArray);		
		// ---- Pivot Offset
		FVector PivotOffset = StaticMesh->GetBoundingBox().GetCenter();		
		PivotOffset[2] = StaticMesh->GetBoundingBox().Min[2];  // min height
		TArray<TSharedPtr<FJsonValue>> MeshPivotOffsetArray;
		MeshPivotOffsetArray.Add(MakeShareable(new FJsonValueNumber(-PivotOffset.X)));
		MeshPivotOffsetArray.Add(MakeShareable(new FJsonValueNumber(-PivotOffset.Y)));
		MeshPivotOffsetArray.Add(MakeShareable(new FJsonValueNumber(-PivotOffset.Z)));
		JSONDict->SetArrayField(TEXT("pivot_offset"), MeshPivotOffsetArray);

		// ---- Thumbnail Image ----- cached thumbnails can be consistenly blurry so have to regenerate every time
		FObjectThumbnail* ObjectThumbnail = nullptr;		
		FName ObjectFullName = FName(*(AssetData.GetFullName()));		
		TArray<FName> ObjectFullNames;
		ObjectFullNames.Add(ObjectFullName);
		FThumbnailMap LoadedThumbnails;  // out tmap of <name:thumbnail>
		if (ThumbnailTools::ConditionallyLoadThumbnailsForObjects(ObjectFullNames, LoadedThumbnails))
		{			
			ObjectThumbnail = LoadedThumbnails.Find(ObjectFullName);					
			if (ObjectThumbnail != NULL && !ObjectThumbnail->IsEmpty())
			{
				ObjectThumbnail->DecompressImageData();  // if success decompress before writing
			}
		}
		else
		{
			ObjectThumbnail = ThumbnailTools::GenerateThumbnailForObjectToSaveToDisk(StaticMesh);  // if couldn't load - generate
		}		
		// FObjectThumbnail* ObjectThumbnail = ThumbnailTools::GenerateThumbnailForObjectToSaveToDisk(StaticMesh);  // if couldn't load - generate
		// ---- If got thumbnail - save
		FString FileName = AssetData.ObjectPath.ToString();
		FileName = FileName.Replace(TEXT("/"), TEXT("--"));
		FString OutputPath = OutFolderPath + "/thumbnails/" + FileName;  // define path globally because it's reused for triangle too
		if (ObjectThumbnail != NULL && !ObjectThumbnail->IsEmpty())
		{
			UE_LOG(LogPromethean, Display, TEXT("Thumbnail Found. Size: %i"), ObjectThumbnail->GetCompressedDataSize());			
			FString ThumbPath = SaveThumbnail(ObjectThumbnail, OutputPath);  // will add extension at the end
			JSONDict->SetStringField(TEXT("thumbnail"), ThumbPath);
		}
		else
		{
			UE_LOG(LogPromethean, Display, TEXT("Thumbnail could not be generated! %s"), *OutputPath);
		}
		// ---- Raw Triangles
		TArray<FVector> Vertexes = GetMeshTrianglePositionsFromLibraryStaticMesh(StaticMesh);
		if (Vertexes.Num() != 0)
		{
			FString VertexString = FVectorArrayToJsonString(Vertexes, TEXT("verts"));
			SaveToFile(OutputPath, VertexString);  // no file format is currently reserved for vertexes
			JSONDict->SetStringField(TEXT("verts"), OutputPath);
		}
	}
	return JSONDict;
}

TSharedPtr<FJsonObject> getMaterialAssetData(FAssetData AssetData)
{
	TSharedPtr<FJsonObject> JSONDict = MakeShareable(new FJsonObject);
	if (AssetData.AssetClass.ToString() == "Material")
	{		
		// ---- Unique ID - Name
		JSONDict->SetStringField(TEXT("path"), AssetData.ObjectPath.ToString());
		UE_LOG(LogPromethean, Display, TEXT("Asset Path: %s"), *AssetData.ObjectPath.ToString());
		// ---- Name
		JSONDict->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		// ---- Type
		JSONDict->SetStringField(TEXT("type"), "material");
		// turn AssetData to UMaterial
		UMaterial* MaterialInstance = Cast<UMaterial>(AssetData.GetAsset());
		// ---- Parent Material
		JSONDict->SetStringField(TEXT("parent_path"), MaterialInstance->GetBaseMaterial()->GetPathName());
		UE_LOG(LogPromethean, Display, TEXT("Parent Name: %s"), *MaterialInstance->GetBaseMaterial()->GetPathName());
		// ---- Is Instance
		JSONDict->SetNumberField(TEXT("is_instance"), 0);
		// ---- Instruction Counts
		JSONDict->SetNumberField(TEXT("instructions_static"), 0);  // set default value
		JSONDict->SetNumberField(TEXT("instructions_dynamic"), 0);  // set default value
		JSONDict->SetNumberField(TEXT("instructions_vertex"), 0);  // set default value
		#if ENGINE_MAJOR_VERSION >= 4  && ENGINE_MINOR_VERSION > 19  // MaterialStatsCommon missing on 4.19 so compiling out for now
			FShaderStatsInfo OutInfo;  // WARNING: defined the structure locally since global definition is private. might change
			FMaterialResource* Resource = MaterialInstance->GetMaterialResource(GMaxRHIFeatureLevel);
			FMaterialStatsUtils::ExtractMatertialStatsInfo(OutInfo, Resource);
			for (auto& Elem : OutInfo.ShaderInstructionCount)
			{
				int32 intValue = FCString::Atoi(*Elem.Value.StrDescription);
				switch (Elem.Key)  // key is a shader type enum
				{
				case ERepresentativeShader::StationarySurface:
					JSONDict->SetNumberField(TEXT("instructions_static"), intValue);
					UE_LOG(LogPromethean, Display, TEXT("INSTRUCTION COUNT Static %s"), *Elem.Value.StrDescription);
					break;
				case ERepresentativeShader::DynamicallyLitObject:
					JSONDict->SetNumberField(TEXT("instructions_dynamic"), intValue);
					UE_LOG(LogPromethean, Display, TEXT("INSTRUCTION COUNT Dynamic %s"), *Elem.Value.StrDescription);
					break;
				case ERepresentativeShader::FirstVertexShader:
					JSONDict->SetNumberField(TEXT("instructions_vertex"), intValue);
					UE_LOG(LogPromethean, Display, TEXT("INSTRUCTION COUNT Vertex Shader %s"), *Elem.Value.StrDescription);
					break;
				};  // UE_LOG(LogPromethean, Display, TEXT("SAMPLER COUNT : %s"), *OutInfo.SamplersCount.StrDescription);  // alternate way to get textures					
			}
		#endif
		// ---- Number of textures
		TArray<UTexture*> MaterialTextures;
		//MaterialInstance->GetUsedTextures(MaterialTextures, EMaterialQualityLevel::Num, true, GMaxRHIFeatureLevel, true);
		MaterialInstance->GetUsedTextures(MaterialTextures, EMaterialQualityLevel::Num, true, ERHIFeatureLevel::SM5, true);
		JSONDict->SetNumberField(TEXT("texture_count"), MaterialTextures.Num());
		UE_LOG(LogPromethean, Display, TEXT("Textures: %i"), MaterialTextures.Num());
		// ---- Texture Paths
		TArray<TSharedPtr<FJsonValue>> TexturePathArray;
		for (UTexture* Texture : MaterialTextures)
		{
			UE_LOG(LogPromethean, Display, TEXT("Texture Used: %s"), *Texture->GetPathName());  // gets material slots on the static mesh
			TexturePathArray.Add(MakeShareable(new FJsonValueString(Texture->GetPathName())));
		}
		JSONDict->SetArrayField(TEXT("texture_paths"), TexturePathArray);
		// ---- Two Sided
		JSONDict->SetNumberField(TEXT("is_two_sided"), MaterialInstance->TwoSided);
		UE_LOG(LogPromethean, Display, TEXT("Two Sided: %i"), MaterialInstance->TwoSided);
		// ---- Masked
		JSONDict->SetNumberField(TEXT("is_masked"), MaterialInstance->IsMasked());
		UE_LOG(LogPromethean, Display, TEXT("Masked: %i"), MaterialInstance->IsMasked());
		// ---- Thumbnail ---- previews are consistenly blurry and don't update, so have to regenerate every time
		/*
		FName ObjectFullName = FName(*(AssetData.GetFullName()));
		TArray<FName> ObjectFullNames;
		ObjectFullNames.Add(ObjectFullName);
		FThumbnailMap LoadedThumbnails;  // out tmap of <name:thumbnail>
		FObjectThumbnail* ObjectThumbnail = nullptr;
		if (ThumbnailTools::ConditionallyLoadThumbnailsForObjects(ObjectFullNames, LoadedThumbnails))
		{
			ObjectThumbnail = LoadedThumbnails.Find(ObjectFullName);
			if (ObjectThumbnail != NULL && !ObjectThumbnail->IsEmpty())
				ObjectThumbnail->DecompressImageData();  // if success decompress before writing
		}
		else
		{
			ObjectThumbnail = ThumbnailTools::GenerateThumbnailForObjectToSaveToDisk(MaterialInstance);  // if couldn't load - generate
		}
		*/
		FObjectThumbnail* ObjectThumbnail = ThumbnailTools::GenerateThumbnailForObjectToSaveToDisk(MaterialInstance);  // if couldn't load - generate

		if (ObjectThumbnail != NULL && !ObjectThumbnail->IsEmpty())  // if got thumbnail - save
		{
			UE_LOG(LogPromethean, Display, TEXT("Thumbnail Found. Size: %i"), ObjectThumbnail->GetCompressedDataSize());
			FString FileName = AssetData.ObjectPath.ToString();
			FileName = FileName.Replace(TEXT("/"), TEXT("--"));
			FString OutputPath = OutFolderPath + "/thumbnails/" + FileName;
			OutputPath = SaveThumbnail(ObjectThumbnail, OutputPath);  // will add extension at the end
			JSONDict->SetStringField(TEXT("thumbnail"), OutputPath);
		}
		// ---- Material Functions Used
		auto MaterialFunctions = GetMaterialFunctionDependencies(*MaterialInstance);
		TArray<TSharedPtr<FJsonValue>> MaterialFunctionPaths;
		for (auto& Function : MaterialFunctions)
		{
			UE_LOG(LogPromethean, Display, TEXT("Material Function Used: %s"), *Function->GetPathName());
			MaterialFunctionPaths.Add(MakeShareable(new FJsonValueString(Function->GetPathName())));
		}
		JSONDict->SetArrayField(TEXT("material_functions"), MaterialFunctionPaths);
	}
	return JSONDict;
}

TSharedPtr<FJsonObject> getMaterialInstanceAssetData(FAssetData AssetData)
{
	TSharedPtr<FJsonObject> JSONDict = MakeShareable(new FJsonObject);
	if (AssetData.AssetClass.ToString() == "MaterialInstanceConstant")
	{
		// ---- Unique ID - Name
		JSONDict->SetStringField(TEXT("path"), AssetData.ObjectPath.ToString());
		UE_LOG(LogPromethean, Display, TEXT("Asset path: %s"), *AssetData.ObjectPath.ToString());
		// ---- Name		
		JSONDict->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		// ---- Type
		JSONDict->SetStringField(TEXT("type"), "material");
		// turn AssetData to UMaterialInstanceConstant
		UMaterialInstanceConstant* MaterialInstance = Cast<UMaterialInstanceConstant>(AssetData.GetAsset());
		// ---- Parent Material				
		FString ParentPath = "";		
		UMaterial* ParentMaterial = MaterialInstance->GetBaseMaterial();
		if (IsValid(ParentMaterial))  // need to check if material exists first - otherwise hard crash
			ParentPath = ParentMaterial->GetPathName();
		else
			UE_LOG(LogPromethean, Display, TEXT("Warning! Can't find a valid parent for this MaterialInstance - it might be corrupted"));
		JSONDict->SetStringField(TEXT("parent_path"), ParentPath);
		UE_LOG(LogPromethean, Display, TEXT("Parent Name: %s"), *ParentPath);
		// ---- Is Instance
		JSONDict->SetNumberField(TEXT("is_instance"), 1);
		// ---- Instruction Counts
		JSONDict->SetNumberField(TEXT("instructions_static"), 0);  // set default value
		JSONDict->SetNumberField(TEXT("instructions_dynamic"), 0);  // set default value
		JSONDict->SetNumberField(TEXT("instructions_vertex"), 0);  // set default value
		#if ENGINE_MAJOR_VERSION >= 4  && ENGINE_MINOR_VERSION > 19  // MaterialStatsCommon missing on 4.19 so compiling out for now
			FShaderStatsInfo OutInfo;  // WARNING: defined the structure locally since global definition is private. might change
			FMaterialResource* Resource = MaterialInstance->GetMaterialResource(GMaxRHIFeatureLevel);  // 
			if (Resource == nullptr)
			{
				UE_LOG(LogPromethean, Display, TEXT("!Warning! Material resource is not valid. Asset could be corrupted."));
			}
			else
			{
				FMaterialStatsUtils::ExtractMatertialStatsInfo(OutInfo, Resource);
				for (auto& Elem : OutInfo.ShaderInstructionCount)  // Elem = TMap<ERepresentativeShader, FContent>
				{
					UE_LOG(LogPromethean, Display, TEXT("Getting Shader Instruction Count..."));  // without this 4.23 seems to inexplicably crash
					
					int32 intValue = FCString::Atoi(*Elem.Value.StrDescription);
					
					switch (Elem.Key)  // key is a shader type enum
					{
					case ERepresentativeShader::StationarySurface:
						JSONDict->SetNumberField(TEXT("instructions_static"), intValue);
						UE_LOG(LogPromethean, Display, TEXT("INSTRUCTION COUNT Static %s"), *Elem.Value.StrDescription);
						break;
					case ERepresentativeShader::DynamicallyLitObject:
						JSONDict->SetNumberField(TEXT("instructions_dynamic"), intValue);
						UE_LOG(LogPromethean, Display, TEXT("INSTRUCTION COUNT Dynamic %s"), *Elem.Value.StrDescription);
						break;
					case ERepresentativeShader::FirstVertexShader:
						JSONDict->SetNumberField(TEXT("instructions_vertex"), intValue);
						UE_LOG(LogPromethean, Display, TEXT("INSTRUCTION COUNT Vertex Shader %s"), *Elem.Value.StrDescription);
						break;
					};  // UE_LOG(LogPromethean, Display, TEXT("SAMPLER COUNT : %s"), *OutInfo.SamplersCount.StrDescription);  // alternate way to get textures					
					
				}
			}
		#endif
		// ---- Number of textures					
		TArray<UTexture*> MaterialTextures;
		MaterialInstance->GetUsedTextures(MaterialTextures, EMaterialQualityLevel::Num, true, GMaxRHIFeatureLevel, true);
		JSONDict->SetNumberField(TEXT("texture_count"), MaterialTextures.Num());
		UE_LOG(LogPromethean, Display, TEXT("Textures: %i"), MaterialTextures.Num());
		// ---- Texture Paths
		TArray<TSharedPtr<FJsonValue>> TexturePathArray;
		FString TexturePath = "";
		for (UTexture* Texture : MaterialTextures)
		{
			TexturePath = Texture->GetFullName();
			UE_LOG(LogPromethean, Display, TEXT("Texture Used: %s"), *TexturePath);  // gets material slots on the static mesh
			TexturePathArray.Add(MakeShareable(new FJsonValueString(TexturePath)));
		}
		JSONDict->SetArrayField(TEXT("texture_paths"), TexturePathArray);
		// ---- Two Sided
		JSONDict->SetNumberField(TEXT("is_two_sided"), MaterialInstance->TwoSided);
		UE_LOG(LogPromethean, Display, TEXT("Two Sided: %i"), MaterialInstance->TwoSided);
		// ---- Masked
		JSONDict->SetNumberField(TEXT("is_masked"), MaterialInstance->IsMasked());
		UE_LOG(LogPromethean, Display, TEXT("Masked: %i"), MaterialInstance->IsMasked());
		// ---- Thumbnail ---- previews are consistenly blurry and don't update, so have to regenerate every time
		/*
		FName ObjectFullName = FName(*(AssetData.GetFullName()));
		TArray<FName> ObjectFullNames;
		ObjectFullNames.Add(ObjectFullName);
		FThumbnailMap LoadedThumbnails;  // out tmap of <name:thumbnail>
		FObjectThumbnail* ObjectThumbnail = nullptr;
		if (ThumbnailTools::ConditionallyLoadThumbnailsForObjects(ObjectFullNames, LoadedThumbnails))
		{
			ObjectThumbnail = LoadedThumbnails.Find(ObjectFullName);
			if (ObjectThumbnail != NULL && !ObjectThumbnail->IsEmpty())
				ObjectThumbnail->DecompressImageData();  // if success decompress before writing
		}
		else
		{
			ObjectThumbnail = ThumbnailTools::GenerateThumbnailForObjectToSaveToDisk(MaterialInstance);  // if couldn't load - generate
		}
		*/
		FObjectThumbnail* ObjectThumbnail = ThumbnailTools::GenerateThumbnailForObjectToSaveToDisk(MaterialInstance);  // if couldn't load - generate

		if (ObjectThumbnail != NULL && !ObjectThumbnail->IsEmpty())  // if got thumbnail - save
		{
			UE_LOG(LogPromethean, Display, TEXT("Thumbnail Found. Size: %i"), ObjectThumbnail->GetCompressedDataSize());
			FString FileName = AssetData.ObjectPath.ToString();
			FileName = FileName.Replace(TEXT("/"), TEXT("--"));
			FString OutputPath = OutFolderPath + "/thumbnails/" + FileName;
			OutputPath = SaveThumbnail(ObjectThumbnail, OutputPath);  // will add extension at the end
			JSONDict->SetStringField(TEXT("thumbnail"), OutputPath);
		}
	}
	return JSONDict;
}

TSharedPtr<FJsonObject> getTextureAssetData(FAssetData AssetData)
{
	TSharedPtr<FJsonObject> JSONDict = MakeShareable(new FJsonObject);	
	if (AssetData.AssetClass.ToString() == "Texture2D")
	{		
		for (const auto& TagAndValuePair : AssetData.TagsAndValues)
		{	// tags include: Format, AssetImportData, SRGB, CompressionSettings, Filter, NeverStream, LODGroup, AddressY, AddressX, Dimensions
			// UE_LOG(LogPromethean, Display, TEXT("Tag NameValue Pair: %s : %s"), *TagAndValuePair.Key.ToString(), *TagAndValuePair.Value);
			if (TagAndValuePair.Key.ToString() == "Dimensions")
			{
				// ---- Texture Size
				TArray<FString> DimensionsList;				
				TagAndValuePair.Value.GetValue().ParseIntoArray(DimensionsList, TEXT("x"), true);  // value is "512x512"
				if (DimensionsList.Num() == 2)  // make sure we split correctly into 2 items. unreal crashes if index is out of range for whatever unlikely reason
				{
					int width = FCString::Atoi(*DimensionsList[0]);
					int height = FCString::Atoi(*DimensionsList[1]);
					JSONDict->SetNumberField(TEXT("width"), width);
					JSONDict->SetNumberField(TEXT("height"), height);
					UE_LOG(LogPromethean, Display, TEXT("Width: %i"), width);
					UE_LOG(LogPromethean, Display, TEXT("Height: %i"), height);
				}
			}
			else if (TagAndValuePair.Key.ToString() == "HasAlphaChannel")
			{
				// ---- Masked or Not
				int has_alpha_int_bool = 0;
				if (TagAndValuePair.Value.GetValue() == "True")
					has_alpha_int_bool = 1;
				JSONDict->SetNumberField(TEXT("has_alpha"), has_alpha_int_bool);
				UE_LOG(LogPromethean, Display, TEXT("Has Alpha: %i"), has_alpha_int_bool);
			}
		}		
		// ---- Unique ID - Name		
		JSONDict->SetStringField(TEXT("path"), AssetData.ObjectPath.ToString());
		UE_LOG(LogPromethean, Display, TEXT("Asset path: %s"), *AssetData.ObjectPath.ToString());
		// ---- Name		
		JSONDict->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		// ---- Type
		JSONDict->SetStringField(TEXT("type"), "texture");
		// --- Get source files paths of the asset
		TOptional<FAssetImportInfo> ImportInfo = FAssetSourceFilenameCache::ExtractAssetImportInfo(AssetData);
		if (ImportInfo.IsSet())
		{
			FString SourcePaths;
			for (const auto& File : ImportInfo->SourceFiles)
			{
				SourcePaths += " " + File.RelativeFilename;
			}
			JSONDict->SetStringField(TEXT("source_path"), SourcePaths);
			UE_LOG(LogPromethean, Display, TEXT("Source Assets: %s"), *SourcePaths);
		}
		// ---- Thumbnail ---- previews are consistenly blurry and don't update, so have to regenerate every time
		/*
		FName ObjectFullName = FName(*(AssetData.GetFullName()));
		TArray<FName> ObjectFullNames;
		ObjectFullNames.Add(ObjectFullName);
		FThumbnailMap LoadedThumbnails;  // out tmap of <name:thumbnail>
		FObjectThumbnail* ObjectThumbnail = nullptr;
		if (ThumbnailTools::ConditionallyLoadThumbnailsForObjects(ObjectFullNames, LoadedThumbnails))
		{
			ObjectThumbnail = LoadedThumbnails.Find(ObjectFullName);
			if (ObjectThumbnail != NULL && !ObjectThumbnail->IsEmpty())
				ObjectThumbnail->DecompressImageData();  // if success decompress before writing
		}
		else
		{
			// turn AssetData to UTexture2D
			UTexture2D* Texture2D = Cast<UTexture2D>(AssetData.GetAsset());
			ObjectThumbnail = ThumbnailTools::GenerateThumbnailForObjectToSaveToDisk(Texture2D);  // if couldn't load - generate
		}
		*/
		UTexture2D* Texture2D = Cast<UTexture2D>(AssetData.GetAsset());
		FObjectThumbnail* ObjectThumbnail = ThumbnailTools::GenerateThumbnailForObjectToSaveToDisk(Texture2D);  // if couldn't load - generate
		// ---- If got thumbnail - save
		if (ObjectThumbnail != NULL && !ObjectThumbnail->IsEmpty())  
		{
			UE_LOG(LogPromethean, Display, TEXT("Thumbnail Found. Size: %i"), ObjectThumbnail->GetCompressedDataSize());
			FString FileName = AssetData.ObjectPath.ToString();
			FileName = FileName.Replace(TEXT("/"), TEXT("--"));
			FString OutputPath = OutFolderPath + "/thumbnails/" + FileName;
			OutputPath = SaveThumbnail(ObjectThumbnail, OutputPath);  // will add extension at the end
			JSONDict->SetStringField(TEXT("thumbnail"), OutputPath);
		}

		// Get source file path of the asset
		JSONDict->SetStringField(TEXT("source_path"), GetJSONImportSourceDataFromAsset<UTexture>(AssetData.GetAsset()->GetPathName()));
	}
	return JSONDict;
}


TSharedPtr<FJsonObject> getLevelAssetData(FAssetData AssetData)
{
	TSharedPtr<FJsonObject> JSONDict = MakeShareable(new FJsonObject);
	// UE_LOG(LogPromethean, Display, TEXT("Data Type: %s"), *AssetData.AssetClass.ToString());
	if (AssetData.AssetClass.ToString() == "World")
	{
		for (const auto& TagAndValuePair : AssetData.TagsAndValues)
		{	// tags include: NumReplicatedProperties, BlueprintPath, PrimaryAssetType, PrimaryAssetName, DateModified - nothing we need really
			// UE_LOG(LogPromethean, Display, TEXT("Tag NameValue Pair: %s : %s"), *TagAndValuePair.Key.ToString(), *TagAndValuePair.Value);
		}
		// ---- Thumbnail - do thumbnail sooner so it has more time to save
		FString FileName = AssetData.ObjectPath.ToString();
		FileName = FileName.Replace(TEXT("/"), TEXT("--"));
		FString OutputPath = OutFolderPath + "/thumbnails/" + FileName + ".png";  // needs to be bmp as expected by standalone promethean app
		CaptureScreenshot(OutputPath);
		JSONDict->SetStringField(TEXT("thumbnail"), OutputPath);
		// ---- Unique ID - Name		
		JSONDict->SetStringField(TEXT("path"), AssetData.ObjectPath.ToString());
		UE_LOG(LogPromethean, Display, TEXT("Asset path: %s"), *AssetData.ObjectPath.ToString());
		// ---- Name		
		JSONDict->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		// ---- Type
		JSONDict->SetStringField(TEXT("type"), "level");
		// ---- Open Level, get assets
		OpenLevel(AssetData.ObjectPath.ToString());				
		// ---- Get All Asset Paths
		TArray<FString> AssetPathArray = GetCurrentLevelArtAssets();
		TArray<TSharedPtr<FJsonValue>> AssetPathJsonArray;
		for (FString AssetPath : AssetPathArray)
			AssetPathJsonArray.Add(MakeShareable(new FJsonValueString(AssetPath)));
		JSONDict->SetArrayField(TEXT("asset_paths"), AssetPathJsonArray);
	}
	return JSONDict;
}


TArray<FString> GetCurrentLevelArtAssets()
{
	UWorld* EditorWorld = GetEditorWorld();  // comes from PromehteanGeneric
	TArray<FString> OutPathArray;
	TArray<AActor*> AllValidActors = GetWorldValidActors(EditorWorld);
	for (AActor* CurrentLearnActor : AllValidActors)
	{
		USceneComponent* SceneComponent = GetValidSceneComponent(CurrentLearnActor);
		// --- skip if actor has no valid static mesh
		if (SceneComponent == nullptr)
			continue;

		auto StaticMeshComponent = Cast<UStaticMeshComponent>(SceneComponent);
		if (StaticMeshComponent != nullptr) {
			UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
			// --- skip empty static meshes which are most likely there to signify semantic groups
			if (StaticMesh->GetPathName() == FString("None"))
				continue;
			// --- art asset path
			OutPathArray.Add(StaticMesh->GetPathName());
		}
	}
	return OutPathArray;
}


TArray<UStaticMeshComponent*> GetStaticMeshComponents(UBlueprint* blueprint)
{   // https://answers.unrealengine.com/questions/140647/get-default-object-for-blueprint-class.html
	TArray<UStaticMeshComponent*> ActorStaticMeshComponents;
	
	if (blueprint && blueprint->SimpleConstructionScript)
	{
		for (auto scsnode : blueprint->SimpleConstructionScript->GetAllNodes())
		{
			if (scsnode)
			{
				if (auto itemComponent = Cast<UStaticMeshComponent>(scsnode->ComponentTemplate))
				{
					ActorStaticMeshComponents.AddUnique(itemComponent);
				}
			}
		}
	}

	return ActorStaticMeshComponents;
}


TSharedPtr<FJsonObject> getBlueprintAssetData(FAssetData AssetData)
{
	TSharedPtr<FJsonObject> JSONDict = MakeShareable(new FJsonObject);
	if (AssetData.AssetClass.ToString() == "Blueprint")
	{
		// AssetData.PrintAssetData();
		// ---- Blueprint Flag
		JSONDict->SetBoolField(TEXT("blueprint"), true);
		// ---- Unique ID - Name
		JSONDict->SetStringField(TEXT("path"), AssetData.ObjectPath.ToString());
		UE_LOG(LogPromethean, Display, TEXT("Asset Path: %s"), *AssetData.ObjectPath.ToString());
		// ---- Name
		JSONDict->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		// ---- Type
		JSONDict->SetStringField(TEXT("type"), "mesh");
		// ---- Vertex Color Channels   // TODO?
		JSONDict->SetNumberField(TEXT("vertex_color_channels"), 1);
		// ---- Source Files
		TOptional<FAssetImportInfo> ImportInfo = FAssetSourceFilenameCache::ExtractAssetImportInfo(AssetData);
		if (ImportInfo.IsSet())
		{
			FString SourcePaths;
			for (const auto& File : ImportInfo->SourceFiles)
			{
				SourcePaths += " " + File.RelativeFilename;
			}
			// JSONDict->SetStringField(TEXT("source_path"), SourcePaths);  // TODO: non english characters prevent json from being written :( 
			UE_LOG(LogPromethean, Display, TEXT("Source Assets: %s"), *SourcePaths);
		}
		// ---- WARNING! Have to load the actual StaticMesh here. Need it to get material paths and pivot offset
		UBlueprint* BlueprintAsset = Cast<UBlueprint>(AssetData.GetAsset());
		TArray<UStaticMeshComponent*> ActorStaticMeshComponents = GetStaticMeshComponents(BlueprintAsset);
		UE_LOG(LogPromethean, Display, TEXT("Blueprint static mesh components: %i"), ActorStaticMeshComponents.Num());
		// get unique static meshes
		TArray<UStaticMesh*> StaticMeshes;
		for (auto StaticMeshComponent : ActorStaticMeshComponents)
		{
			UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
			StaticMeshes.AddUnique(StaticMesh);
		}
		// get attributes
		TArray<int32> NumLods;
		TArray<int32> NumFaces;
		int32 NumVerts = 0;
		int32 NumUVSets = 0;
		int32 NumTris = 0;		
		for (auto StaticMesh : StaticMeshes)
		{
			NumLods.Add(StaticMesh->GetNumLODs());
			NumVerts += StaticMesh->GetNumVertices(0);  // TrianglesCount += StaticMesh->RenderData->LODResources[0].GetNumVertices();
			NumTris += StaticMesh->GetRenderData()->LODResources[0].GetNumTriangles();
			NumUVSets += StaticMesh->GetNumUVChannels(0);
		}
		// ---- Face Count
		UE_LOG(LogPromethean, Display, TEXT("Faces: %i"), NumTris);
		JSONDict->SetNumberField(TEXT("face_count"), NumTris);
		// ---- Vertex Count
		UE_LOG(LogPromethean, Display, TEXT("Vertices: %i"), NumVerts);
		JSONDict->SetNumberField(TEXT("vertex_count"), NumVerts);
		// ---- UV Channels
		UE_LOG(LogPromethean, Display, TEXT("UV Channels: %i"), NumUVSets);
		JSONDict->SetNumberField(TEXT("uv_sets"), NumUVSets);
		// ---- Num Lods
		UE_LOG(LogPromethean, Display, TEXT("Lod Count: %i"), NumLods.Max());
		JSONDict->SetNumberField(TEXT("lod_count"), NumLods.Max());
		// ---- Material Paths
		
		TArray<FString> MaterialPathStringArray;
		for (auto StaticMesh : StaticMeshes)
		{			
			for (FStaticMaterial StaticMaterial : StaticMesh->GetStaticMaterials())
			{
				UE_LOG(LogPromethean, Display, TEXT("Material Used: %s"), *StaticMaterial.MaterialInterface->GetPathName());  // gets material slots on the static mesh
				MaterialPathStringArray.AddUnique(StaticMaterial.MaterialInterface->GetPathName());
			}			
		}
		TArray<TSharedPtr<FJsonValue>> MaterialPathArray;
		for (auto UniqueMatPath : MaterialPathStringArray)
		{
			MaterialPathArray.AddUnique(MakeShareable(new FJsonValueString(UniqueMatPath)));
		}
		JSONDict->SetArrayField(TEXT("material_paths"), MaterialPathArray);
		// ---- Material Count				
		UE_LOG(LogPromethean, Display, TEXT("Material Count: %i"), MaterialPathArray.Num());
		JSONDict->SetNumberField(TEXT("material_count"), MaterialPathArray.Num());

		
		// ---- Size
		FBox CombinedBoundingBox;
		for (auto StaticMesh : StaticMeshes)
		{
			CombinedBoundingBox += StaticMesh->GetBoundingBox();
		}
		UE_LOG(LogPromethean, Display, TEXT("Size: %s"), *CombinedBoundingBox.GetSize().ToString());
		FVector Size = CombinedBoundingBox.GetSize();
		TArray<TSharedPtr<FJsonValue>> MeshScaleArray;
		MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(Size.X)));
		MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(Size.Y)));
		MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(Size.Z)));
		JSONDict->SetArrayField(TEXT("bounding_box"), MeshScaleArray);
		
		// ---- Pivot Offset
		FVector PivotOffset = CombinedBoundingBox.GetCenter();
		PivotOffset[2] = CombinedBoundingBox.Min[2];  // min height
		TArray<TSharedPtr<FJsonValue>> MeshPivotOffsetArray;
		MeshPivotOffsetArray.Add(MakeShareable(new FJsonValueNumber(-PivotOffset.X)));
		MeshPivotOffsetArray.Add(MakeShareable(new FJsonValueNumber(-PivotOffset.Y)));
		MeshPivotOffsetArray.Add(MakeShareable(new FJsonValueNumber(-PivotOffset.Z)));
		JSONDict->SetArrayField(TEXT("pivot_offset"), MeshPivotOffsetArray);		

		// ---- Thumbnail Image ----- cached thumbnails can be consistenly blurry so have to regenerate every time
		FObjectThumbnail* ObjectThumbnail = ThumbnailTools::GenerateThumbnailForObjectToSaveToDisk(BlueprintAsset);  // if couldn't load - generate
		// ---- If got thumbnail - save
		FString FileName = AssetData.ObjectPath.ToString();
		FileName = FileName.Replace(TEXT("/"), TEXT("--"));
		FString OutputPath = OutFolderPath + "/thumbnails/" + FileName;  // define path globally because it's reused for triangle too
		if (ObjectThumbnail != NULL && !ObjectThumbnail->IsEmpty())
		{
			UE_LOG(LogPromethean, Display, TEXT("Thumbnail Found. Size: %i"), ObjectThumbnail->GetCompressedDataSize());
			FString ThumbPath = SaveThumbnail(ObjectThumbnail, OutputPath);  // will add extension at the end
			JSONDict->SetStringField(TEXT("thumbnail"), ThumbPath);
		}
		// ---- Raw Triangles
		/*
		TArray<FVector> Vertexes = GetMeshTrianglePositionsFromLibraryStaticMesh(StaticMesh);
		if (Vertexes.Num() != 0)
		{
			FString VertexString = FVectorArrayToJsonString(Vertexes, TEXT("verts"));
			SaveToFile(OutputPath, VertexString);  // no file format is currently reserved for vertexes
			JSONDict->SetStringField(TEXT("verts"), OutputPath);
		}
		*/
	}
	return JSONDict;
}

TSharedPtr<FJsonObject> getUnknownAssetData()
{
	TSharedPtr<FJsonObject> JSONDict = MakeShareable(new FJsonObject);	
	JSONDict->SetStringField(TEXT("error"), "Unsupported data type");
	return JSONDict;
}

