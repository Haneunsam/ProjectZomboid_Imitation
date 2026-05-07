# Project Zomboid Imitation - Session Handover Guide

---

## 0. Claude에게 — 세션 규칙 (필독)

> **세션 시작 시**: 이 파일을 반드시 먼저 읽고 현재 진행 상황을 파악한 뒤 작업을 시작할 것.
>
> **세션 종료 시** (사용자가 작업을 마쳤다고 하거나 대화가 끝날 때): 이 파일의 해당 섹션을 업데이트할 것.
> - 완료된 작업 → **섹션 1**의 완료 목록에 추가
> - 새로 발견된 버그 및 수정 → **섹션 2**에 기록
> - 미완료 블루프린트 작업 → **섹션 3** 갱신
> - 알려진 이슈 변동 → **섹션 4** 갱신
> - **섹션 5의 "다음 세션 시작 프롬프트"**를 다음 세션에서 이어받을 작업 기준으로 반드시 최신화할 것.

---

## 1. 현재 진행 상황 (Current Progress)

### ✅ 완료된 시스템

#### 인벤토리 & 루팅
- `UPZInventoryComponent`: 무게 계산 및 아이템 목록 관리 완료.
- `BP_InventoryMain` / `BP_InventorySlot` 위젯이 C++ 클래스와 바인딩, 실시간 인벤토리 시각화.
- `PZItemActor`: 월드 배치 아이템 감지 및 습득 (E키, 한 번에 1개 줍기 - 의도적 설계).
- `DropItem`: 인벤토리에서 제거 후 캐릭터 앞에 `PZItemActor` 스폰.
- 컨텍스트 메뉴(Context Menu) 포인터 댕글링 버그 수정 완료.

#### 무기 시스템 & 아이템 데이터 (`PZItemData.h`)
- `EPZWeaponType` 열거형: `None(맨손)`, `Handgun`, `Rifle`, `Shotgun`, `Melee` 5종.
- `EPZEquipmentSlot` 열거형: `Primary`, `Secondary`, `Head`, `Top`, `Bottom`, `Shoes`, `Back`.
- **`UPZItemData` 최종 변수 목록**:
  - `AttackMontage` — 일반 공격/사격
  - `DryFireMontage` — 마지막 탄 발사 (Fire to Dry)
  - `ReloadMontage` — 일반 장전 (탄 남아있을 때)
  - `DryReloadMontage` — 빈 탄창 장전 (Reload from Dry)
  - `ChamberRoundMontage` — 약실 장전 / 펌프액션
  - `bIsAutomaticFire` — 연발 여부 (기본 false)
  - `bCanChangeFireMode` — 격발모드 변경 가능 여부 (소총만 true)
  - `WeaponActorClass` — 손에 붙을 무기 액터 클래스
- **제거된 항목 (의도적 삭제)**:
  - `WeaponAnimLayer` — 블루프린트 Enum 분기로 처리하므로 불필요
  - `PushMontage` — 캐릭터 공용 `UnarmedPushMontage` 하나로 통합
  - `GroundStrikeMontage` — 블루프린트에서 처리하므로 불필요

#### 캐릭터 맨손 공격 (`PZCharacter.h`)
- 캐릭터에 3개의 공용 몽타주 변수 추가 (`Combat|Unarmed` 카테고리):
  - `UnarmedJabMontage` — 기본 잽
  - `UnarmedPushMontage` — 밀치기 (맨손/근접무기 공용)
  - `UnarmedStompMontage` — 쓰러진 적 발로 내려찍기 (맨손/근접무기 공용)

#### 데미지 시스템 (이번 세션 추가)

**`PZItemData.h` — 무기 스탯 추가 (Category: "Weapon Stats")**
- `BaseDamage` — 기본 데미지 (총기: 탄당, 근접: 1회당)
- `BulletInitialSpeed` — 총알 초기 속도 (cm/s, 기본 50000)
- `MaxRange` — 최대 사거리 (cm, 기본 100000=1000m, 샷건 권장 10000~20000)
- `FalloffStartRange` — 데미지 감소 시작 거리 (cm)
- `FalloffEndRange` — 데미지 감소 종료 거리 (cm)
- `MinDamagePercent` — 최소 데미지 비율 (0.0~1.0, 기본 0.6=60%)
- `ShotgunPelletCount` — 산탄 수 (Shotgun 전용, 기본 8)
- `ShotgunSpreadAngle` — 비조준 확산 반각 (도, 기본 10°)
- `ShotgunAimSpreadAngle` — 조준 확산 반각 (도, 기본 4°)
- `MeleeDamage` — 근접무기 전용 데미지 (최종 = Strength + MeleeDamage)

