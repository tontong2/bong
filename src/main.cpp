#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>


// 전역 관리

// 창 크기 상수
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720; 

// 플레이어 크기 
const int PLAYER_WIDTH = 50;
const int PLAYER_HEIGHT = 52; 

// 플레이어 기본 체력 (게임 시작 시 초기값)
const int PLAYER_BASE_MAX_HP = 100;


int main(int argc, char* argv[]){
    // SDL 초기화 : 비디오 기능만 사용
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        std::cerr << "SDL 초기화 실패: " << SDL_GetError() << std::endl;
        return 1;
    }

    // SDL_Image 초기화 : PNG 로딩 기능 활성화
    if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image 초기화 실패" << IMG_GetError() << std::endl;
        SDL_Quit(); 
        return 1; 
    }

    // 창 생성 : 제목, 위치, 너비, 높이 옵션
    // SDL_WINDOWPOS_CENTERED: 화면 정중앙에 창 띄움
    // 1280, 720: 창 크기 
    // SDL_WINDOW_SHOWN: 창을 즉시 보이게 함
    SDL_Window* window = SDL_CreateWindow(
        "bong", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN
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

    // 플레이어 체력 설정
    int playerMaxHP = PLAYER_BASE_MAX_HP; // 최대 체력
    int playerCurrentHP = playerMaxHP; // 현재 체력

    // 체력바 설정
    const int HP_BAR_X = 20; 
    const int HP_BAR_Y = 20;
    const int HP_BAR_WIDTH = 200; 
    const int HP_BAR_HEIGHT = 24; 

    // 플레이어 표현(x, y, 너비, 높이)
    SDL_Rect player;
    player.x = WINDOW_WIDTH/ 2 - PLAYER_WIDTH/2;
    player.y = WINDOW_HEIGHT/ 2 - PLAYER_HEIGHT/2;
    player.w = PLAYER_WIDTH;
    player.h = PLAYER_HEIGHT;

    // 이전 프레임 시각 기록 (델타타임 계산)
    Uint64 lastTime = SDL_GetPerformanceCounter();

    // 배경 이미지 불러오기 
    SDL_Texture* backgroundTexture = IMG_LoadTexture(renderer, "assets/background2.png");
    if(!backgroundTexture){
        std::cerr << "배경 이미지 로드 실패" << IMG_GetError();
        return 1; 
    }

    // 플레이어 이미지 불러오기
    SDL_Texture* playerTexture = IMG_LoadTexture(renderer, "assets/mong_idle.png");
    if(!playerTexture){
        std::cerr << "플레이어 이미지 로드 실패" << IMG_GetError();
        return 1; 
    }

    // 걷기 애니메이션 프레임 
    const int WALK_FRAME_COUNT = 5;
    SDL_Texture* walkTextures[WALK_FRAME_COUNT];
    walkTextures[0] = IMG_LoadTexture(renderer, "assets/walk/mong_walk_0.png");
    walkTextures[1] = IMG_LoadTexture(renderer, "assets/walk/mong_walk_1.png");
    walkTextures[2] = IMG_LoadTexture(renderer, "assets/walk/mong_walk_2.png");
    walkTextures[3] = IMG_LoadTexture(renderer, "assets/walk/mong_walk_3.png");
    walkTextures[4] = IMG_LoadTexture(renderer, "assets/walk/mong_walk_4.png");


    for(int i=0; i< WALK_FRAME_COUNT; i++){
        if(!walkTextures[i]){
            std::cerr << "걷기 프레임 로드 실패: " << IMG_GetError() <<std::endl;
            return 1; 
        }
    }


    // 애니메잇션 상태 관리
    int currentWalkFrame = 0;  // 현재 몇번 째 걷기 프레임인지
    float animTimer = 0.0f; // 프레임 전환 타이머
    const float ANIM_FRAME_TIME = 0.15f; // 프레임 하나당 지속 시간 (초)
    bool facingLeft = false; // 마지막 방향 (기본 오른쪽);

    while(running){

        // 현재 시각 가져오기
        Uint64 currentTime = SDL_GetPerformanceCounter();

        // 이전 프레임과의 시간 차이를 초 단위로 계산
        // SDL_GetPerformanceCounter(): 아주 정밀한 시간 카운터 값을 가져옴
        // SDL_GetPerformanceFrequency(): 그 카운터가 1초에 몇 번 증가하는지 알려주는 값
        // 두 값을 나누면 이전 프레임에서 지금까지 몇 초 지났는지가 나옴 (보통 0.016초 정도, 60fps 기준)
        float deltaTime = (currentTime - lastTime) / (float)SDL_GetPerformanceFrequency();
        lastTime = currentTime;

        // 델타타임이 비정상적으로 크게 튀는 걸 방지
        if(deltaTime > 0.01f) deltaTime = 0.01f;

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

        // 화면 경계 안으로 위치 제한 (clamp)
        if(player.x < 0) player.x = 0;
        if(player.y < 0) player.y = 0;
        if(player.x > WINDOW_WIDTH - player.w) player.x = WINDOW_WIDTH - player.w;
        if(player.y > WINDOW_HEIGHT - player.h) player.y = WINDOW_HEIGHT - player.h;

        // 이동 중인지 확인
        bool isMoving = (dx != 0.0f || dy != 0.0f);

        // 좌우 방향 갱신
        if(dx <0) facingLeft = true;
        if(dx >0) facingLeft = false;

        // 애니메이션 프레임 갱신
        
        if(isMoving){ // 방향키가 눌려있는지 
            animTimer += deltaTime;
            // 이동 중이면 타이머를 누적시켜서 해당 시간마다 다음 프레임으로 순환 
            if(animTimer >= ANIM_FRAME_TIME) {
                animTimer = 0.0f;
                currentWalkFrame = (currentWalkFrame + 1) % WALK_FRAME_COUNT; // 계속 반복 할 수 있도록 함 
                
            }
        }else{
            // 멈춰 있으면 애니메이션 초기화
            currentWalkFrame = 0;
            animTimer = 0.0f;
        }

        // 배경 지우기
        SDL_RenderClear(renderer);
        
        // SDL_RenderCopy(renderer, texture, srcRect, dstRect): 텍스처를 화면에 그리는 함수
        // srcRect가 nullptr이면 이미지 전체를 사용
        // dstRect는 화면에서 어디에, 얼마 크기로 그릴지 지정

        SDL_Rect backgroundRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderCopy(renderer, backgroundTexture, nullptr, &backgroundRect);

        // 이동중이면 걷기, 정지 중이면 대기로 표시
        SDL_Texture* currentTexture = isMoving? walkTextures[currentWalkFrame] : playerTexture;

        // 좌우 반전 여부 결정
        SDL_RendererFlip flip = facingLeft ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

        // 회전 없이, 중심점을 기준으로, 좌우 반전 해서 그리기
        SDL_RenderCopyEx(
            renderer,
            currentTexture,
            nullptr, // 이미지 크기 전체
            &player, // 화면에 그릴 위치
            0.0, // 회전 각도
            nullptr, // 회전 중심점
            flip // 좌우반전 여부 
        );

        // ------ 체력바 -------- //
        // 현재 체력 비율 계산 (0.0 ~ 1.0)
        float hpRatio = (float)playerCurrentHP / (float)playerMaxHP; 
        if(hpRatio < 0.0f) hpRatio = 0.0f;
        if(hpRatio < 1.0f) hpRatio = 1.0f;

        // 배경(빈 체력바) 
        SDL_Rect hpBarBackground = { HP_BAR_X, HP_BAR_Y, HP_BAR_WIDTH, HP_BAR_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        SDL_RenderFillRect(renderer, &hpBarBackground);

        // 실제 체력만큼 채워지는 바 
        SDL_Rect hpBarFill = { HP_BAR_X, HP_BAR_Y, (int)(HP_BAR_WIDTH * hpRatio), HP_BAR_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 220, 50, 50, 255);
        SDL_RenderFillRect(renderer, &hpBarFill);

        // 테두리 - 흰색 
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &hpBarBackground);

        // 지금까지 그린 내용을 실제 화면에 표시
        SDL_RenderPresent(renderer);

    }
    
    // 텍스쳐 리소스 정리
    SDL_DestroyTexture(playerTexture);
    SDL_DestroyTexture(backgroundTexture);

    for(int i=0; i<WALK_FRAME_COUNT; i++){
        SDL_DestroyTexture(walkTextures[i]);
    }

    // 사용한 리소스들을 역순으로 정리
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();
  
    return 0;
}