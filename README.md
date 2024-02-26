# FCPP oldnbr case study

Implementation in FCPP of the `oldnbr-evaluation` case study presented in the paper *An Enhanced Exchange Operator for XC*, submitted to the COORDINATION 2024 event.
All commands below are assumed to be issued from the cloned git repository folder. For any issues, please contact [Giorgio Audrito](mailto:giorgio.audrito@unito.it).

## References

- FCPP main website: [https://fcpp.github.io](https://fcpp.github.io).
- FCPP documentation: [http://fcpp-doc.surge.sh](http://fcpp-doc.surge.sh).
- FCPP presentation paper: [http://giorgio.audrito.info/static/fcpp.pdf](http://giorgio.audrito.info/static/fcpp.pdf).
- FCPP sources: [https://github.com/fcpp/fcpp](https://github.com/fcpp/fcpp).


## Setup

The next sections contain the setup instructions for the various supported OSs. Jump to the section dedicated to your system of choice and ignore the others.

### Windows

Pre-requisites:
- [MSYS2](https://www.msys2.org)
- [Asymptote](http://asymptote.sourceforge.io) (for building the plots)

At this point, run "MSYS2 MinGW x64" from the start menu; a terminal will appear. Run the following commands:
```
pacman -Syu
```
After updating packages, the terminal will close. Open it again, and then type:
```
pacman -Sy --noconfirm --needed base-devel mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-make git
```
The build system should now be available from the "MSYS2 MinGW x64" terminal.

### Linux

Pre-requisites:
- Xorg-dev package (X11)
- G++ 9 (or higher)
- CMake 3.18 (or higher)
- Asymptote (for building the plots)

To install these packages in Ubuntu, type the following command:
```
sudo apt-get install xorg-dev g++ cmake asymptote
```
In Fedora, the `xorg-dev` package is not available. Instead, install the packages:
```
libX11-devel libXinerama-devel.x86_6 libXcursor-devel.x86_64 libXi-devel.x86_64 libXrandr-devel.x86_64 mesa-libGL-devel.x86_64
```

### MacOS

Pre-requisites:
- Xcode Command Line Tools
- CMake 3.18 (or higher)
- Asymptote (for building the plots)

To install them, assuming you have the [brew](https://brew.sh) package manager, type the following commands:
```
xcode-select --install
brew install cmake asymptote
```

### Virtual Machines

If you use a VM with a graphical interface, refer to the section for the operating system installed on it.

**Warning:** the graphical simulations are based on OpenGL, and common Virtual Machine software (e.g., VirtualBox) has faulty support for OpenGL. If you rely on a virtual machine for graphical simulations, it might work provided that you select hardware virtualization (as opposed to software virtualization). However, it is recommended to use the native OS whenever possible.


## Execution

In order to run the case study, type the following command in a terminal:
```
./make.sh gui run -O [options] <target>
```
On newer Mac M1 computers, the `-O` argument may induce compilation errors: in that case, use the `-O3` argument instead.

The `target` can either be:
- `graphic`, in order to run a single simulation with the graphical user interface (GUI), or
- `batch`, in order to execute a batch of 1000 simulations without GUI.

If you also provide `options`, they will be passed to the C++ compiler. Through options you can change the simulation scenario between two usecases:
- *SMALL* (10 nodes in a rectangle area of 150m by side);
- *BIG* (**default**, 100 nodes in a rectangle area of 150m by side). 

In order to launch the *SMALL* usecase, add the option `-DAP_USE_CASE=SMALL`.

Running the above command, you should first see output about building the executables. Then, the graphical simulation should pop up (if using the `graphic` target) while the console will show the most recent `stdout` and `stderr` outputs of the application, together with resource usage statistics (both on RAM and CPU).  During the execution, log files containing the standard input and output will be saved in the `output/` repository sub-folder. For the `batch` target, individual simulation results will be logged in the `output/raw/` subdirectory, with the overall resume in the `output/` directory. After the simulation end, PDF plots will be generated in the `plot/` repository sub-folder.

### Graphical User Interface

Executing a graphical simulation will open a window displaying the simulation scenario, initially still: you can start running the simulation by pressing `P` (current simulated time is displayed in the bottom-left corner). While the simulation is running, network statistics will be periodically printed in the console. You can interact with the simulation through the following keys:

- `Esc` to end the simulation
- `P` to stop/resume
- `O`/`I` to speed-up/slow-down simulated time
- `L` to show/hide connection links between nodes
- `G` to show/hide the grid on the reference plane and node pins
- `M` enables/disables the marker for selecting nodes
- `left-click` on a selected node to open a window with node details
- `C` resets the camera to the starting position
- `Q`,`W`,`E`,`A`,`S`,`D` to move the simulation area along orthogonal axes
- `right-click`+`mouse drag` to rotate the camera
- `mouse scroll` for zooming in and out
- `left-shift` added to the camera commands above for precision control
- any other key will show/hide a legenda displaying this list

Hovering on a node will also display its UID in the top-left corner.

### Case Study Summary

Our goal is to count the number of the battery-powered IoT devices in the network in a scenario
with unstable connections. To achieve this, one of the node will be selected as the
source node where the total number of nodes will be computed.

**TODO: I DON'T UNDERSTAND THE MEANING AND PURPOSE OF THE FOLLOWING TEXT**

Parameters:

- `communication_range`: distance between two nodes that allows their communication
- `battery profile` (HIGH, MEDIUM, LOW): battery setting that affects communication range
- `tvar`: variance of the round durations, as a percentage of the avg

Metrics:

- `sacount` (sum of nodes): sum of nodes computed by source node