**`PZBulletActor.cpp` — 총알 구현 (신규 파일)**
- Tick에서 직접 이동 (ProjectileMovement 미사용)
- 거리 비례 속도 감쇠: MaxRange 도달 시 초기속도의 30%까지 선형 감소
- OnComponentHit + Sweep 충돌 이중 탐지 → `HandleHit()` 공용 처리
- `CalculateFinalDamage()`: FalloffStart~FalloffEnd 구간 선형 보간
- `IPZDamageInterface` 구현체에 데미지 전달

**`PZCharacter.h/.cpp` — 전투 함수 추가**
- `Strength` (float, 기본 10) — 근접 기본 데미지에 합산
- `CurrentAmmo`, `MaxAmmo` — 탄약 변수 (탄약 소모 로직은 블루프린트에서 처리)
- `BulletActorClass` — BP에서 할당할 총알 클래스
- `FireAction`, `ReloadAction`, `DebugRangeAction` — 입력 액션 변수 추가
- `FireWeapon()` — 총기/샷건 분기, 샷건은 VRandCone으로 랜덤 산탄 스폰
- `SpawnBullet()` — 총알 헬퍼 (InitBullet 호출)
- `MeleeAttack()` — SphereTraceMulti로 전방 150cm 탐색, Strength+MeleeDamage 적용
- `ToggleDebugRanges()` — 디버그 범위 토글 + UI 위젯 on/off
- `DrawDebugRanges()` — 무기 종류별 DrawDebugCircle/Line/Sphere

**`PZRangeDebugWidget.h/.cpp` — 디버그 UI (신규 파일)**
- `UpdateWeaponInfo(UPZItemData*, bIsAiming)` — 무기 이름/데미지/사거리/확산각 표시
- BindWidget 목록: WeaponNameText, DamageText, FalloffStartText, FalloffEndText, MaxRangeText, SpreadText, MinDamageText

**`APZZombieCharacter`** — 이전 세션에서 이미 완성 (AI 없는 허수아비)
- `ReceiveDamage_Implementation` 구현 완료
- `OnDamageReceived`, `OnDeath` BP 이벤트 연결 대기 중

#### 부위별 체력 시스템 (이번 세션 추가) ★ 핵심

**`PZBodyPartType.h`** (신규)
- `EPZBodyPart` enum: None / Head / Torso / LeftArm / RightArm / LeftLeg / RightLeg

**`PZHealthComponent.h/.cpp`** (신규)
- 부위별 체력 + 전체 체력 관리
- 머리/몸통/전체 0 → 즉사
- 몸통 70% 미만 → 출혈 (초당 1 데미지로 몸통+전체 동시 감소)
- 양다리 0 (좀비) → 영구 눕힘
- `LimbEffectThreshold = 0.5` (50% 미만부터 효과)
- `MaxEffectReduction = 0.5` (최대 50% 감소)
- `GetMovementSpeedMultiplier()`, `GetAttackSpeedMultiplier()`, `GetAimSwayMultiplier()`
- `BoneNameToBodyPart()` — UE5 Manny 본 이름 매핑
- 이벤트: `OnDeath`, `OnPartDamaged`, `OnKnockedDown`, `OnGetUp`

**`PZDamageInterface.h`** — `ReceiveDamage`에 `FName HitBoneName` 파라미터 추가

**`PZBulletActor`** — Sweep/OnHit Hit.BoneName을 ReceiveDamage로 전달

**`PZZombieCharacter`** — UPZHealthComponent 부착, 좀비 기본값 (Head=10, Torso=30, Arm=20, Leg=25, Total=80, bIsZombie=true)
- BP 이벤트: `OnDamageReceived(Damage, Loc, Dealer, BoneName)`, `OnDeath(Killer)`, `OnKnockedDown(bPermanent)`, `OnGetUp()`

