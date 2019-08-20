#pragma once
#include "Core/Math/Constants.h"

namespace Pu
{
	/* Defines the top level types of Human-Interface-Devices. */
	enum class HIDUsagePage
		: uint16
	{
		Undefined = 0x00,
		GenericDesktop = 0x01,
		Simulation = 0x02,
		VR = 0x03,
		Sport = 0x04,
		Game = 0x05,
		GenericDevice = 0x06,
		Keyboard = 0x07,
		LEDs = 0x08,
		Button = 0x09,
		Ordinal = 0x0A,
		Telephony = 0x0B,
		Consumer = 0x0C,
		Digitizer = 0x0D,
		PIDPage = 0x0F,
		Unicode = 0x10,
		AlphanumericDisplay = 0x14,
		MedicalInstruments = 0x40,
		BarCodeScanner = 0x8C,
		Scale = 0x8D,
		MagneticStripeReader = 0x8E,
		CameraControl = 0x90,
		Arcade = 0x91
	};

	/* Defines the usage for generic desktop Human-Interface-Devices. */
	enum class HIDUsageGenericDesktop
		: uint16
	{
		Undefined = 0x00,
		FeatureNotification = 0x47,
		SystemSpeakerMute = 0xA7,
#pragma region Application
		Pointer = 0x01,
		Mouse = 0x02,
		Joystick = 0x04,
		GamePad = 0x05,
		Keyboard = 0x06,
		Keypad = 0x07,
		MultiAxisController = 0x08,
		TabletPcSystemControls = 0x09,
#pragma endregion
#pragma region Axis
		X = 0x30,
		Y = 0x31,
		Z = 0x32,
		Rx = 0x33,
		Ry = 0x34,
		Rz = 0x35,
#pragma endregion
#pragma region Miscellaneous
		Slider = 0x36,
		Dial = 0x37,
		Wheel = 0x38,
		HatSwitch = 0x39,
		CountedBuffer = 0x3A,
		ByteCount = 0x3B,
		MontionWakeup = 0x3C,
		Start = 0x3D,
		Select = 0x3E,
		ResolutionMultiplier = 0x48,
#pragma endregion
#pragma region Vector
		Vx = 0x40,
		Vy = 0x41,
		Vz = 0x42,
		Vbrx = 0x43,
		Vbry = 0x44,
		Vbrz = 0x45,
		Vno = 0x46,
#pragma endregion
#pragma region System
		SystemControl = 0x80,
		SystemPowerDown = 0x81,
		SystemSleep = 0x82,
		SystemWakeUp = 0x83,
		SystemContextMenu = 0x84,
		SystemMainMenu = 0x85,
		SystemAppMenu = 0x86,
		SystemMenuHelp = 0x87,
		SystemMenuExit = 0x88,
		SystemMenuSelect = 0x89,
		SystemMenuRight = 0x8A,
		SystemMenuLeft = 0x8B,
		SystemMenuUp = 0x8C,
		SystemMenuDown = 0x8D,
#pragma endregion
#pragma region Power
		SystemColdRestart = 0x8E,
		SystemWarmRestart = 0x8F,
		SystemHibernate = 0xA8,
#pragma endregion
#pragma region Direction Pads
		DpadUp = 0x90,
		DpadDown = 0x91,
		DpadRight = 0x92,
		DpadLeft = 0x93,
#pragma endregion
		SystemDock = 0xA0,
		SystemUndock = 0xA1,
		SystemSetup = 0xA2,
#pragma region Software Flow
		SystemBreak = 0xA3,
		SystemDebuggerBreak = 0xA4,
		ApplicationBreak = 0xA5,
		ApplicationDebuggerBreak = 0xA6,
#pragma endregion
#pragma region Display
		SystemDisplayInvert = 0xB0,
		SystemDisplayInternal = 0xB1,
		SystemDisplayExternal = 0xB2,
		SystemDisplayBoth = 0xB3,
		SystemDisplayDual = 0xB4,
		SystemDisplayToggleIntExt = 0xB5,
		SystemDisplaySwap = 0xB6,
		SystemDisplayLCDAutoscale = 0xB7,
#pragma endregion
		VendorDefined = 0xFF00	// This is 0xFF00 to 0xFFFF
	};

	/* Defines the usage for simulation Human-Interface-Devices. */
	enum class HIDUsageSimulation
		: uint16
	{
		Undefined = 0x00,
#pragma region Simulation Devices
		FlightSimulationDevice = 0x01,
		AutomobileSimulationDevice = 0x02,
		TankSimulationDevice = 0x03,
		SpaceshipSimulationDevice = 0x04,
		SubmarineSimulationDevice = 0x05,
		SailingSimulationDevice = 0x06,
		MotorcycleSimulationDevice = 0x07,
		SportsSimulationDevice = 0x08,
		AirplaneSimulationDevice = 0x09,
		HelicopterSimulationDevice = 0x0A,
		MagicCarpetSimulationDevice = 0x0B,
		BicycleSimulationDevice = 0x0C,
#pragma endregion
#pragma region General Aviation
		FlightControlStick = 0x20,
		FlightStick = 0x21,
		CyclicControl = 0x22,
		CyclicTrim = 0x23,
		FlightYoke = 0x24,
		Aileron = 0xB0,
		AileronTrim = 0xB1,
		AntiTorqueControl = 0xB2,
		AutopilotEnable = 0xB3,
		ChaffRelease = 0xB4,
		CollectiveControl = 0xB5,
		DiveBrake = 0xB6,
		ElectronicCountermeasures = 0xB7,
		Elevator = 0xB8,
		ElevatorTrim = 0xB9,
		Rudder = 0xBA,
		Throttle = 0xBB,
		FlightCommunications = 0xBC,
		FlareRelease = 0xBD,
		LandingGear = 0xBE,
		ToeBrake = 0xBF,
		Trigger = 0xC0,
		WeaponsArm = 0xC1,
		WeaponsSelect = 0xC2,
		WingFlaps = 0xC3,
#pragma endregion
#pragma region Automobile and Tanks
		Accelerator = 0xC4,
		Brake = 0xC5,
		Clutch = 0xC6,
		Shifter = 0xC7,
		Steering = 0xC8,
		TrackControl = 0x25,
		TurretDirection = 0xC9,
		BarrelElevation = 0xCA,
#pragma endregion
#pragma region Maritime
		DivePlane = 0xCB,
		Ballast = 0xCC,
#pragma endregion
#pragma region Bicycle
		BicycleCrank = 0xCD,
		HandleBars = 0xCE,
		FrontBrake = 0xCF,
		RearBrake = 0xD0
#pragma endregion
	};

	/* Defines the usage for VR controls Human-Interface-Devices. */
	enum class HIDUsageVR
		: uint16
	{
		Undefined = 0x00,
		Belt = 0x01,
		BodySuit = 0x02,
		Flexor = 0x03,
		Glove = 0x04,
		HeadTracker = 0x05,
		HeadMountedDisplay = 0x06,
		HandTracker = 0x07,
		Oculometer = 0x08,
		Vest = 0x09,
		AnimatronicDevice = 0x0A,
		StereoEnable = 0x20,
		DisplayEnable = 0x21
	};
}