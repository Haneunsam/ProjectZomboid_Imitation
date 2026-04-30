# Project Zomboid Imitation - 작업 기록 (DevLog)

이 문서는 AI와 함께 작업한 주요 내역을 기록하여, 다음 작업 시 맥락을 빠르게 파악하고 이어가기 위해 작성되었습니다.
새로운 대화를 시작하실 때 "작업 기록(DevLog) 읽고 시작해" 라고 명령해 주시면, 이 파일을 우선적으로 분석한 후 작업을 진행합니다.

---

## [2026-04-29] 무기 장착 및 Left Hand IK(왼손 파지) 애니메이션 구현

### 💡 주요 작업 내역
1. **Locomotion State Machine 개선**
   - `Idle` 및 `Walk/Run` 스테이트 내부에서 `Blend List by Bool` 노드를 사용.
   - `bIsArmed`(무기 장착 여부)와 `bIsAiming`(조준 여부) 변수를 조합하여 맨손, 지향사격(Hip), 정조준(Ironsights) 애니메이션이 자연스럽게 분기 및 전환되도록 구성 완료.

2. **Left Hand IK 좌표 계산 로직 (이벤트 그래프)**
   - 스켈레탈 메시에 종속된 위치를 정확히 구하기 위해 `Get Owning Component`의 World Transform을 기준으로 삼음.
   - 무기(`WeaponSkeletalMesh`)의 `LeftHandSocket`의 World Location을 `Inverse Transform Location`으로 변환하여, 캐릭터 메시 기준의 로컬 좌표(`LeftHandIKLocation`)를 성공적으로 계산 및 변수화.

3. **Two Bone IK 꼬임(Twist) 현상 해결 및 세팅 (애님 그래프)**
   - **문제:** 초기 세팅 시 왼손이 엉뚱한 곳으로 가고 팔꿈치가 꼬이는 현상 발생.
   - **해결 방안 적용:**
     - `Effector Location Space`를 `Component Space`로 지정하고 계산된 `LeftHandIKLocation` 변수를 연결.
     - 팔꿈치가 꼬이는 방향성 상실 문제를 해결하기 위해, `Joint Target Location Space`를 `Bone Space`(`upperarm_l`)로 맞추고 오프셋 값(예: Y: -50)을 주어 팔꿈치가 바깥을 향하도록 수정 완료.

### 📌 향후 작업 시 참고 사항
- 현재 애니메이션 시스템은 무기를 `오른손`에 부착(Attach)하고, `왼손`은 무기의 `LeftHandSocket` 위치를 IK로 추적하는 구조입니다.
- 새로운 무기를 추가할 경우, 해당 무기 블루프린트/스켈레탈 메시에 반드시 `LeftHandSocket`을 추가하고 위치를 잡아주어야 IK가 정상 작동합니다.
