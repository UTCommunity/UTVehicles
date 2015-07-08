#pragma once

#include "SVehicle.h"
#include "UTVehicleBase.generated.h"

UCLASS(Blueprintable, Abstract, NotPlaceable)
class AUTVehicleBase : public ASVehicle
{
	GENERATED_UCLASS_BODY()

	/** Switches weapons using classic groups. */
	virtual void SwitchWeapon(int32 Group);

	/** Switches weapons using modern groups. */
	virtual void SwitchWeaponGroup(int32 Group);

	/**
	* Request change to adjacent vehicle seat
	*
	* @network	all
	*/
	UFUNCTION(BlueprintCallable, Category = Seats)
	void AdjacentSeat(int32 Direction, AController* C);

	/**
	* Request change to adjacent vehicle seat
	*
	* @network	Server-Side
	*/
	UFUNCTION(reliable, server, WithValidation)
	virtual void ServerAdjacentSeat(int32 Direction, AController* C);

	/**
	* Called when a client is requesting a seat change
	*
	* @network	Server-Side
	*/
	UFUNCTION(reliable, server, WithValidation)
	virtual void ServerChangeSeat(int32 RequestedSeat);

};
