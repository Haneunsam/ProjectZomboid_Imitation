# Project Zomboid Imitation - Session Handover Guide

## 1. 현재 진행 상황 (Current Progress)
- **인벤토리 시스템**: `UPZInventoryComponent`에서 무게 계산 및 아이템 목록 관리 완료.
- **UI 시스템**: `BP_InventoryMain` 및 `BP_InventorySlot` 위젯이 C++ 클래스와 바인딩되어 실시간으로 인벤토리를 시각화함.
- **루팅 및 버리기**: 
    - `PZItemActor`: 월드에 배치된 아이템(스태틱 메시 기반)을 감지하고 습득하는 기능.
    - `DropItem`: 인벤토리에서 아이템을 제거하고 캐릭터 앞에 `PZItemActor`를 스폰하는 기능.
- **물리 및 충돌**: 아이템 액터 스폰 시 `PhysicsActor` 프로파일과 `Simulate Physics`를 통해 바닥에 떨어지는 로직 완성.

## 2. 다음 세션을 위한 프롬프트 (AI Prompt)
> "안녕, 나는 Project Zomboid 모작 프로젝트를 진행 중이야. 현재 인벤토리 HUD와 아이템 루팅/버리기 시스템까지 구현이 완료된 상태야. `CurrentStatus.txt`를 읽어서 현재 진행 상황을 파악해줘. 오늘은 3일차 계획인 **아이템 장착(Equip) 시스템**부터 시작하고 싶어. 무기 아이템을 인벤토리에서 사용했을 때 플레이어 캐릭터의 소켓에 메시가 부착되도록 구조를 잡아줘."

## 3. Git LFS 및 대용량 파일 설정 가이드
다른 컴퓨터에서 프로젝트를 처음 클론(Clone)받거나 푸시(Push)할 때 대용량 파일(100MB 이상) 에러를 방지하기 위한 설정입니다.

### [초기 설정 명령어]
터미널(PowerShell/CMD)에서 프로젝트 루트 폴더로 이동 후 아래 명령어를 순서대로 실행하세요.

1. **Git LFS 설치 및 초기화**:
   ```bash
   git lfs install
   ```

2. **언리얼 에셋 추적 설정** (이미 되어 있을 수 있지만 확인용):
   ```bash
   git lfs track "*.uasset"
   git lfs track "*.umap"
   git lfs track "*.exe"
   ```

3. **속도 제한 해제** (업로드 중 끊김 방지):
   ```bash
   git config --global http.postBuffer 524288000
   ```

4. **파일 업로드**:
   ```bash
   git add .
   git commit -m "Add: Inventory and Looting system"
   git push origin main
   ```

### [주의사항]
- 깃허브(GitHub) 무료 계정은 LFS 용량 제한이 있으므로, 에셋이 너무 많아지면 푸시가 실패할 수 있습니다. 그럴 경우 `Binaries`, `Intermediate`, `Saved` 폴더가 `.gitignore`에 잘 등록되어 있는지 꼭 확인하세요.