**`PZCharacter`** — UPZHealthComponent 부착, 플레이어 기본값 (Head=35, Torso=85, Arm=60, Leg=65, Total=200, bIsZombie=false)
- `IPZDamageInterface` 구현 (플레이어도 데미지 받음)
- 거리/상태별 자동 분기 디스패처:
  - **30cm 이내 + 누워있는 좀비** → `StompOrHeadStrike()` (맨손=2, 근접무기=`HeadStrikeDamage` 4)
  - **80cm 이내 + 서있는 좀비** → `PushAttack()` (넉백 임펄스 + 30% 확률 임시 눕힘)
  - **그 외 (150cm 이내)** → `BasicMeleeAttack()` (Strength + MeleeDamage)
- `SpawnBullet()`: 팔 부상 시 VRandCone으로 조준 흔들림 (최대 8°)
- `UpdateMovementSpeed()`: 무게 + 다리 부상 합산 적용
- 새 프로퍼티: StompDamage(2), PushKnockdownChance(0.3), PushRange(80), HeadStrikeRange(30), BasicMeleeRange(150), PushImpulseStrength(800), AimSwayMaxDeg(8)

**`PZItemData.h`** — `HeadStrikeDamage = 4.0f` 추가 (근접무기 머리찍기)

#### 마우스 기반 헤드샷 시스템 (이번 세션 추가)

**작동 원리:**
- 사격 순간 마우스 커서 월드 위치(`GetHitResultUnderCursor`) ↔ 좀비 머리 본 위치 2D 거리 계산
- `ProximityScore = 1 - clamp(MouseDist2D / HeadProximityRadius, 0, 1)`
- `DistanceFactor = 1.0` (사거리 ≤ HeadshotEffectiveRange) → `0.0` (사거리 = MaxRange)에서 선형 감쇠
- `최종 확률 = ProximityScore × DistanceFactor` 굴림
- 성공 시 총알의 `bForceHeadshot=true` → 적중 시 본 이름을 `"head"`로 강제 오버라이드

**관련 변수:**
- `PZItemData.HeadshotEffectiveRange` (기본 5000cm = 50m, 권총 기준)
- `PZCharacter.HeadProximityRadius` (기본 30cm — 마우스가 머리에서 이 거리 이내면 100%)
- `PZBulletActor.bForceHeadshot` (사격 시점에 굴린 결과)

**관련 함수:**
- `PZCharacter::FindPrimaryTargetZombie()` — 커서 아래 좀비 → 전방 5m 가장 가까운 좀비 순으로 탐색
- `PZCharacter::TryComputeHeadshot()` — 확률 계산 + 굴림 (Handgun/Rifle만, 샷건 제외)
- `PZBulletActor::HandleHit()` — `bForceHeadshot` 시 본 이름 → `"head"` 강제 변경

**디버그:**
- `bShowDebugRanges = true` (F1 토글) 시 사격할 때마다 화면에 `[Headshot] Prox=0.85 DistFac=1.00 Chance=85% (3.2m to target)` 출력

#### 웅크리기 (Crouch)
- `CharacterMovement`의 `bCanCrouch = true` 설정.
- `CrouchAction` 입력 바인딩 (Left Ctrl).
- `StartCrouching()` / `StopCrouching()` → 언리얼 기본 `Crouch()` / `UnCrouch()` 호출.

#### 상/하체 분리 애니메이션 구조
- `Layered Blend Per Bone` (`spine_01` 기준) 으로 하체(이동)와 상체(무기 액션) 완벽 분리.
- **상체**: `Blend Poses by EPZWeaponType` → 내부에 `Blend Poses by bool (bIsAiming)` 중첩.
  - **중요**: `Blend Poses by EPZWeaponType` 노드의 `Active Enum Value` 핀에 반드시 `CurrentWeaponType` 변수를 연결해야 무기 전환이 작동함.
  - **중요**: `Default` 핀도 반드시 로코모션 캐시 포즈를 연결해야 A포즈 고착 에러 제거됨.
- **하체**: 로코모션 스테이트 머신 (서기, 걷기, 웅크리기).

---

## 2. 오늘 수정/추가된 코드 버그 수정 (`PZCharacter.cpp`)

### 버그 1: Secondary 슬롯 장착 시 Primary 무기 액터가 파괴되는 문제 ✅ 수정
- `EquipItem` 함수에서 `Primary`와 `Secondary` 처리 블록을 완전히 분리.
- `CurrentWeaponActor`(Primary 손 무기)는 **Primary 슬롯 장착 시에만** 파괴/재생성.
- Secondary 슬롯 장착 시 Primary 액터를 절대 건드리지 않음.

