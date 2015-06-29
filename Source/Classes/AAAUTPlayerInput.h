#pragma once

#include "UTPlayerInput.h"
#include "AAAUTPlayerInput.generated.h"

UCLASS()
class UAAAUTPlayerInput : public UUTPlayerInput
{
	GENERATED_BODY()

public:
	UAAAUTPlayerInput();

	virtual bool OverrideCustomBinding(TArray<FCustomKeyBinding>& CustomBinds, FName KeyName, FString Command);

};
