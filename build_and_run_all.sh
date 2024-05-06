# Exit script if one command fails
set -e

# Set environment variable
# Note: It is assumed that your emsdk folder lies in the same directory as your pulsed-power-ml folder.
# For expample:
# folder
#  -- emsdk
#  -- pulsed-power-ml
source ../emsdk/emsdk_env.sh

# Build and install GNU Radio blocks
cd src/gr-pulsed_power/
cmake -S . -B build
sudo cmake --build build --target install
sudo ldconfig

cd ..

# Build ImPlot visualization
cd implot_visualization/
emcmake cmake -S . -B build && (cd build && emmake make -j 20)

cd ..

# Build PulsedPowerService and InferencTool
cd opencmw_worker/
cmake -S . -B build
# only next line for small changes (worker)
cmake --build build -j4
cd ..

# Run everything
cd opencmw_worker/
(trap 'kill 0' SIGINT; ./build/src/PulsedPowerService & ./build/src/InferenceTool > /dev/null & (cd ../implot_visualization/ && cmake --build build --target serve))