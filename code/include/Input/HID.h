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

	/* Defines the usages for generic desktop Human-Interface-Devices. */
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

	/* Defines the usages for simulation Human-Interface-Devices. */
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

	/* Defines the usages for VR controls Human-Interface-Devices. */
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

	/* Defines the usages for game controls Human-Interface-Devices. */
	enum class HIDUsageGame
		: uint16
	{
		Undefined = 0x00,
#pragma region 3D Game Controller
		Controller3D = 0x01,
		PoV = 0x20,
		TurnRightLeft = 0x21,
		PitchForwardBackward = 0x22,
		RollRightLeft = 0x23,
		MoveRightLeft = 0x24,
		MoveForwardBackward = 0x25,
		MoveUpDown = 0x26,
		LeanRightLeft = 0x27,
		LeanForwardBackward = 0x28,
		HeightPoV = 0x29,
#pragma endregion
#pragma region Pinball Device
		PinballDevice = 0x02,
		Flipper = 0x2A,
		SecondaryFlipper = 0x2B,
		Bump = 0x2C,
		NewGame = 0x2D,
		ShootBall = 0x2E,
		Player = 0x2F,
#pragma endregion
#pragma region Gun Device
		GunDevice = 0x03,
		GunBolt = 0x30,
		GunClip = 0x31,
		GunSelector = 0x32,
		GunSingleShot = 0x33,
		GunBurst = 0x34,
		GunAutomatic = 0x35,
		GunSafety = 0x36,
#pragma endregion
		GamepadFireJump = 0x37,
		GamepadTrigger = 0x39
	};

	/* Defines the usages for digitizer Human-Interface-Devices. */
	enum class HIDUsageDigitizer
		: uint16
	{
		Undefined = 0x00,
#pragma region Digitizer Devices
		Digitizer = 0x01,
		Pen = 0x02,
		LightPen = 0x03,
		TouchScreen = 0x04,
		TouchPad = 0x05,
		WhiteBoard = 0x06,
		CoordinateMeasuringMachine = 0x07,
		Digitizer3D = 0x08,
		StereoPlotter = 0x09,
		ArticulatedArm = 0x0A,
		Armature = 0xB,
		MultiplePointDigitizer = 0x0C,
		FreeSpaceWand = 0x0D,
#pragma endregion
#pragma region Transducer Collection Usages
		Stylus = 0x20,
		Puck = 0x21,
		Finger = 0x22,
#pragma endregion
#pragma region Report Field Usages
		TipPressure = 0x30,
		BarrelPressure = 0x31,
		InRange = 0x32,
		Touch = 0x33,
		Untouch = 0x34,
		Tap = 0x35,
		Quality = 0x36,
		DataValid = 0x37,
		TransducerIndex = 0x38,
		TabletFunctionKeys = 0x39,
		ProgramChangeKeys = 0x3A,
		BatteryStrength = 0x3B,
		Invert = 0x3C,
		XTilt = 0x3D,
		YTilt = 0x3E,
		Azimuth = 0x3F,
		Altitude = 0x40,
		Twist = 0x41,
#pragma endregion
#pragma region Switch Usages
		TipSwitch = 0x42,
		SecondaryTipSwitch = 0x43,
		BarrelSwitch = 0x44,
		Eraser = 0x45,
		TabletPick = 0x46
#pragma endregion
	};
}