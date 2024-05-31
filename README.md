# Antilatency.DisplayStylus.UE.SDK

## Setup

### Add Antilatency SDK
Download [AntilatencySDK](https://github.com/AntilatencySDK/Release_4.3.0/tree/subset-6c532c97373a2056951bb8d53bf6b6be3ba30b5c) to the Plugins directory of your project.

### Add Antilatency.DisplayStylus.UE.SDK
Download this repository to your project's Plugins directory.

## Adding DisplayStylus to the Scene
Check SamplePawn at Content\Blueprints\Samples\.

## Display

The Display component is responsible for connecting to the controller that manages the markers.

### Sync With Physical Display Rotation
Applies rotation to the virtual screen based on the tilt of the physical one **when markers are visible to the Stylus (Alt device)**.

### Show Display Border
Shows the borders of the virtual display.

### Display Properties

- **Screen Position**: Position of the screen relative to the Environment (in centimeters).
- **ScreenX**: X-axis of the environment (screen width in meters / 2f).
- **ScreenY**: Y-axis of the environment (screen height in meters / 2f).

## Display Handle

The Display Handle is the parent component for the Display.

- **Origin X**: Sets the origin point for the virtual display on the X-axis.
- **Origin Y**: Sets the origin point for the virtual display on the Y-axis.
- **Scale Mode**: Sets the scale mode in which the virtual display will operate.

## Device Network Subsystem

[Device Network](https://developers.antilatency.com/Terms/Antilatency_Device_Network_en.html) is the communication link between the application and connected Antilatency devices. It helps to monitor changes in connected devices and provides access to the [nodes](https://developers.antilatency.com/Terms/Node_en.html) of the [device tree](https://developers.antilatency.com/Terms/Antilatency_Device_Network_en.html#Device_tree).

## Stylus Creator

Stylus Creator finds stylus nodes and creates Stylus actors based on the number of connected devices.

### Parameters

- **Required Tags**: Allows adding custom styluses (assembled by the user) based on their Tag properties. These values will be used to search for custom stylus nodes.

### About Custom Styluses

An unlimited number of styluses can be connected. A custom stylus can be assembled, for example, using a Hardware Extension Module and a socket with a tracker. The stylus must have a non-empty Tag property, which can be set in AntilatencyService. In the "Styluses Creator" component, the same Tag must be added to Required Tags for the stylus to be found by the application.

## Stylus

- OnStylusButton: Called with each new state update, even if the state stays the same. If the stylus is disconnected while the button is pressed, it signals the released state.
- OnStylusPoseUpdated: Called after the stylus pose is updated, with the following parameters:
  - **stylusPosition**: The position of the stylus in world space.
  - **stylusRotation**: The rotation of the stylus in world space.
  - **angular velocity**: The angular velocity.
- OnStylusDestroy: Called when the stylus GameObject is deleted.
- GetExtrapolatedPose: The last extrapolated pose received.

---

For more detailed information, refer to the official [Antilatency documentation](https://antilatency.com).