### 버그 2: `UpdateMovementSpeed` 디버그 메시지 매 프레임 스팸 ✅ 수정
- `#if WITH_EDITOR ... #endif`로 감싸서 에디터에서만 표시, 패키징 빌드 미포함.
- `AddOnScreenDebugMessage` ID를 `-1`(중복허용) → **`10`(고정 ID)**으로 변경, 화면 1줄 유지.
- `UE_LOG(Warning)` 도 에디터 전용으로 격리.

### 버그 3: `WeaponType == None`인 아이템이 무기로 판정되는 문제 ✅ 수정
- `bIsHoldingWeapon = true` 고정 → `bIsHoldingWeapon = (Item->WeaponType != EPZWeaponType::None)` 조건부 처리.
- 무기가 아닌 아이템이 Primary 슬롯에 들어가도 무기 소지로 오인되지 않음.
- **애니메이션 상태 업데이트(`CurrentWeaponType`, `CurrentWeaponData`)는 Primary 슬롯 장착 시에만 반영.**

---

## 3. 블루프린트 작업 가이드 (미완료 → 다음 세션 이어서)

### 이번 세션에서 추가된 블루프린트 작업 목록

#### BP 설정 — BP_PZCharacter (이벤트 그래프)
1. **`FireAction` 입력 액션 생성** (IMC에 좌클릭 등록) → `FireAction` 변수에 할당.
2. **`DebugRangeAction` 입력 액션 생성** (F1 등) → `DebugRangeAction` 변수에 할당.
3. **`BulletActorClass`** → `BP_PZBullet` (APZBulletActor 파생 BP) 할당.
4. **`RangeDebugWidgetClass`** → `WBP_RangeDebug` (UPZRangeDebugWidget 파생 BP) 할당.
5. 공격 이벤트 그래프에서 **무기 종류 분기**:
   - **Melee/None** → `MeleeAttack()` C++ 호출
   - **총기** → `FireWeapon()` C++ 호출 → `CurrentAmmo - 1`
   - **총기 탄약 = 0** → `DryFireMontage` 재생 (C++ 호출 생략)

#### BP 설정 — BP_PZBullet (APZBulletActor 파생)
- `CollisionSphere` Profile을 `BlockAllDynamic`으로 유지.
- 총알 메시 선택적으로 할당 (없어도 동작).

#### WBP_RangeDebug (UPZRangeDebugWidget 파생)
- 다음 이름으로 `TextBlock` 위젯 생성 (BindWidget):
  - `WeaponNameText`, `DamageText`, `FalloffStartText`, `FalloffEndText`, `MaxRangeText`, `SpreadText`, `MinDamageText`
- 배경 패널은 반투명 검정 권장.
- 화면 좌상단 고정 앵커.

#### BP_Zombie_Dummy (APZZombieCharacter 파생)
- `OnDamageReceived` 이벤트에서 피격 파티클/데미지 숫자 표시.
- `OnDeath` 이벤트에서 래그돌 또는 사망 애니메이션 처리.
- 에디터에서 적당한 위치에 배치 후 테스트.



### A. AnimGraph - 미완료 확인 필요 항목
1. `Blend Poses by EPZWeaponType`의 `Active Enum Value` 핀 → `CurrentWeaponType` 변수 연결 여부 확인.
2. `Default` 핀 → 로코모션 캐시 포즈 연결 여부 확인.
3. `Melee` 핀 추가 → 우클릭 `Add Pin For Element -> Melee` 후 적절한 포즈 연결.

### B. LowerBody 슬롯 애니메이션 (발로 내려찍기 전용)
- 하체에만 모션을 재생하는 방법: **슬롯(Slot) 노드 이용**.
- **설정 순서**:
  1. `Window → Animation Slot Manager`에서 `LowerBody` 슬롯 생성.
  2. 발차기 몽타주(`UnarmedStompMontage`) 파일 열기 → 슬롯 이름을 `LowerBody`로 변경.
  3. `ABP_Manny` AnimGraph에서 `Use Cached Pose (LocoCache)` → `Slot "LowerBody"` → `Layered Blend Per Bone (Base 핀)` 순서로 연결.
  4. 이렇게 하면 Stomp 몽타주 재생 시 상체는 무기 포즈를 유지하고 **하체만 발차기 모션** 재생.

