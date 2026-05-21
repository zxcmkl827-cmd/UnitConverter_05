# Test Plan: meter to feet Conversion

## 1. 테스트 대상

- 대상 기능: `meter -> feet` 길이 변환
- 대표 입력: `meter:2.5`
- 기대 결과: `2.5 meter = 8.202100 feet`
- 기준 비율: `1 meter = 3.28084 feet`
- 검증 오차: 내부 변환값 기준 `0.00001` 이내

이 테스트 계획은 README와 PRD에 정의된 기본 길이 변환 계약을 기준으로 한다. 테스트의 핵심은 `meter` 입력값이 `feet`로 정확히 변환되는지, 입력 파싱 및 오류 정책이 변환 전에 올바르게 동작하는지 검증하는 것이다.

## 2. 테스트 목표

- `meter:2.5` 정상 입력이 `8.202100 feet`로 계산되는지 검증한다.
- `meter:0` 경계값이 유효 입력으로 처리되고 결과가 `0 feet`인지 검증한다.
- 음수, 숫자 아님, 형식 오류, 미지원 단위가 변환 전에 실패하는지 검증한다.
- Domain 계층의 변환 비율 회귀를 테스트로 보호한다.
- Boundary 계층의 입력 파싱 및 오류 코드 계약을 테스트로 보호한다.

## 3. Catch2 기반 단위 테스트 범위와 우선순위

| 우선순위 | 영역 | 테스트 범위 | 주요 검증 |
|---|---|---|---|
| P0 | Domain | `meter -> feet` 변환 계산 | `value * 3.28084`, 허용 오차 `0.00001` |
| P0 | Domain | `meter:0` 영값 변환 | 결과가 정확히 `0`이며 예외가 발생하지 않음 |
| P0 | Boundary | 입력 파싱 성공 | `meter:2.5`를 단위 `meter`, 값 `2.5`로 분리 |
| P0 | Boundary | 입력 검증 실패 | 음수, 숫자 아님, 콜론 누락, 미지원 단위 오류 코드 |
| P1 | Control | 파싱된 요청과 Domain 변환 연결 | 정상 요청이 변환 결과로 이어짐 |
| P1 | Output | 기본 표시 문자열 | 원 입력 값과 단위를 보존하고 대상 단위 `feet`를 표시 |
| P2 | Regression | 기준 비율 고정 | `1 meter = 3.28084 feet` 변경 시 테스트 실패 |

P0 테스트는 빌드마다 반드시 실행한다. P1 테스트는 통합 흐름 안정성을 확인하기 위해 PR 검증에 포함한다. P2 테스트는 기준 비율과 출력 계약이 변경되지 않았는지 회귀 보호 목적으로 유지한다.

## 4. 정상 경로 테스트

| ID | 입력 | 기대 결과 | 검증 포인트 |
|---|---|---|---|
| TC-MF-001 | `meter:2.5` | `8.202100 feet` | `2.5 * 3.28084 = 8.20210` |
| TC-MF-002 | `meter:1` | `3.28084 feet` | README 기준 비율 직접 검증 |
| TC-MF-003 | `meter:0.5` | `1.640420 feet` | 소수 입력 계산 정확도 |

## 5. 경계값 케이스 목록

| ID | 케이스 | 입력 | 기대 결과 | 우선순위 |
|---|---|---|---|---|
| TC-BV-001 | `value = 0` 영값 변환 | `meter:0` | 유효 입력, `0 feet` 반환 | P0 |
| TC-BV-002 | `value = 매우 큰 수` 오버플로 위험 | `meter:1e308` 또는 구현이 지원하는 최대 표현 근처 값 | 오버플로 발생 시 명시적 실패 또는 비정상 값 방지 | P1 |
| TC-BV-003 | `value < 0` 음수 입력 정책 | `meter:-1` | 변환 전 `NEGATIVE_VALUE` 실패, 결과 없음 | P0 |
| TC-BV-004 | 소수점 파싱 실패 | `meter:abc` | `INVALID_VALUE` 실패, 결과 없음 | P0 |
| TC-BV-005 | `:` 없는 입력 | `meter 2.5` | `INVALID_FORMAT` 실패, 결과 없음 | P0 |
| TC-BV-006 | 없는 단위 | `parsec:1.0` | `UNKNOWN_UNIT` 실패, 결과 없음 | P0 |

큰 수 테스트는 구현의 숫자 파서와 내부 자료형 정책에 맞춰 구체 값을 확정한다. C++ `double` 기반 구현이라면 `std::isfinite` 결과를 확인해 `inf`, `nan`이 정상 변환 결과로 노출되지 않도록 검증한다.

## 6. 예외 및 특이 케이스 목록

