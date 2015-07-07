#pragma once

#include "IBaseChangeInterface.generated.h"

UINTERFACE(MinimalAPI)
class UBaseChangeInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class IBaseChangeInterface
{
	GENERATED_IINTERFACE_BODY()

	/** Called when base actor changes for this actor. */
	UFUNCTION(BlueprintNativeEvent, Category = "Base")
	void BaseChange();

};
