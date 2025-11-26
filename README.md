# MobileSim

간단한 **C++ 텍스트 기반 비행 시뮬레이터** 예제입니다. 콘솔에서 요/피치/롤과 스로틀을 조작하며 링을 통과해 점수를 얻을 수 있습니다.

## 특징
- `std`만 사용하는 순수 C++17 텍스트 시뮬레이터
- 간단한 양력/항력/중력 모델, 스로틀/피치/요/롤 입력 반영
- 고도·속도·연료·점수·남은 링을 실시간 HUD로 표시
- 난수 기반 링 배치와 롤-요 연동(뱅킹) 턴 구현

## 실행 방법 (데스크탑)
> **중요:** 먼저 저장소 루트(이 `README.md`가 있는 위치)로 이동하세요. 다른 경로에서 빌드하면 `src/main.cpp`를 찾지 못해 `g++: error: src/main.cpp: No such file or directory`와 같은 오류가 발생합니다.

- **리눅스/맥OS**
  - **자동 빌드 스크립트**: 어느 위치에서든 저장소 안의 `build.sh`를 실행하면 됩니다.
    ```bash
    bash build.sh
    ./flightsim
    ```
  - **수동 빌드**: 루트에서 직접 컴파일하려면 아래와 같이 실행하세요.
    ```bash
    g++ -std=c++17 src/main.cpp -o flightsim
    ./flightsim
    ```

- **윈도우(.exe)**
  - **자동 빌드 스크립트**: 탐색기에서 `build.bat` 더블클릭 또는 터미널에서 실행하면 현재 위치와 상관없이 루트 기준으로 빌드합니다.
    ```bat
    build.bat
    flightsim.exe
    ```
  - **수동 빌드**: 루트에서 다음을 실행하세요.
    ```bat
    g++ -std=c++17 -O2 -static src\main.cpp -o flightsim.exe   # MinGW-w64 예시

    :: 또는 MSVC (cl) 사용 시
    cl /std:c++17 /O2 /EHsc src\main.cpp /Fe:flightsim.exe
    ```
    ```bat
    flightsim.exe
    ```

### CMake 사용 시
```bash
cmake -S . -B build
cmake --build build --config Release
```

### 기본 조작 (한 줄에 여러 개 공백 구분 입력 가능)
- `+`, `t+`, `throttle+` : 스로틀 증가
- `-`, `t-`, `throttle-` : 스로틀 감소
- `w`, `pitch+`, `p+` : 기수 올리기(피치 업)
- `s`, `pitch-`, `p-` : 기수 내리기(피치 다운)
- `a`, `yaw-`, `y-` : 좌선회
- `d`, `yaw+`, `y+` : 우선회
- `q`, `roll-`, `r-` : 좌측 롤
- `e`, `roll+`, `r+` : 우측 롤
- `help` : 도움말 표시, `exit` : 즉시 종료

## 프로젝트 구조
```
├─ src
│  └─ main.cpp   # 텍스트 기반 비행 시뮬레이터 (C++17)
├─ index.html    # 이전 웹 프로토타입(참고용)
├─ src/style.css # 이전 웹 프로토타입 스타일(참고용)
├─ src/main.js   # 이전 웹 프로토타입 스크립트(참고용)
```
