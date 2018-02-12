# Instructions for building iOS samples for Azure IoT

#### Prerequisites
 You will need these prerequisites to run the samples:
* The latest version of XCode
* The latest version of the iOS SDK
* [The latest version of CocoaPods](https://guides.cocoapods.org/using/index.html) and familiarity with their basic usage
* An IoT Hub and a connection string for a client device (link needed)

#### 1. Clone the Azure IoT C SDK repo

Change to a location where you would like your samples, and run

`git clone -b ios-samples https://github.com/Azure/azure-iot-sdk-c.git`

It is not necessary to do a recursive clone to build the iOS samples.

#### 2. Make sure XCode doesn't interfere

Make sure that XCode does not already have the sample project open. If
it does, the CocoaPods may not install properly.

#### 3. Navigate to the sample directory

Change your current directory to the sample that you'd like to run. For 
example, the sample for AMQP over WebSockets would be:

`cd build_all/ios/samples/AMQPWS-Client-Sample`

#### 4. Install the CocoaPods

Run this command:

`pod install`

This will cause CocoaPods to read the `Podfile` and install the pods accordingly.

#### 5. Set your connection string

Open the `ViewController.swift` file and replace the `<insert connections string here>` string with
the connection string for your provisioned device.

#### 6. Run the app in the simulator

Open the `<sample name>_WS` workspace file that `pod install` generated, select your
desired target device (iPhone 7 works well), then build and run the project.

Once the project is running in the simulator, click the "Start" hyperlink to begin
sending and receiving messages.


