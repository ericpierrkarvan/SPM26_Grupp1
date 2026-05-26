#pragma once

UENUM(BlueprintType)
enum class EProgressFlag : uint8 //add flag to DevGiveAllProgress if you add/remove
{
	MagneticGunUnlocked UMETA(DisplayName = "Mechanic Has Magnetic Gun"),
	MagneticGunCanSwitchPolarity UMETA(DisplayName = "Magnetic Gun Can Switch Polarity"),
	RobotCanSwitchPolarity UMETA(DisplayName = "Robot Can Switch Polarity"),
	RobotCanHeadLaunch UMETA(DisplayName = "Robot Can Head Launch"),
	None UMETA(DisplayName = "None"),
};