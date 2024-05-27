// Copyright (c) 2024 Minseok Kim

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "AAU_AutoTextureMapping.generated.h"

class UMaterialInstanceConstant;
class UMaterialInterface;

UCLASS()
class APEXLEGENDSMATERIAL_API UAAU_AutoTextureMapping : public UAssetActionUtility
{
	GENERATED_BODY()

public:
	UAAU_AutoTextureMapping();

	UFUNCTION(CallInEditor, meta = (DisplayName = "[Apex Legends Material] Auto Texture Mapping"))
	void AutoTextureMapping(UPARAM(DisplayName = "Custom Texture Folder") FString TextureFolderNameOverride);

protected:
	/**
	* Read SkeletalMesh's materials info, find existing Material Instance or create new Material Instance, and set.
	*/
	bool SetMaterialInstances(UObject* Object, TMap<FString, UMaterialInstance*>& OutMaterialNameMap);

	bool SetMaterialInstances_SkeletalMesh(USkeletalMesh* SkeletalMesh, TMap<FString, UMaterialInstance*>& OutMaterialNameMap);

	bool SetMaterialInstances_StaticMesh(UStaticMesh* StaticMesh, TMap<FString, UMaterialInstance*>& OutMaterialNameMap);

	bool LoadEssentialMaterials(UMaterialInterface*& OutMasterMaterial, UMaterialInterface*& OutEyeCorneaMaterial, UMaterialInterface*& OutEyeShadowMaterial);

	UMaterialInstance* CastOrCreateMaterialInstance(UMaterialInterface*& MaterialInterface, const FString& BasePath, const FString& MaterialSlotName, UMaterialInterface* ParentMaterial);

	UMaterialInstanceConstant* CreateMaterialInstance(UMaterialInterface* ParentMaterial, FString FullPath);

	void MapTexturesToMaterial(TMap<FString, UMaterialInstance*>& InMaterialNameMap, FString TextureFolderPath);

	UPROPERTY(EditAnywhere, Category = "AutoTextureMapping Setup")
	FString DefaultTextureFolderName;

	FString MasterMaterialPath;

	FString EyeCorneaMaterialPath;

	FString EyeShadowMaterialPath;

	UPROPERTY(EditAnywhere, Category = "AutoTextureMapping Setup|Material Override")
	TObjectPtr<UMaterialInterface> MasterMaterialOverride;

	UPROPERTY(EditAnywhere, Category = "AutoTextureMapping Setup|Material Override")
	TObjectPtr<UMaterialInterface> EyeCorneaMaterialOverride;

	UPROPERTY(EditAnywhere, Category = "AutoTextureMapping Setup|Material Override")
	TObjectPtr<UMaterialInterface> EyeShadowMaterialOverride;

	UPROPERTY(EditAnywhere, Category = "AutoTextureMapping Setup")
	TMap<FString, FName> TextureTypeToParamName;

	UPROPERTY(EditAnywhere, Category = "AutoTextureMapping Setup")
	TSet<FString> LinearTextureTypes;
};
