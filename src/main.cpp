#include <SDL2/SDL.h>
#include <iostream>

int main(int argc, char* argv[]){
    // SDL 초기화 : 비디오 기능만 사용
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cerr << "SDL 초기화 실패: " << SDL_GetError() << std::endl;
        return 1;
    }

    // 창 생성 : 제목, 위치, 너비, 높이 옵션
    // SDL_WINDOWPOS_CENTERED: 화면 정중앙에 창 띄움
    // 1280, 720: 창 크기 
    // SDL_WINDOW_SHOWN: 창을 즉시 보이게 함
    SDL_Window* window = SDL_CreateWindow(
        "bong", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN
    );

    if(!window){
        std::cerr << "창 생성 실패 : " << SDL_GetError() << std::endl; 
        SDL_Quit();
        return 1; 
    }

    // 랜더러 생성 : 창에 그림 그리는 도구
    // SDL_RENDERER_ACCELERATED: GPU 가속 사용
    // SDL_RENDERER_PRESENTVSYNC: 화면 주사율에 맞춰 그리기 (화면 찢김 방지)
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if(!renderer){
        std::cerr << "랜더러 생성 실패: " << SDL_GetError() << std::endl; 
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 게임 루프 실행 여부
    bool running = true;
    SDL_Event event;

    // 플레이어 표현(x, y, 너비, 높이)
    SDL_Rect player;
    player.x = 1280/ 2 - 16;
    player.y = 720/ 2 - 16;
    player.w = 32;
    player.h = 32;

    // 이전 프레임 시각 기록 (델타타임 계산)
    Uint64 lastTime = SDL_GetPerformanceCounter();

    while(running){

        // 현재 시각 가져오기
        Uint64 currentTime = SDL_GetPerformanceCounter();

        // 이전 프레임과의 시간 차이를 초 단위로 계산
        // SDL_GetPerformanceCounter(): 아주 정밀한 시간 카운터 값을 가져옴
        // SDL_GetPerformanceFrequency(): 그 카운터가 1초에 몇 번 증가하는지 알려주는 값
        // 두 값을 나누면 이전 프레임에서 지금까지 몇 초 지났는지가 나옴 (보통 0.016초 정도, 60fps 기준)
        float deltaTime = (currentTime - lastTime) / (float)SDL_GetPerformanceFrequency();
        lastTime = currentTime;

        // 큐에 쌓인 모든 이벤트(키보드, 창 닫기) 처리
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                running = false; // 창닫기 버튼을 누르면 종료
            }
        }
        
        // 현재 눌려있는 모든 키 상태를 배열로 가져옴
        const Uint8* keystate = SDL_GetKeyboardState(nullptr);

        // 이동 방향 
        float dx = 0.0f;
        float dy = 0.0f;

        if(keystate[SDL_SCANCODE_W]) dy -= 1.0f; 
        if(keystate[SDL_SCANCODE_S]) dy += 1.0f; 
        if(keystate[SDL_SCANCODE_A]) dx -= 1.0f; 
        if(keystate[SDL_SCANCODE_D]) dx += 1.0f; 

        // 이동 속도
        float speed = 300.0f;

        // 실제 위치 갱신 
        player.x += (int)(dx * speed * deltaTime);
        player.y += (int)(dy * speed * deltaTime);

          // 배경색 지정 
        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);

        // 랜더러를 위에서 지정한 색으로 지움
        SDL_RenderClear(renderer);

        // 플레이어 색상 지정
        SDL_SetRenderDrawColor(renderer, 100, 200, 255, 255);

        // 지정한 샋으로 player 그리기
        SDL_RenderFillRect(renderer, &player);

        // 지금까지 그린 내용을 실제 화면에 표시
        SDL_RenderPresent(renderer);
    }

     // 사용한 리소스들을 역순으로 정리
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
  
    return 0;
}