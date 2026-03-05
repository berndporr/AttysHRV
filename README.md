# AttysHRV

![alt tag](attyshrv.jpg)

An immersive heartrate variability (HRV) biofeedback app for the Oculus Quest 2 and the Attys DAQ.

The app turns the heartrate into a seaside animation where the idea is
that a relaxed person has a regular physiological heartrate variability
which creates large regular waves in the animation. The person can use
deep breathing to create these and in turn relax.

The Attys is
used to record the ECG in realtime and the Oculus then detects
with high precision the heartbeats and calculates the heartrate.

The app is written in C++ using the openXR / openGL framework
which allows realtime processing and animation of the ECG
in the headset.

## Prerequisites

1. unzip the Oculus openXR API

   Edit `app/src/main/cpp/CMakeLists.txt` and point it to the openXR API:
  `set(OCULUS_OPENXR_MOBILE_SDK /home/yourname/ovr_openxr_mobile_sdk)`

2. Clone [AttysComm](https://github.com/berndporr/AttysComm) and modify `app/build.gradle` so that it points to AttysComm.

## Compile & run

Start Android Studio, open AttysHRV and click `run`.
