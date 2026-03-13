echo "[Script] Building cmake files ..."
export MTL_CACHE_ENABLE=0
rm -rf ~/Library/Caches/com.apple.metal
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -S . -B ./build 
echo "[Script] Building project binary ..."
ninja -v -C ./build
echo "[Script] Executing project binary ..."
./build/EXPLORER

if [ -f "./build/EXPLORER" ]; then
    echo "[Script] Keeping binary ..."
		#	We don't want to remove the binary when we want to debug it
		# rm ./build/EXPLORER
else
    echo "[Script] No binary found!"
fi
echo "[Script] Done."
