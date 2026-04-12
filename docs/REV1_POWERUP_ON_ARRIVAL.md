# Rev 1 Power-Up Plan (After Boards Arrive)

Use this document when the newly ordered Rev 1 control boards and BOM parts arrive.

## Current milestone (2026-04-12)
- Rev 1 PCB order placed
- BOM/parts order placed
- waiting for delivery

## Phase 0: incoming inspection (no power)
- confirm the board revision and quantity match the order
- inspect for shipping damage or board defects
- check critical footprints and connectors are present and oriented as expected
- verify the smart-switch section (`U1`..`U5`) looks physically correct

## Phase 1: assembly sanity checks
- populate in a controlled order: power/regulator section first, then MCU/core logic, then support circuits
- leave high-energy / optional sections unpowered until logic checks pass
- inspect solder joints under magnification, especially fine-pitch and polarized parts

## Phase 2: pre-power meter checks
- check continuity for shorts between `5V` and `GND`
- check continuity for shorts between `+22vdc` and `GND`
- verify there is no accidental continuity between `+22vdc` and logic rails
- verify connector pin-to-pin continuity where needed

## Phase 3: first power-up (`5V` only)
- use a current-limited bench supply
- power logic only at `5V`
- keep real coil supply disconnected
- watch current draw and temperature immediately
- verify expected regulator outputs and MCU boot behavior

## Phase 4: firmware and IO bring-up
- verify programming/debug access
- verify boot logs and expected startup behavior
- test safe GPIO and low-energy outputs first
- log observations and any anomalies

## Phase 5: staged output validation
- validate one smart-switch output channel at a time
- keep settings conservative while validating
- log CL/DELAY behavior in `docs/TPS1H200_CL_DELAY_BENCH_TABLE.md`

## Stop conditions
- unexpected current draw jump
- wrong regulator output
- unusual heating or smell
- any evidence of rail coupling that should not exist

## Linked docs
- `docs/FIRST_POWERUP_CHECKLIST.md`
- `docs/NEXT_STEP_BENCH_PLAN.md`
- `docs/TPS1H200_CL_DELAY_BENCH_TABLE.md`
- `docs/PCB_ROUTE_AND_ORDER_CHECKLIST.md`

## Success criteria for first arrival session
- board powers safely on `5V` only
- MCU and core logic are stable
- no critical thermal or short-circuit issues
- next session can proceed to controlled channel-level validation
