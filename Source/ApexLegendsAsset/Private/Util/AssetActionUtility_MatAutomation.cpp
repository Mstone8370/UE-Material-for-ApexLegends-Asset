// Fill out your copyright notice in the Description page of Project Settings.


#include "Util/AssetActionUtility_MatAutomation.h"

#include "Engine/SkinnedAssetCommon.h"
#include "Materials/MaterialInstanceConstant.h"

#include "UnrealEd.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "AssetRegistry/AssetRegistryModule.h"

//#include "MaterialUtilities.h"

UAssetActionUtility_MatAutomation::UAssetActionUtility_MatAutomation()
    : MasterMaterialPath(TEXT("/Game/ApexMaterials/M_Master_Opaque_Subsurface"))
    , EyeCorneaMaterialPath(TEXT("/Game/ApexMaterials/MI_eyecornea"))
    , EyeShadowMaterialPath(TEXT("/Game/ApexMaterials/MI_wraith_base_eyeshadow"))
    , DefaultTextureFolderName(TEXT("Textures"))
{
    // TextureTypeToParamName
    TextureTypeToParamName.Empty();

    TextureTypeToParamName.Add(TEXT("albedoTexture"), FName("Albedo"));
    TextureTypeToParamName.Add(TEXT("aoTexture"), FName("AO"));
    TextureTypeToParamName.Add(TEXT("cavityTexture"), FName("Cavity"));
    TextureTypeToParamName.Add(TEXT("emissiveTexture"), FName("Emissive"));
    TextureTypeToParamName.Add(TEXT("glossTexture"), FName("Gloss"));
    TextureTypeToParamName.Add(TEXT("normalTexture"), FName("Normal"));
    TextureTypeToParamName.Add(TEXT("opacityMultiplyTexture"), FName("Opacity"));
    TextureTypeToParamName.Add(TEXT("scatterThicknessTexture"), FName("ScatterThickness"));
    TextureTypeToParamName.Add(TEXT("specTexture"), FName("Specular"));

    // LinearTextureTypes
    LinearTextureTypes.Empty();
    
    LinearTextureTypes.Add(TEXT("aoTexture"));
    LinearTextureTypes.Add(TEXT("opacityMultiplyTexture"));
    LinearTextureTypes.Add(TEXT("cavityTexture"));
    LinearTextureTypes.Add(TEXT("glossTexture"));
}

void UAssetActionUtility_MatAutomation::AutoTextureMapping(FString TextureFolderNameOverride)
{
    TArray<FAssetData> SelectedAssetDatas = UEditorUtilityLibrary::GetSelectedAssetData();
    for (FAssetData& SelectedAssetData : SelectedAssetDatas)
    {
        USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(SelectedAssetData.GetAsset());
        if (!SkeletalMesh)
        {
            continue;
        }

        const FString SelectedAssetObjectPath = SelectedAssetData.GetObjectPathString();
        const FString AssetFolderPath = FPaths::GetPath(SelectedAssetObjectPath);
        const FString TextureFolderName = TextureFolderNameOverride.Len() > 0 ? TextureFolderNameOverride : DefaultTextureFolderName;
        const FString TextureFolderPath = FPaths::ConvertRelativePathToFull(AssetFolderPath, TextureFolderName);

        TMap<FString, UMaterialInstance*> MaterialTypeMap;

        // Set Skeletal Mesh's materials
        SetMaterialInstances(SkeletalMesh, MaterialTypeMap);

        // Check if Texture Folder exists
        if (!UEditorAssetLibrary::DoesDirectoryExist(TextureFolderPath))
        {
            UE_LOG(LogTemp, Error, TEXT("Texture Folder [%s] not exist"), *TextureFolderPath);
            continue;
        }

        // Map Textures
        MapTextures(MaterialTypeMap, TextureFolderPath);
    }
}

void UAssetActionUtility_MatAutomation::SetMaterialInstances(USkeletalMesh* SkeletalMesh, TMap<FString, UMaterialInstance*>& OutMaterialTypeMap)
{
    UMaterialInterface* MasterMaterial = Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(MasterMaterialPath));
    UMaterialInterface* EyeCorneaMaterial = Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(EyeCorneaMaterialPath));
    UMaterialInterface* EyeShadowMaterial = Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(EyeShadowMaterialPath));

    TArray<FSkeletalMaterial>& MaterialList = SkeletalMesh->GetMaterials();
    for (FSkeletalMaterial& Material : MaterialList)
    {
        FName MaterialSlotName = Material.MaterialSlotName;
        if (MaterialSlotName.IsEqual(FName("wraith_base_eyecornea")))
        {
            Material.MaterialInterface = EyeCorneaMaterial;
            continue;
        }
        if (MaterialSlotName.IsEqual(FName("wraith_base_eyeshadow")))
        {
            Material.MaterialInterface = EyeShadowMaterial;
            continue;
        }

        UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(Material.MaterialInterface);
        if (!MaterialInstance)
        {
            // Create New Material Instance
            const FString NewMaterialInstanceName = FString("MI_") + MaterialSlotName.ToString();
            const FString BasePath = FPaths::GetPath(SkeletalMesh->GetPathName());

            MaterialInstance = CreateMaterialInstance(
                MasterMaterial,
                FPaths::ConvertRelativePathToFull(BasePath, NewMaterialInstanceName)
            );

            if (!MaterialInstance)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to create Material Instance: %s"), *NewMaterialInstanceName);
                continue;
            }

            // Set Material as New Material Instance
            Material.MaterialInterface = MaterialInstance;
        }

        // Add to map for texture mapping
        TArray<FString> ParsedName;
        MaterialSlotName.ToString().ParseIntoArray(ParsedName, TEXT("_"));
        const FString MaterialType = ParsedName.Last();
        OutMaterialTypeMap.Add(MaterialType, MaterialInstance);
    }

    // Save Skeletal Mesh
    const FString SkeletalMeshObjectPath = SkeletalMesh->GetPathName();
    const FString SkeletalMeshPath = FPaths::GetBaseFilename(SkeletalMeshObjectPath, false);
    UEditorAssetLibrary::SaveAsset(SkeletalMeshPath, false);
}

