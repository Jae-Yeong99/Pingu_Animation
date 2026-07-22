# Pingu Desktop Pet
<img width="1026" height="527" alt="image" src="https://github.com/user-attachments/assets/bdd1b168-ef87-4880-ad7e-74840db9ac41" />

`Pingu`는 Windows 화면 위를 걸어 다니는 가벼운 데스크톱 펫 프로그램입니다. 투명한 항상-위 창에서 펭귄이 발을 번갈아 움직이고 양 날개를 파닥거리며 화면 하단을 왕복합니다.

Pingu의 창은 마우스 입력을 통과시키므로 VS Code, 브라우저, 게임 등 뒤에 있는 프로그램을 그대로 조작할 수 있습니다.

## 주요 기능

- 4프레임 걷기 및 양 날개 파닥임 애니메이션
- 화면 가장자리에서 진행 방향 전환
- 다른 창 위에 표시되는 투명 배경
- 마우스 클릭 통과
- 전역 단축키를 이용한 일시 정지 및 종료
- 작업표시줄 알림 영역의 트레이 메뉴를 이용한 종료
- 외부 게임 엔진 없이 Win32와 GDI+로 구현

## 프로젝트 구조

```text
windowAnimation/
├─ src/
│  └─ main.cpp                  # 프로그램 소스 코드
├─ assets/
│  ├─ penguin_walk.png          # 앱에서 사용하는 투명 스프라이트
│  └─ penguin_walk_chroma.png   # 배경 제거 전 크로마키 원본
├─ build/
│  ├─ DesktopPet.exe            # 빌드된 실행 파일
│  └─ assets/
│     └─ penguin_walk.png       # 실행 시 불러오는 스프라이트
├─ build.cmd                    # C:\MinGW 경로용 빌드 스크립트
├─ build.ps1                    # PATH에 등록된 g++용 빌드 스크립트
└─ README.md
```

## 실행 환경

### 프로그램만 실행하는 경우

- Windows 10 또는 Windows 11
- `DesktopPet.exe`와 `assets` 폴더가 함께 있어야 합니다.
- 현재 빌드에는 다음 MinGW 런타임 DLL이 필요합니다.
  - `libgcc_s_dw2-1.dll`
  - `libstdc++-6.dll`

개발 PC처럼 `C:\MinGW\bin`이 시스템 `PATH`에 등록되어 있다면 별도 작업 없이 실행할 수 있습니다. 다른 PC로 복사할 때 DLL 오류가 발생하면 위 두 DLL을 `DesktopPet.exe`와 같은 폴더에 복사합니다.

```powershell
Copy-Item C:\MinGW\bin\libgcc_s_dw2-1.dll build\
Copy-Item C:\MinGW\bin\libstdc++-6.dll build\
```

배포하거나 다른 PC로 옮길 때는 다음 구조를 유지합니다.

```text
Pingu/
├─ DesktopPet.exe
├─ libgcc_s_dw2-1.dll
├─ libstdc++-6.dll
└─ assets/
   └─ penguin_walk.png
```

### 소스에서 빌드하는 경우

필요한 도구는 다음과 같습니다.

- Windows 10 또는 Windows 11
- C++17을 지원하는 MinGW `g++`
- PowerShell 또는 명령 프롬프트

이 프로젝트는 현재 `C:\MinGW\bin\g++.exe`로 빌드가 검증되었습니다.

MinGW가 다른 경로에 설치되어 있다면 해당 `bin` 폴더를 사용자 또는 시스템 `PATH`에 추가합니다. 새 터미널을 연 뒤 다음 명령으로 설정을 확인합니다.

```powershell
g++ --version
```

버전 정보가 출력되면 준비가 완료된 것입니다. GDI32, User32, Shell32, GDI+는 Windows 시스템 라이브러리이므로 별도 패키지 설치가 필요하지 않습니다.

## 빌드 방법

### 방법 1: `C:\MinGW`에 설치된 경우

프로젝트 폴더에서 다음 명령을 실행합니다.

```powershell
.\build.cmd
```

### 방법 2: `g++`가 PATH에 등록된 경우

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
```

빌드가 성공하면 다음 파일들이 생성됩니다.

```text
build\DesktopPet.exe
build\assets\penguin_walk.png
```

## 실행 방법

프로젝트 폴더에서 다음 명령을 실행합니다.

```powershell
.\build\DesktopPet.exe
```

또는 파일 탐색기에서 `build\DesktopPet.exe`를 더블 클릭합니다. Pingu는 화면 하단에 나타나며 별도의 일반 창이나 콘솔은 표시하지 않습니다.

## 조작법

| 조작 | 기능 |
|---|---|
| `Ctrl + Alt + P` | 애니메이션 일시 정지 또는 재개 |
| `Ctrl + Alt + Q` | 프로그램 종료 |
| 트레이 아이콘 오른쪽 클릭 → `Exit Desktop Pet` | 프로그램 종료 |

트레이 아이콘이 바로 보이지 않으면 작업표시줄 알림 영역의 숨겨진 아이콘 화살표를 확인하세요.

## 문제 해결

### `g++`를 찾을 수 없다는 오류

MinGW의 `bin` 폴더가 `PATH`에 등록되어 있는지 확인합니다. `build.cmd`는 기본적으로 `C:\MinGW\bin\g++.exe`를 사용합니다.

### `libgcc_s_dw2-1.dll` 또는 `libstdc++-6.dll` 오류

`C:\MinGW\bin`을 `PATH`에 추가하거나, 해당 DLL을 `DesktopPet.exe`와 같은 폴더에 복사합니다.

### 실행되지만 Pingu가 보이지 않음

다음 파일이 존재하는지 확인합니다.

```text
build\assets\penguin_walk.png
```

실행 파일은 자신의 위치를 기준으로 `assets\penguin_walk.png`를 찾으므로 폴더 구조를 변경하면 스프라이트를 불러올 수 없습니다.

### 종료 방법
<img width="182" height="74" alt="image" src="https://github.com/user-attachments/assets/aaf68897-0fb9-4698-873c-1c51706f7c5f" />


`작업표시줄의 Pingu 트레이 아이콘을 오른쪽 클릭한 뒤 `Exit Desktop Pet`을 선택합니다.

## 기술 정보

- 언어: C++17
- 플랫폼: Windows
- 창 및 입력: Win32 API
- PNG 및 알파 렌더링: GDI+
- 트레이 아이콘: Shell API
- 창 크기: 210 × 180 픽셀
- 애니메이션 타이머: 약 60 FPS