| 케이스 | 입력 예 | 기대 정책 |
|---|---|---|
| 빈 입력 | `` | `INVALID_FORMAT` |
| 빈 단위 | `:2.5` | `INVALID_UNIT_NAME` |
| 빈 값 | `meter:` | `INVALID_VALUE` |
| 콜론 2개 이상 | `meter:2.5:feet` | `INVALID_FORMAT` |
| 잘못된 소수 형식 | `meter:2.5.1` | `INVALID_VALUE` |
| 대문자 단위 | `Meter:2.5` | `INVALID_UNIT_NAME` 또는 단위명 규칙 실패 |
| 공백 포함 단위 | `meter :2.5` | 단위명 규칙 실패 또는 형식 실패 |
| 미등록 단위 | `parsec:1.0` | `UNKNOWN_UNIT` |
| 계산 결과 비정상 값 | 매우 큰 입력 | `inf`, `nan`을 성공 결과로 반환하지 않음 |

오류 케이스는 변환 결과를 생성하지 않아야 한다. Boundary 테스트는 실패 코드와 메시지 패턴을 함께 검증하고, Domain 테스트는 잘못된 값이 계산 규칙까지 전달되지 않는지 확인한다.

## 7. 커버리지 목표

| 영역 | 목표 | 측정 기준 |
|---|---:|---|
| Domain | 95% 이상 | 변환 공식, 단위 비율, 0 값, 큰 수 처리, 회귀 비율 테스트 |
| Boundary | 85% 이상 | 정상 파싱, 형식 오류, 값 오류, 음수, 미지원 단위 |
| Control | 80% 이상 | 정상 요청 흐름과 실패 요청 차단 흐름 |
| Integration | 핵심 시나리오 포함 | `meter:2.5`, `meter:0`, 대표 실패 입력 |

Domain 목표는 README의 기본 목표보다 높게 설정한다. `meter -> feet` 변환은 기준 비율 회귀 위험이 가장 크므로 정상값, 경계값, 비정상 계산값 방어를 포함해 95% 이상을 목표로 한다.

## 8. gcov, lcov, gcov UnitConverter.cpp 측정 전략

### 8.1 빌드 설정

- CMake Debug 빌드에서 커버리지 플래그를 활성화한다.
- GCC 또는 MinGW 환경 기준으로 `--coverage`, `-O0`, `-g` 옵션을 사용한다.
- Catch2 테스트 실행 파일과 `UnitConverter.cpp`가 동일 커버리지 산출 대상에 포함되는지 확인한다.

### 8.2 측정 절차

1. 커버리지 빌드 디렉터리를 새로 생성한다.
2. CMake configure 단계에서 커버리지 컴파일 옵션을 적용한다.
3. Catch2 테스트 바이너리를 실행한다.
4. `gcov UnitConverter.cpp`로 파일 단위 라인 실행 결과를 확인한다.
5. `lcov`로 전체 캡처 파일을 생성한다.
6. `genhtml`로 HTML 리포트를 생성해 Domain과 Boundary 목표 달성 여부를 확인한다.

### 8.3 예시 명령

```sh
cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug
cmake --build build-coverage
ctest --test-dir build-coverage --output-on-failure
gcov UnitConverter.cpp
lcov --capture --directory build-coverage --output-file coverage.info
genhtml coverage.info --output-directory coverage-report
```

Windows에서 MinGW 경로를 사용할 경우 `gcov`가 `.gcno`, `.gcda` 파일 위치를 찾지 못할 수 있다. 이 경우 `build-coverage` 하위의 오브젝트 파일 생성 경로에서 `gcov`를 실행하거나 `gcov -o <object-directory> UnitConverter.cpp` 형태로 오브젝트 디렉터리를 명시한다.

### 8.4 판정 기준

- `UnitConverter.cpp`의 Domain 관련 라인은 95% 이상 실행되어야 한다.
- Boundary 파싱 및 오류 분기 라인은 85% 이상 실행되어야 한다.
- `meter -> feet` 기준 비율 테스트가 실패하면 커버리지와 무관하게 빌드 실패로 판정한다.
- `NEGATIVE_VALUE`, `INVALID_VALUE`, `INVALID_FORMAT`, `UNKNOWN_UNIT` 경로 중 하나라도 미실행이면 테스트 보강 대상으로 분류한다.

## 9. 완료 기준

- Catch2 테스트에서 정상 경로 `meter:2.5`가 `8.202100 feet`와 허용 오차 이내로 통과한다.
- 모든 P0 경계값과 오류 정책 테스트가 통과한다.
- Domain 커버리지 95% 이상, Boundary 커버리지 85% 이상을 만족한다.
- `gcov UnitConverter.cpp`와 `lcov` 리포트가 동일한 핵심 미실행 라인을 보고하지 않는다.
- 변환 실패 입력은 어떠한 변환 결과도 생성하지 않는다.
