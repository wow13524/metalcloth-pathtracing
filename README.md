# metalcloth-pathtracing

**I turned in the project and will most likely no longer maintain this repository. All code is available to reference for those looking for modern metal-cpp references.**

This is a repository to document my journey in learning Metal by writing a cloth simulation and rendering the result in real-time using path tracing. It's also my homework!

This project is written in C++ using metal-cpp bindings for Metal and AppKit. A bit of Objective-C++ is used to bridge features that don't have metal-cpp bindings (MPSSVGFDenoiser).

## Current State

As of Thursday February 29 2024, the project creates a 1920x1080 window and renders a green cloth above an indigo ground plane. The background is an HDR image that can be loaded into scenes. Rays are path traced with 1 samples-per-pixel and 8 bounces. Light is calculated using a Cook-Torrence model.

An SVGFDenoiser is used to produce smoother frames as low as 1 sample-per-pixel.

The cloth is currently simulated using a spring-damper model with the addition of bending springs. The cloth is blown against the wind and simulates aerodynamic drag. Normals are averaged between vertices.

Camera Controls: WASD, QE (down/up)
Cloth Controls: IJKL, UO (down/up)
Wind Toggle: Space

![What the project currently looks like](images/current_state_6.png)

## Prerequisites

- A machine running MacOS (I'm using Sonoma 14.3)
- XCode (I'm using version 15.2)
- `metal` utility available on XCode PATH
    > This can be tested by running `xcrun metal`; if your output looks like this:
    > ```
    > xcrun: error: unable to find utility "metal", not a developer tool or in PATH
    > ```
    > then running the following should make the `metal` utility available: `xcode-select --switch /Applications/Xcode.app/Contents/Developer`

## Building

Building is as simple as running `make` in the project directory.

This compiles the Metal shaders into a library (Shaders.metallib) and the project into an executable (metalcloth).

## Running

Ensuring that the shader library and executable are in the same directory, open Terminal, `cd` into the executable directory, and run `./metalcloth`.
