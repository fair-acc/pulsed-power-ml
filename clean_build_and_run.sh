# Exit script if one command fails
set -e

# Set environment variable
# Note: It is assumed that your emsdk folder lies in the same directory as your pulsed-power-ml folder.
# For expample:
# folder
#  -- emsdk
#  -- pulsed-power-ml
source ../emsdk/emsdk_env.sh

# Delete build folders
sudo rm -rf src/gr-pulsed_power/build
rm -rf src/implot_visualization/build
rm -rf src/opencmw_worker/build

# Delete persistency files
if test src/opencmw_worker/src/data; then
  rm -rf src/opencmw_worker/src/data
fi
if test -f src/opencmw_worker/t.txt; then
  rm src/opencmw_worker/t.txt
fi

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
cmake --build build -j4

cd ..

# Run everything
cd opencmw_worker/
(trap 'kill 0' SIGINT; ./build/src/PulsedPowerService > /dev/null & ./build/src/InferenceTool & (cd ../implot_visualization/ && cmake --build build --target serve))
