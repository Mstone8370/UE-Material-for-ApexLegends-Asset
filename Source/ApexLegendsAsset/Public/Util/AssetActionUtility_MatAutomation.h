// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "AssetActionUtility_MatAutomation.generated.h"

class UMaterialInstanceConstant;
class UMaterialInterface;

/**
 * 
 */
UCLASS()
class APEXLEGENDSASSET_API UAssetActionUtility_MatAutomation : public UAssetActionUtility
{
	GENERATED_BODY()
	
public:
	UAssetActionUtility_MatAutomation();

	UFUNCTION(CallInEditor, meta = (DisplayName = "[DEPRECATED] Auto Texture Mapping"))
	void AutoTextureMapping(UPARAM(DisplayName = "Custom Texture Folder") FString TextureFolderNameOverride);

protected:
	/**
	* Read SkeletalMesh's materials info, find existing Material Instance or create new Material Instance, and set.
	*/
	bool SetMaterialInstances(USkeletalMesh* SkeletalMesh, TMap<FString, UMaterialInstance*>& OutMaterialTypeMap);

	bool LoadEssentialMaterials(UMaterialInterface*& OutMasterMaterial, UMaterialInterface*& OutEyeCorneaMaterial, UMaterialInterface*& OutEyeShadowMaterial);

	UMaterialInstanceConstant* CreateMaterialInstance(UMaterialInterface* ParentMaterial, FString FullPath);

	void MapTexturesToMaterial(TMap<FString, UMaterialInstance*>& InMaterialTypeMap, FString TextureFolderPath);

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