### C. 공격 이벤트 (BP_PZCharactor 이벤트 그래프)
- 좌클릭 발동 → `Switch on EPZWeaponType` 분기:
  - **None (맨손)**:
    - `SphereTrace` → 적 감지 → `Cast to BP_Zombie`
    - 좀비 `bIsDown == true` → `UnarmedStompMontage` (발로 내려찍기)
    - 좀비와 거리 < 70cm → `UnarmedPushMontage` (밀치기)
    - 그 외 → `UnarmedJabMontage` (잽)
  - **Melee (근접무기)**: 동일 거리 분기, 공격은 `CurrentWeaponData.AttackMontage`
  - **Shotgun / Rifle / Handgun**:
    - `CurrentAmmo > 0` → `AttackMontage` 재생 + Ammo 1 감소
    - `CurrentAmmo == 0` → `DryFireMontage` 재생
    - 샷건은 AttackMontage 끝나면 `ChamberRoundMontage` 이어서 재생

### D. 장전 이벤트 (R키)
- `CurrentAmmo == 0` → `DryReloadMontage`
- `CurrentAmmo > 0` → `ReloadMontage`
- 샷건의 경우: 장전 몽타주 안에 **Anim Notify (`AddOneShell`)** 를 탄 넣는 프레임마다 추가 → 블루프린트에서 `AnimNotify_AddOneShell` 이벤트 수신 시 `CurrentAmmo + 1` 처리.

---

## 4. 무기 액터 회전 문제 (알려진 이슈)
- **원인**: `EquipItem` 에서 `SnapToTargetIncludingScale` 규칙 사용 → 무기 Root 회전이 강제 초기화됨.
- **해결책 (권장)**: 무기 블루프린트 구조 변경.
  1. 무기 BP에서 `DefaultSceneRoot` (빈 Scene Component)를 최상단 루트로 설정.
  2. 무기 메시(StaticMesh/SkeletalMesh)를 루트의 자식(Child)으로 배치.
  3. 자식 메시에서 원하는 각도로 조정 후 저장.
  - Root가 0으로 초기화되어도 자식 메시의 상대 오프셋은 보존됨.

---

## 5. 다음 세션 시작 프롬프트

> "안녕, 나는 Project Zomboid 모작 프로젝트를 진행 중이야. `Session_Handover.md` 파일을 먼저 읽어서 현재 진행 상황을 파악해줘. 오늘 이어서 할 작업은:
> 1. **부위별 체력 시스템 BP 연동** — `BP_Zombie_Dummy`의 `OnDamageReceived(BoneName)` / `OnDeath` / `OnKnockedDown(bPermanent)` / `OnGetUp` 이벤트 구현 (피격 파티클, 눕힘 애니메이션, 사망 래그돌 등)
> 2. **HUD UI** — 플레이어 6개 부위 체력바 + 전체 체력바 위젯 만들기 (HealthComponent 이벤트 바인딩)
> 3. **이전 세션 미완료 BP 연결**: `WBP_RangeDebug` UMG 위젯 (TextBlock 7개 이름 맞추기), 탄약 소모/장전 이벤트 그래프
> 4. **공격 모션 분기 BP** — `BP_PZCharacter` 좌클릭에서 C++ `MeleeAttack()` 분기 디스패처가 자동으로 결정한 분기에 맞는 애니메이션 몽타주 재생 연결 (BasicMelee/Push/Stomp/HeadStrike)
> 5. **AI 좀비 기초** — 순찰, 탐지, 추격 (영구 눕힘 시 이동 비활성화)
> 6. **출혈 비주얼** — 화면 가장자리 빨간 비네트 등 (HealthComponent->bIsBleeding 바인딩)"

---

## 6. Git LFS 설정 가이드 (다른 PC에서 클론 시)

```bash
git lfs install
git lfs track "*.uasset"
git lfs track "*.umap"
git lfs track "*.exe"
git config --global http.postBuffer 524288000
git add .
git commit -m "Add: session update"
git push origin main
```

> **주의**: GitHub 무료 계정은 LFS 용량 제한이 있음. `Binaries`, `Intermediate`, `Saved` 폴더가 `.gitignore`에 등록되어 있는지 반드시 확인.
