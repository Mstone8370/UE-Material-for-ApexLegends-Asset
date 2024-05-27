![main](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/e2b148e8-2442-4c6a-a16a-719ddd93dcd9)

[Full resolution of render image](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/a56f4c9a-36ba-490d-9cf5-48f8502d537c)

# Unreal Engine Material for Apex Legend Asset

A repository for an Unreal Engine plugin containing materials for extracted Apex Legends assets and an auto texture mapping tool.

(Also includes a Post Process Sharpening Material.)

## About this repository's UE project

This project file includes my PostProcessVolume setup and the plugin code.

You can access both the plugin's contents and my PostProcessVolume setup by cloning this repository.

If you only want the plugin, it's recommended to download the zipped plugin file from the [release page](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/releases).

This project's Unreal Engine version is ```5.3```

## Installation

* By clonning this repository:

  **Apex Legends Material** plugin is disabled by default, so you should enable it.

![plugin](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/6ad1df8a-db95-47dd-9fcb-291e45837f59)

* By downloading the plugin from release page:

  Extract the zip file and place the **ApexLegendsMaterial** folder inside your project's ```Plugins``` folder.

  If the ```Plugins``` folder doesn't exist, create a new one.

  After placing the **ApexLegendsMaterial** folder in your project's ```Plugins``` folder, make sure to enable the plugin.

## Usage

* Material

  The Master Material is located in ```Plugins/ApexLegendsMaterial/Materials```.

  Use ```M_Master_AlphaMask_Subsurface``` for general use (e.g. opaque, subsurface scatter, alpha mask(0 or 1)), but use ```M_Master_AlphaBlend``` if translucent materials are needed.

  It is recommended to create instances of these materials and override the settings.

  - For those not using the Auto Texture Mapping tool: Make sure to uncheck sRGB for the AO, cavity, gloss, and alpha mask textures in their respective settings. Set the normal map's ```Texture Group``` to ```WorldNormalMap``` and ```Compression Settings``` to ```Normal map```.

![mat_select](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/8f19c19e-59f3-4963-8fb2-24ea90db1bc7)

* Auto Texture Mapping tool

1. In the Skeletal Mesh's **Asset Details** pannel, disconnect all Materials if any are connected by default.

   If any materials are connected, this tool will try to change only textures of that material.

   And if that material is not a Master Material from this plugin, the texture settings will be fail.

   If no Materials are connected to this Skeletal Mesh, this tool will create new instances of Master Material, and textures will be connected to those material instances.

![slotname](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/e26d26cc-2c46-4575-8382-9e0c07ed1aa3)

3. Create a folder in directory where Skeletal Mesh exists, and import all textures to that folder.

   This tool will search for a folder named ```Textures``` by default, but you can designate another folder if you are texturing recolor skins.

![texturefoldername](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/d52d34d1-3d51-4cb2-9445-a9e3b1aa81b5)

5. **Save all** assets before running the Auto Texture Mapping tool.

6. Right-click the Skeletal Mesh, and select ```Auto Texture Mapping``` inside the **Scripted Asset Actions**.

![right-click](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/23e92291-fe78-489c-83ee-39d6d0e8d021)

![demo_gif](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/c9824a30-231f-4f86-96c0-5ad237994dca)

* Auto Texture Mapping tool with **Recolor Skins**

  Before running the tool, open the Skeletal Mesh and change the ```Slot Name```s of the materials to match the names of recolor skins.

  For example, if the ```Slot Name``` is ```wraith_lgnd_v19_voidwalker_body```, recolor skin's names will be ```wraith_lgnd_v19_voidwalker_[recolor name]_[parts]``` like ```wraith_lgnd_v19_voidwalker_rc01_gear``` and ```wraith_lgnd_v19_voidwalker_rt01_helmet```.

![recolor](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/96de7184-26bd-40c5-8daa-a06da53c5595)

* The settings for the Auto Texture Mapping tool can be changed in the ```BP_AAU_AutoTextureMapping``` Blueprint located in ```Plugins/ApexLegendsMaterial/Util```.

![aau](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/185bb79f-d32a-4dc5-a603-66cad21ff20e)

* You can watch the demo of my workflow.

  [Auto Texture Mapping tool with recolor skins](https://youtu.be/RVcoh0FYdR4)

  [Unreal Engine Sequencer work with animations](https://youtu.be/UpkA9dgYGuA)

## Issues

### Material issues

* Pixelated Artifacts

  If you find pixelated artifacts on metal parts, try overriding the ```MetallicMult``` value in the Material Instance.

  - Artifacts example

![artifact](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/bdb4fe40-12da-4eab-b211-9a0b30bd170c)

![metallic](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/097a5392-b7bb-42b7-9bcb-5b0ba80d1390)

  - Result

![result](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/686efd6a-6c8f-45e9-afa9-0b88fc96555a)

* Hair Color

  If the hair color appears as too dark, turn off ```CavityAffectAlbedo``` checkbox or lower the ```CavityPow``` value in the Material Instance.

  - Hair example

![hair](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/cd517e67-cc77-47ca-a74b-f7a98e83a6e1)

![cavity](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/cb2b18f5-74a9-4ec8-b833-97b3b2865d42)

  or

![cavity_val](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/bcae8eeb-f033-4854-a12d-2dab2dfd12c2)

  - Result

![hair_result](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/9f5a5f6f-4d75-41da-b121-780601ec09b2)

## Auto Texture Mapping tool issue

  If the Auto Texture Mapping tool is not working and error logs are printed in the **Output Log**, the reason might be an old reference issue.

  Try deleting all Material Instances connected to the Skeletal Mesh, and in the content browser, right-click the current folder and select ```Fix Up Redirectors``` to fix the old references.
  
