## DFSPH
Divergence-free smoothed particle hydrodynamics fluid simulation in the course TN1008 - Advanced simulation and visualization of fluids in computer graphics.

The project is compiled with a Makefile, however as installation directories depends on the OS will it not be uploaded.

### Results

* Coming soon

### Dependencies

* `OpenGL v3.3+` - graphical context
* `GLM` - mathematical library
* `FreeImage` - saves rendered frames
* `ffmpeg` - generates a video from the output
* `picojson` - loads simulation properties from a json-file


### Compile and Install

1. Download the latest [release](https://github.com/ronjagrosz/DFSPH/releases)
2. Install all dependencies, `picojson.h` has been provided in the rep
3. Create a folder named `frames/` in the `results/` directory
4. Install with `make` with all appropriate flags
5. Change simulation properties through `scene_parameters.json`
6. Run with `./out`