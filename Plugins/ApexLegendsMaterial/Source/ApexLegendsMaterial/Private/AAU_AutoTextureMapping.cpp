// Fill out your copyright notice in the Description page of Project Settings.


#include "AAU_AutoTextureMapping.h"

#include "Engine/SkinnedAssetCommon.h"
#include "Materials/MaterialInstanceConstant.h"

#include "Engine/ObjectLibrary.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "AssetRegistry/AssetRegistryModule.h"

UAAU_AutoTextureMapping::UAAU_AutoTextureMapping()
    : DefaultTextureFolderName(TEXT("Textures"))
    , MasterMaterialPath(TEXT("/ApexLegendsMaterial/Materials/M_Master_AlphaMask_Subsurface"))
    , EyeCorneaMaterialPath(TEXT("/ApexLegendsMaterial/Materials/MI_eyecornea_raytracing"))
    , EyeShadowMaterialPath(TEXT("/ApexLegendsMaterial/Materials/MI_eyeshadow"))
    , MasterMaterialOverride(nullptr)
    , EyeCorneaMaterialOverride(nullptr)
    , EyeShadowMaterialOverride(nullptr)
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
    LinearTextureTypes.Add(TEXT("normalTexture"));
}

void UAAU_AutoTextureMapping::AutoTextureMapping(FString TextureFolderNameOverride)
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

        TMap<FString, UMaterialInstance*> MaterialNameMap;

        // Set Skeletal Mesh's materials
        if (!SetMaterialInstances(SkeletalMesh, MaterialNameMap))
        {
            continue;
        }

        // Check if Texture Folder exists
        if (!UEditorAssetLibrary::DoesDirectoryExist(TextureFolderPath))
        {
            UE_LOG(LogTemp, Error, TEXT("[AutoTextureMapping] Texture Folder [%s] not exist"), *TextureFolderPath);
            continue;
        }

        // Map Textures
        MapTexturesToMaterial(MaterialNameMap, TextureFolderPath);
    }
}

bool UAAU_AutoTextureMapping::SetMaterialInstances(USkeletalMesh* SkeletalMesh, TMap<FString, UMaterialInstance*>& OutMaterialNameMap)
{
    // Load Essential Materials
    UMaterialInterface* MasterMaterial = nullptr;
    UMaterialInterface* EyeCorneaMaterial = nullptr;
    UMaterialInterface* EyeShadowMaterial = nullptr;
    if (!LoadEssentialMaterials(MasterMaterial, EyeCorneaMaterial, EyeShadowMaterial))
    {
        return false;
    }

    // Set Material Instances
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
                UE_LOG(LogTemp, Error, TEXT("[AutoTextureMapping] Failed to create Material Instance: %s"), *NewMaterialInstanceName);
                continue;
            }

            // Set Material as New Material Instance
            Material.MaterialInterface = MaterialInstance;
        }

        // Add to map for texture mapping
        OutMaterialNameMap.Add(MaterialSlotName.ToString(), MaterialInstance);
    }

    // Save Skeletal Mesh
    const FString SkeletalMeshObjectPath = SkeletalMesh->GetPathName();
    const FString SkeletalMeshPath = FPaths::GetBaseFilename(SkeletalMeshObjectPath, false);
    UEditorAssetLibrary::SaveAsset(SkeletalMeshPath, false);

    return true;
}

bool UAAU_AutoTextureMapping::LoadEssentialMaterials(UMaterialInterface*& OutMasterMaterial, UMaterialInterface*& OutEyeCorneaMaterial, UMaterialInterface*& OutEyeShadowMaterial)
{
    OutMasterMaterial = MasterMaterialOverride;
    if (!OutMasterMaterial)
    {
        OutMasterMaterial = Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(MasterMaterialPath));
        if (!OutMasterMaterial)
        {
            UE_LOG(LogTemp, Error, TEXT("[AutoTextureMapping] Failed to load Master Material"));
            return false;
        }
    }

    OutEyeCorneaMaterial = EyeCorneaMaterialOverride;
    if (!OutEyeCorneaMaterial)
    {
        OutEyeCorneaMaterial = Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(EyeCorneaMaterialPath));
        if (!OutEyeCorneaMaterial)
        {
            UE_LOG(LogTemp, Error, TEXT("[AutoTextureMapping] Failed to load Eye Cornea Material"));
            return false;
        }
    }

    OutEyeShadowMaterial = EyeShadowMaterialOverride;
    if (!OutEyeShadowMaterial)
    {
        OutEyeShadowMaterial = Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(EyeShadowMaterialPath));
        if (!OutEyeShadowMaterial)
        {
            UE_LOG(LogTemp, Error, TEXT("[AutoTextureMapping] Failed to load Eye Shadow Material"));
            return false;
        }
    }

    return true;
}

UMaterialInstanceConstant* UAAU_AutoTextureMapping::CreateMaterialInstance(UMaterialInterface* ParentMaterial, FString FullPath)
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

void UAAU_AutoTextureMapping::MapTexturesToMaterial(TMap<FString, UMaterialInstance*>& InMaterialNameMap, FString TextureFolderPath)
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

        FString MaterialName = TextureAssetName;
        FString TextureType = "";
        int32 DelimeterIndex;
        if (TextureAssetName.FindLastChar('_', DelimeterIndex))
        {
            MaterialName = TextureAssetName.Left(DelimeterIndex);
            TextureType = TextureAssetName.RightChop(DelimeterIndex + 1);
        }

        if (!InMaterialNameMap.Contains(MaterialName))
        {
            UE_LOG(LogTemp, Warning, TEXT("[AutoTextureMapping] Material Not Found: %s"), *MaterialName);
            continue;
        }

        // Load Texture
        UTexture2D* Texture = Cast<UTexture2D>(UEditorAssetLibrary::LoadAsset(TextureAssetFilePath));
        if (LinearTextureTypes.Contains(TextureType))
        {
            Texture->SRGB = 0;
            if (TextureType == TEXT("NormalTexture"))
            {
                Texture->LODGroup = TextureGroup::TEXTUREGROUP_WorldNormalMap;
                Texture->CompressionSettings = TextureCompressionSettings::TC_Normalmap;
            }

            // Update and save texture
            Texture->UpdateResource();
            UEditorAssetLibrary::SaveAsset(TextureAssetFilePath, false);
        }

        // Set Material Instance Texture Parameter
        FName* ParamName = TextureTypeToParamName.Find(TextureType);
        if (!ParamName)
        {
            UE_LOG(LogTemp, Warning, TEXT("[AutoTextureMapping] Unknown Texture Type: %s"), *TextureAssetFilePath);
            continue;
        }

        UMaterialInstance* TargetMaterialInstance = *InMaterialNameMap.Find(MaterialName);

        FMaterialInstanceParameterUpdateContext Context(TargetMaterialInstance);
        FMaterialParameterInfo ParamInfo(*ParamName);
        FMaterialParameterValue Value(Texture);
        FMaterialParameterMetadata Data(Value);
        Context.SetParameterValueEditorOnly(ParamInfo, Data);
    }

    // Save Material Instances
    TArray<FString> MapKeys;
    InMaterialNameMap.GetKeys(MapKeys);
    for (const FString& Key : MapKeys)
    {
        if (UMaterialInstance* MatInst = *InMaterialNameMap.Find(Key))
        {
            FString PathName = MatInst->GetPathName();
            UEditorAssetLibrary::SaveAsset(FPaths::GetBaseFilename(PathName, false), false);
        }
    }
}
