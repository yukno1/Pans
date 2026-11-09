.PHONY: build

debug:
	@mkdir -p build/debug
	@cmake -S . -B build/debug # 现代 CMake（3.13+）的推荐写法，等价于 cd build && cmake ..，还避免了 shell 状态的混乱
	@make -C build/debug -j20

release:
	@mkdir -p build/release
	@cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
	@make -C build/release -j20