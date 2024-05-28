// Copyright (c) 2024 Minseok Kim


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
    , EyeCorneaMaterialPath(TEXT("/ApexLegendsMaterial/Materials/MI_eyecornea"))
    , EyeShadowMaterialPath(TEXT("/ApexLegendsMaterial/Materials/MI_eyeshadow"))
    , MasterMaterialOverride(nullptr)
{
    // CustomMaterialMap
    CustomMaterialMap.Empty();

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
    // Check Master Material
    MasterMaterial = MasterMaterialOverride;
    if (!MasterMaterial)
    {
        MasterMaterial = Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(MasterMaterialPath));
        if (!MasterMaterial)
        {
            UE_LOG(LogTemp, Error, TEXT("[AutoTextureMapping] Failed to load Master Material"));
            return;
        }
    }

    // Do mapping
    TArray<FAssetData> SelectedAssetDatas = UEditorUtilityLibrary::GetSelectedAssetData();
    for (FAssetData& SelectedAssetData : SelectedAssetDatas)
    {
        UObject* SelectedObject = SelectedAssetData.GetAsset();
        if (!(SelectedObject->IsA<USkeletalMesh>() || SelectedObject->IsA<UStaticMesh>()))
        {
            continue;
        }

        const FString SelectedAssetObjectPath = SelectedAssetData.GetObjectPathString();
        const FString AssetFolderPath = FPaths::GetPath(SelectedAssetObjectPath);
        const FString TextureFolderName = TextureFolderNameOverride.Len() > 0 ? TextureFolderNameOverride : DefaultTextureFolderName;
        const FString TextureFolderPath = FPaths::ConvertRelativePathToFull(AssetFolderPath, TextureFolderName);

        TMap<FString, UMaterialInstance*> MaterialNameMap;

        // Set Skeletal Mesh's materials
        if (!SetMaterialInstances(SelectedObject, MaterialNameMap))
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

bool UAAU_AutoTextureMapping::SetMaterialInstances(UObject* MeshObject, TMap<FString, UMaterialInstance*>& OutMaterialNameMap)
{
    bool bRet = false;
    if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(MeshObject))
    {
        bRet = SetMaterialInstances_SkeletalMesh(SkeletalMesh, OutMaterialNameMap);
    }
    else if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(MeshObject))
    {
        bRet = SetMaterialInstances_StaticMesh(StaticMesh, OutMaterialNameMap);
    }

    // Save Mesh
    const FString ObjectPath = MeshObject->GetPathName();
    const FString FilePath = FPaths::GetBaseFilename(ObjectPath, false);
    UEditorAssetLibrary::SaveAsset(FilePath, false);

    return bRet;
}

bool UAAU_AutoTextureMapping::SetMaterialInstances_SkeletalMesh(USkeletalMesh* SkeletalMesh, TMap<FString, UMaterialInstance*>& OutMaterialNameMap)
{
    // Set Material Instances
    TArray<FSkeletalMaterial>& MaterialList = SkeletalMesh->GetMaterials();
    for (FSkeletalMaterial& Material : MaterialList)
    {
        FName MaterialSlotName = Material.MaterialSlotName;
        if (CustomMaterialMap.Contains(MaterialSlotName))
        {
            Material.MaterialInterface = *CustomMaterialMap.Find(MaterialSlotName);
            continue;
        }

        // Trying cast Material Interface to Material Instance. If failed, create new Material Instance.
        UMaterialInterface* MaterialInterface = Material.MaterialInterface;
        UMaterialInstance* MaterialInstance = CastOrCreateMaterialInstance(
            MaterialInterface,
            FPaths::GetPath(SkeletalMesh->GetPathName()),
            MaterialSlotName.ToString(),
            MasterMaterial
        );
        if (!MaterialInstance)
        {
            continue;
        }

        // Set Material for when a new Material Instance is created
        Material.MaterialInterface = MaterialInstance;

        // Add to map for texture mapping
        OutMaterialNameMap.Add(MaterialSlotName.ToString(), MaterialInstance);
    }
    return true;
}

bool UAAU_AutoTextureMapping::SetMaterialInstances_StaticMesh(UStaticMesh* StaticMesh, TMap<FString, UMaterialInstance*>& OutMaterialNameMap)
{
    // Set Material Instances
    TArray<FStaticMaterial>& MaterialList = StaticMesh->GetStaticMaterials();
    for (FStaticMaterial& Material : MaterialList)
    {
        FName MaterialSlotName = Material.MaterialSlotName;
        if (CustomMaterialMap.Contains(MaterialSlotName))
        {
            Material.MaterialInterface = *CustomMaterialMap.Find(MaterialSlotName);
            continue;
        }

        // Trying cast Material Interface to Material Instance. If failed, create new Material Instance.
        UMaterialInterface* MaterialInterface = Material.MaterialInterface;
        UMaterialInstance* MaterialInstance = CastOrCreateMaterialInstance(
            MaterialInterface,
            FPaths::GetPath(StaticMesh->GetPathName()),
            MaterialSlotName.ToString(),
            MasterMaterial
        );
        if (!MaterialInstance)
        {
            continue;
        }

        // Set Material for when a new Material Instance is created
        Material.MaterialInterface = MaterialInstance;

        // Add to map for texture mapping
        OutMaterialNameMap.Add(MaterialSlotName.ToString(), MaterialInstance);
    }
    return true;
}

UMaterialInstance* UAAU_AutoTextureMapping::CastOrCreateMaterialInstance(UMaterialInterface*& MaterialInterface, const FString& BasePath, const FString& MaterialSlotName, UMaterialInterface* ParentMaterial)
{
    UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(MaterialInterface);
    if (!MaterialInstance)
    {
        // Create New Material Instance
        const FString NewMaterialInstanceName = FString("MI_") + MaterialSlotName;
        const FString MaterialInstanceFullPath = FPaths::ConvertRelativePathToFull(BasePath, NewMaterialInstanceName);

        if (UEditorAssetLibrary::DoesAssetExist(MaterialInstanceFullPath))
        {
            // If Material exists in working folder, use that.
            MaterialInstance = Cast<UMaterialInstance>(UEditorAssetLibrary::LoadAsset(MaterialInstanceFullPath));
        }
        else
        {
            MaterialInstance = CreateMaterialInstance(ParentMaterial, MaterialInstanceFullPath);
        }

        if (!MaterialInstance)
        {
            UE_LOG(LogTemp, Error, TEXT("[AutoTextureMapping] Failed to create Material Instance: %s"), *NewMaterialInstanceName);
            return nullptr;
        }
    }

    return MaterialInstance;
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
        if (LinearTextureTypes.Contains(TextureType) && Texture->SRGB > 0)
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
