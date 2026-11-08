.PHONY: build

build:
	@mkdir -p build
	@cmake -S . -B build # 现代 CMake（3.13+）的推荐写法，等价于 cd build && cmake ..，还避免了 shell 状态的混乱
	@make -C build