UMaterialInstanceConstant* UAssetActionUtility_MatAutomation::CreateMaterialInstance(UMaterialInterface* ParentMaterial, FString FullPath)
{
    UPackage* Package = CreatePackage(*FullPath);
    Package->FullyLoad();

    UMaterialInstanceConstant* MaterialInstanceAsset = NewObject<UMaterialInstanceConstant>(
        Package,
        *FPaths::GetCleanFilename(FullPath),
        RF_Public | RF_Standalone | RF_MarkAsRootSet
    );
    MaterialInstanceAsset->Parent = ParentMaterial;
    
    Package->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(MaterialInstanceAsset);

    FString PackageFileName = FPackageName::LongPackageNameToFilename(
        FullPath,
        FPackageName::GetAssetPackageExtension()
    );

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
    SaveArgs.Error = GError;
    SaveArgs.bForceByteSwapping = true;
    SaveArgs.bWarnOfLongFilename = true;
    SaveArgs.SaveFlags = SAVE_NoError;

    bool bSaved = UPackage::SavePackage(
        Package,
        MaterialInstanceAsset,
        *PackageFileName,
        SaveArgs
    );
    return MaterialInstanceAsset;
}

void UAssetActionUtility_MatAutomation::MapTextures(TMap<FString, UMaterialInstance*>& InMaterialTypeMap, FString TextureFolderPath)
{
    // Get Textures
    UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(nullptr, false, GIsEditor);
    ObjectLibrary->bRecursivePaths = true;
    ObjectLibrary->ObjectBaseClass = UTexture2D::StaticClass();
    ObjectLibrary->LoadAssetDataFromPath(TextureFolderPath);

    TArray<FAssetData> TextureAssetDatas;
    ObjectLibrary->GetAssetDataList(TextureAssetDatas);

    for (const FAssetData& TextureAssetData : TextureAssetDatas)
    {
        const FString TextureAssetObjectPath = TextureAssetData.GetObjectPathString();
        const FString TextureAssetFilePath = FPaths::GetBaseFilename(TextureAssetObjectPath, false);
        const FString TextureAssetName = FPaths::GetBaseFilename(TextureAssetFilePath);

        TArray<FString> ParsedName;
        int32 ArrayNum = TextureAssetName.ParseIntoArray(ParsedName, TEXT("_"));
        const FString TextureType = ParsedName.Last();
        const FString MaterialType = ParsedName[ArrayNum - 2];

        if (!InMaterialTypeMap.Contains(MaterialType))
        {
            UE_LOG(LogTemp, Warning, TEXT("Material Not Found: %s"), *MaterialType);
            continue;
        }

        // Load Texture
        UTexture2D* Texture = Cast<UTexture2D>(UEditorAssetLibrary::LoadAsset(TextureAssetFilePath));
        if (LinearTextureTypes.Contains(TextureType))
        {
            Texture->SRGB = 0;
        }
        if (TextureType == TEXT("NormalTexture"))
        {
            Texture->LODGroup = TextureGroup::TEXTUREGROUP_WorldNormalMap;
            Texture->CompressionSettings = TextureCompressionSettings::TC_Normalmap;
            Texture->SRGB = 0;
        }

        // Save texture
        UEditorAssetLibrary::SaveAsset(TextureAssetFilePath, false);

        // Set Material Instance Texture Parameter
        FName* ParamName = TextureTypeToParamName.Find(TextureType);
        if (!ParamName)
        {
            UE_LOG(LogTemp, Warning, TEXT("Unknown Texture Type: %s"), *TextureAssetFilePath);
            continue;
        }

        UMaterialInstance* TargetMaterialInstance = *InMaterialTypeMap.Find(MaterialType);

        FMaterialInstanceParameterUpdateContext Context(TargetMaterialInstance);
        FMaterialParameterInfo ParamInfo(*ParamName);
        FMaterialParameterValue Value(Texture);
        FMaterialParameterMetadata Data(Value);
        Context.SetParameterValueEditorOnly(ParamInfo, Data);
    }

    // Save Material Instances
    TArray<FString> MapKeys;
    InMaterialTypeMap.GetKeys(MapKeys);
    for (const FString& Key : MapKeys)
    {
        if (UMaterialInstance* MatInst = *InMaterialTypeMap.Find(Key))
        {
            FString PathName = MatInst->GetPathName();
            UEditorAssetLibrary::SaveAsset(FPaths::GetBaseFilename(PathName, false), false);
        }
    }
}
