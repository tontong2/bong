
# 최소 CMake 버전 지정
cmake_minimum_required(VERSION 3.16)

# 프로젝트 이름
project(bong CXX)

# C++17 표준 사용
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# pkg-config 도구로 라이브러리를 찾을 준비
find_package(PkgConfig REQUIRED)

# pkg-config로 SDL2 찾기
pkg_check_modules(SDL2 REQUIRED sdl2)

# SDL2 image 라이브러리 찾기 
pkg_check_modules(SDL2_IMAGE REQUIRED sdl2_image)

# 실행 파일 생성
add_executable(bong src/main.cpp)

# 실행 파일이 SDL2 헤더파일, SDL2 이미지 을 찾을 수 있게 경로 추가
target_include_directories(bong PRIVATE ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

# 실행 파일에 SDL2, SDL2 이미지 연결
target_link_libraries(bong PRIVATE ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})

# SDL2가 요구하는 추가 컴파일 옵션 적용
target_compile_options(bong PRIVATE ${SDL2_CFLAGS_OTHER} ${SDL2_IMAGE_CFLAGS_OTHER})

