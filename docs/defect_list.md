# Defect List

현재 결함 목록은 RED 테스트 설계와 현재 `cpp/UnitConverter.cpp` 구현 검토 결과를 기준으로 작성한다. 코드 수정은 수행하지 않았으며, 각 결함은 이후 GREEN 단계에서 최소 변경으로 수정한다.

| ID | Severity | 변환 타입 | 재현 절차 | 기대값 | 실제값 | 근본 원인 | 수정 요약 |
|---|---|---|---|---|---|---|---|
| DEF-001 | Minor | meter→feet | `"meter:2.5"` 입력 | `8.202100 feet` | `8.2021 feet` | 출력 정밀도가 `std::fixed`, `std::setprecision(6)`로 고정되어 있지 않음 | 출력 전 `std::fixed << std::setprecision(6)` 적용 |
| DEF-002 | Minor | meter→yard | `"meter:1.0"` 입력 | `1.093610 yard` | `1.09361 yard` | 출력 포맷이 6자리 고정 소수 계약을 보장하지 않음 | plain 출력 포맷에 소수 6자리 고정 |
| DEF-003 | Major | feet→meter | `"feet:1.0"` 입력 | `0.304800 meter` | `0.3048 meter` | 역변환 계산은 가능하나 표시 정밀도 계약이 불일치 | 역변환 출력에도 동일한 정밀도 포맷 적용 |
| DEF-004 | Critical | 입력 검증 | `"meter:-1.0"` 입력 | `std::invalid_argument` 또는 `NEGATIVE_VALUE` 실패 | 음수 값이 그대로 변환됨 | `std::stod` 이후 `value < 0` 검증이 없음 | 변환 전 음수 입력 차단 분기 추가 |
| DEF-005 | Critical | 입력 형식 | `"meter:2.5:feet"` 입력 | `std::invalid_argument` 또는 `INVALID_FORMAT` 실패 | `std::stod`가 `2.5`까지만 파싱해 정상 처리 가능 | 첫 번째 `:` 존재 여부만 검사하고 추가 `:`를 검증하지 않음 | `pos != input.rfind(':')` 조건으로 콜론 개수 검증 |
| DEF-006 | Major | 값 파싱 | `"meter:abc"` 입력 | `std::invalid_argument` 또는 `INVALID_VALUE` 실패 | stderr 출력 후 `return 1` | 실패 정책이 예외 계약과 다르고 오류 코드가 구조화되지 않음 | `std::stod` 실패 시 `std::invalid_argument("INVALID_VALUE")` 발생 |
| DEF-007 | Major | 단위 검증 | `"parsec:1.0"` 입력 | `std::invalid_argument` 또는 `UNKNOWN_UNIT` 실패 | stderr 출력 후 `return 1` | 미지원 단위 실패 정책이 예외 계약과 다름 | unknown unit 분기에서 `std::invalid_argument("UNKNOWN_UNIT")` 발생 |
| DEF-008 | Major | 경계값 | `"meter:1e308"` 입력 | 오버플로 방지 실패 처리 | `inf` 또는 비정상 변환값 노출 가능 | 변환 결과에 대한 `std::isfinite` 검증이 없음 | 변환 결과 산출 후 유한값 검증 추가 |
| DEF-009 | Major | 동적 등록 | `"register:cubit=0.4572"` 후 `"cubit:1.0"` 입력 | `0.457200 meter` 변환 가능 | 등록 명령을 지원하지 않음 | 단위 레지스트리와 `registerUnit` 흐름이 없음 | 기존 if-else 외부에 등록 단위 저장소와 등록 처리 추가 |
| DEF-010 | Major | 설정 로드 | 유효한 JSON/YAML 설정 로드 후 `"meter:1.0"` 입력 | 설정 비율 기준 변환 | JSON/YAML 설정 로드 기능 없음 | 설정 파일 파싱 및 비율 주입 경로가 없음 | 설정 로드 함수와 기본 비율 유지 정책 추가 |
| DEF-011 | Major | 설정 실패 | 잘못된 JSON 또는 중복 단위 설정 로드 | `std::invalid_argument` 또는 설정 실패 코드 | 실패 케이스를 처리할 진입점 없음 | 설정 외부화 기능 자체가 구현되지 않음 | 설정 파싱 실패, 중복 단위, 잘못된 비율 검증 추가 |
| DEF-012 | Minor | 출력 보존 | `"meter:2.123456"` 입력 | 원 입력 값 `2.123456` 보존 | 기본 스트림 정밀도에 따라 값이 축약될 수 있음 | 원 입력 값 출력 정밀도 보존 정책 없음 | source value 출력 포맷 정책 명시 및 적용 |

## 우선 수정 순서

1. `DEF-004`, `DEF-005`: 잘못된 입력이 정상 변환되는 Critical 결함을 먼저 차단한다.
2. `DEF-006`, `DEF-007`, `DEF-008`: 실패 정책과 경계값 방어를 정리한다.
3. `DEF-001`, `DEF-002`, `DEF-003`, `DEF-012`: 출력 포맷 불일치를 정리한다.
4. `DEF-009`, `DEF-010`, `DEF-011`: 동적 등록과 설정 로드를 별도 기능 단위로 구현한다.
