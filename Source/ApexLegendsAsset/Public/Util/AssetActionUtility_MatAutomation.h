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

	UFUNCTION(CallInEditor)
	void AutoTextureMapping(UPARAM(DisplayName = "Custom Texture Folder") FString TextureFolderNameOverride);

protected:
	/**
	* Read SkeletalMesh's materials info, find existing Material Instance or create new Material Instance, and set.
	*/
	void SetMaterialInstances(USkeletalMesh* SkeletalMesh, TMap<FString, UMaterialInstance*>& OutMaterialTypeMap);

	UMaterialInstanceConstant* CreateMaterialInstance(UMaterialInterface* ParentMaterial, FString FullPath);

	void MapTexturesToMaterial(TMap<FString, UMaterialInstance*>& InMaterialTypeMap, FString TextureFolderPath);

	UPROPERTY(EditAnywhere)
	FString MasterMaterialPath;

	UPROPERTY(EditAnywhere)
	FString EyeCorneaMaterialPath;

	UPROPERTY(EditAnywhere)
	FString EyeShadowMaterialPath;

	UPROPERTY(EditAnywhere)
	FString DefaultTextureFolderName;

	UPROPERTY(EditAnywhere)
	TMap<FString, FName> TextureTypeToParamName;

	UPROPERTY(EditAnywhere)
	TSet<FString> LinearTextureTypes;
};
