# Delay Intervals
<p float="left">
  <img src="/screenshots/shot4.png" width="49%" />
  <img src="/screenshots/shot3.png" width="49%" /> 
</p>

An audio plugin that can create fully customizable delay patterns, with adjustable volume, panning, and interval between repeats. No longer do you have to rely on stereo crossfeed formulae to create the rhythmic delay effect that you want! There are also built-in filters for the delayed sound, and the capability to loop indefinitely. Use this plugin to produce shimmering, bouncing atmospheric effects, or phrenetic, energetic rhythimc patterns. Delay Intervals can be downloaded from the "Releases" section of this repository, or see the **[Installation Guide](#installation-guide)** to build it from source yourself.

## Description
Delay Intervals provides up to 16 stereo "intervals" of delay (the first one being the dry signal), with adjustable delay length and the ability to loop audio from the last interval back into the first one. Delay time between each interval can be synched to the tempo of your DAW, or set independently from 0 to 250 milliseconds. Each interval can have its individual volume adjusted (or muted) for each channel, and audio can be set to fade gradually with each repeat (like partial feedback in a typical delay).

Delay Intervals also provides a 2nd order low-pass and high-pass filter for each channel of audio, that will be applied after each interval. As such, further repeats will be more heavily processed by the filter, and so a mix control is provided to mitigate this. There are also a few "ease of use" controls provided, such as a toggles for setting each channels' filters and intervals to mirror each other, and buttons to copy the interval settings from one channel to the other.

## Built With
* [CMake](https://cmake.org/)
* [JUCE](https://juce.com/)

## Installation Guide
You can download Delay Intervals from the "Releases" section. You can also follow these steps to download the source code and build it yourself.

1. [Download and Install CMake](https://cmake.org/download/)

2. Verify that you have a C++ 20 compatible compiler on your machine

3. Clone this repository onto your local machine with the below command in a dedicated directory

    `git clone https://github.com/Matth-ewe-f/audio-plugin-delay-intervals`

4. Install dependencies by running the below command in the same directory (this may take some time)

    `cmake -S . -B build`

5. Build the plugin by running the below command in the same directory (this may also take some time)

    `cmake --build build`

6. Plugin files will be located in subfolders of `build/plugin/Delay-Intervals_artefacts/` based on plugin format (VST, AU, etc). Choose the one you plan to use and install it as you would any other plugin.

<p float="left">
  <img src="/screenshots/shot1.png" width="49%" />
  <img src="/screenshots/shot2.png" width="49%" /> 
</p>
