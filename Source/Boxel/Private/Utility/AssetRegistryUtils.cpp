// 


#include "Utility/AssetRegistryUtils.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Gameplay/DeathBringer/Inventory/ResourceDataAsset.h"

TMap<FGameplayTag, const UResourceDataAsset*> UAssetRegistryUtils::AvailableResourceCDOs;

TMap<FGameplayTag, const UResourceDataAsset*> UAssetRegistryUtils::GetResourceCDOs()
{
	//In editor we want to always update available resources in case one was added while the editor was open
#if !WITH_EDITOR
	if (AvailableResourceCDOs.Num() > 0) return AvailableResourceCDOs;
#endif
	
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	const UClass* Class = UResourceDataAsset::StaticClass();
	AssetRegistryModule.Get().GetAssetsByClass(Class->GetClassPathName(), AssetData, true);
	
	for (int i = 0; i < AssetData.Num(); ++i)
	{
		const auto Asset = AssetData[i].GetAsset();
		if (const UResourceDataAsset* ResourceCDO = Cast<UResourceDataAsset>(Asset))
		{
			AvailableResourceCDOs.Add(ResourceCDO->ResourceTag, ResourceCDO);
		}
	}
	
	return AvailableResourceCDOs;
}
