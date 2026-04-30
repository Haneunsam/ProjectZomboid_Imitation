# Project Zomboid Imitation - Session Handover Guide

## 1. 현재 진행 상황 (Current Progress)
- **인벤토리 시스템**: `UPZInventoryComponent`에서 무게 계산 및 아이템 목록 관리 완료.
- **UI 시스템**: `BP_InventoryMain` 및 `BP_InventorySlot` 위젯이 C++ 클래스와 바인딩되어 실시간으로 인벤토리를 시각화함.
- **루팅 및 버리기**: 
    - `PZItemActor`: 월드에 배치된 아이템(스태틱 메시 기반)을 감지하고 습득하는 기능.
    - `DropItem`: 인벤토리에서 아이템을 제거하고 캐릭터 앞에 `PZItemActor`를 스폰하는 기능.
- **물리 및 충돌**: 아이템 액터 스폰 시 `PhysicsActor` 프로파일과 `Simulate Physics`를 통해 바닥에 떨어지는 로직 완성.
- **무기 장착 및 애니메이션 (데이터 주도형 구조)**:
    - `PZItemData`에 무기 타입별 몽타주(공격, 장전) 및 `WeaponAnimLayer` 변수 추가.
    - C++에서 `EquipItem` 실행 시 `LinkAnimClassLayers`를 호출해 무기별 애님 레이어를 동적 연결.
- **상/하체 분리 애니메이션 시스템 (런앤건 기반)**:
    - `Layered blend per bone` (spine_01)을 사용해 하체(이동)와 상체(무기 액션) 완벽 분리.
    - 하체: 로코모션 스테이트 머신(서기, 걷기, 웅크리기) 작동.
    - 상체: `Blend Poses by EPZWeaponType` 노드와 `Blend Poses by bool(bIsAiming)`을 조합하여 무기 종류와 조준 여부에 따라 상체 포즈를 하체와 독립적으로 덮어씌움.
- **웅크리기(Crouch) 로직 (C++)**:
    - CharacterMovement의 `bCanCrouch = true` 설정 및 언리얼 기본 `Crouch() / UnCrouch()`와 `CrouchAction` 입력 바인딩 완료.
- **UI 버그 수정**: 컨텍스트 메뉴(Context Menu) 포인터 댕글링 현상 및 클릭 시 삭제 로직 안정화.

## 2. 다음 세션을 위한 프롬프트 (AI Prompt)
> "안녕, 나는 Project Zomboid 모작 프로젝트를 진행 중이야. 현재 인벤토리와 루팅 시스템을 거쳐, **데이터 기반 무기 장착과 상/하체 분리 애니메이션(런앤건, 웅크리기 포함) 구조**까지 성공적으로 완성했어. `Session_Handover.md`를 읽어 현재 진행 상황을 파악해줘. 오늘은 애니메이션 기반이 마련되었으니, 이어서 **실제 사격 기능(발사체/레이캐스트) 및 장전 로직**을 구현하거나, **각 무기(라이플, 권총)별 애님 레이어 에셋을 구체적으로 세팅**하는 작업을 진행하고 싶어. 어디서부터 시작하면 좋을까?"

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
