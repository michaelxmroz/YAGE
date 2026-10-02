# Fix PPU sprite timing — conversation transcript

Exported 2026-10-01 from local chat 01a025b9-c5a6-71d3-9d6c-0b68d7e82cf1.

All 10 turns returned by the chat reader, in chronological order. Includes user messages and assistant commentary/final answers; excludes internal reasoning, tool calls/output, and attachments. This is a readable context export, not an app session import. Historical claims and file line references may be outdated; see PPU-HANDOFF.md for the current checkout comparison.

---

## Turn 1 — 2026-08-21T19:08:47.000Z

### User

YAGE is a Gameboy (DMG) emulator. It is feature complete and I'm trying to get it to be as accurate as possible. One thing I have been struggling with are the subtle, cycle-accurate timing changes that happen when the PPU is drawing sprites, where PPU Mode 3/ Mode 0 length changes based on how many sprites are on screen. I have not been able to get that right. The PPU is a very complex state machine. I would like you to help me figure out what code changes would be necessary to get the timings right. The ground truth for this is the mooneye-test-suite intr\_2\_mode0\_timing\_sprites test. You can find the test, and the test source code in C:\Projects\GameBoy\assets\mooneye\acceptance\ppu. You can find the ppu emulator source code in C:\Projects\GameBoy\src\YAGECore\Source. Look for ppu.cpp, pixelfetcher.cpp and pixelfifo.cpp. For reference you can look up the gameboy pandocs, at [https://gbdev.io/pandocs/Rendering.html](https://gbdev.io/pandocs/Rendering.html). They give a good high-level overview but don't go into a lot of detail. What was also helpful for me was this reddit post [https://www.reddit.com/r/EmuDev/comments/s6cpis/gameboy\_trying\_to\_understand\_sprite\_fifo\_behavior/](https://www.reddit.com/r/EmuDev/comments/s6cpis/gameboy_trying_to_understand_sprite_fifo_behavior/), especially if you expand one of the comment trees further, talking about hidden registers. Please do not look at the source code of other emulators. In case you cannot figure it out by static analysis alone, please ask and I can point you towards how to compile the project yourself and run the test. That might need some additional work, as the test outputs the result to the screen, so we would need to modify the emulator code so it logs the result to a file or something, but lets start with static analysis first.&#x20;


### Assistant (progress)

I’ll trace the test’s exact timing expectations against YAGE’s dot-by-dot PPU state transitions, then compare that with Pan Docs and the hidden-register behavior described in the Reddit thread. I’ll keep this to static analysis for now and won’t change the emulator until we have a concrete timing model.

### Assistant (progress)

The test is more revealing than its name suggests: it probes not only “6 dots per sprite,” but phase-dependent penalties, overlap/coalescing, off-screen sprites, and independence from OAM order. YAGE’s current sprite fetcher is a separate four-state machine and begins fetches from `lineX`; I’m now checking whether that representation can express those phase rules at all.

### Assistant (progress)

The static model now matches the ROM exactly: a small penalty formula reproduces all 105 `testcase` rows with zero mismatches, including the reversed-OAM-order cases. That gives us a precise target and also pinpoints why the current prefetch loop and four-stage sprite fetch cannot pass the suite.

### Assistant (answer)

Static analysis found a precise timing model: it reproduces all 105 Mooneye cases with zero mismatches. YAGE’s current sprite pipeline cannot express that model, mainly because every sprite costs approximately 10 dots and left-edge sprites are prefetched incorrectly.

## Exact timing expected by the ROM

For this test, `SCX = 0` and the window is disabled. Using the raw OAM X coordinate:

```text
Process selected sprites in ascending (OAM X, OAM index) order.

Ignore sprites with X >= 168.

tile = floor((X - 8) / 8)

wait =
    if this tile has not been encountered before:
        max(0, 5 - (X & 7))
    otherwise:
        0

sprite penalty = wait + 6 dots
```

The test’s first argument is:

```text
floor(total sprite penalty / 4)
```

This explains every group in the [Mooneye source](/C:/Projects/GameBoy/assets/mooneye/acceptance/ppu/intr_2_mode0_timing_sprites.s:55):

- First sprite in a tile costs, by `X & 7`: `11, 10, 9, 8, 7, 6, 6, 6` dots.
- Additional sprites whose left edges occupy that same background tile cost exactly 6 dots each.
- Ten sprites at X=0: `11 + 9×6 = 65` dots, and `floor(65/4) = 16`.
- Ten sprites eight pixels apart at phase 0: `10×11 = 110` dots, and `floor(110/4) = 27`.
- X=168 and X=169 cause no penalty.
- Sorting by X explains why reversing OAM order does not change timing.

This is the detailed OBJ penalty algorithm now documented by [Pan Docs](https://gbdev.io/pandocs/Rendering.html). One wording nuance: treating every coincident X=0 sprite as costing 11 dots would contradict this ROM. The test requires the first to cost 11 and subsequent sprites in that tile to cost 6.

## Why YAGE currently misses it

1. The sprite prefetch loop is incorrect.

   [PPU.cpp](/C:/Projects/GameBoy/src/YAGECore/Source/PPU.cpp:476) scans offsets from 8 down to 1. Consequently:

   - OAM X=0 is never matched.
   - X=8/9 sprites are considered before farther-left X=1–7 sprites.
   - Sprites are fetched before the background FIFO contains data.

   The prefetch loop should be removed. At screen X=0, all sprites with `OAM X <= 8` are already due and should be selected leftmost-first after the initial background data is available.

2. A sprite fetch currently costs 10 dots.

   `DrawPixels()` consumes 2 dots unconditionally. The sprite `GetTile` state consumes another 2 internally at [PixelFetcher.cpp](/C:/Projects/GameBoy/src/YAGECore/Source/PixelFetcher.cpp:106), and the sprite passes through all four states, including `Sleep`:

   ```text
   GetTile: 4
   Low:     2
   High:    2
   Sleep:   2
   Total:  10 dots
   ```

   An OBJ fetch must be three 2-dot stages—6 dots total. For sprite mode:

   - Remove the extra `processedCycles += 2`.
   - Push/merge the sprite pixels at the end of `ReadTileHigh`.
   - Do not enter `Sleep`; that state remains relevant to background FIFO insertion.

3. There is no phase-dependent waiting penalty.

   YAGE has no record of which background tile has already caused a wait. Add per-scanline state such as:

   ```cpp
   bool    m_objTileValid;
   int16_t m_lastObjTile;
   uint8_t m_objWaitDots;
   bool    m_objFetchActive;
   uint8_t m_activeObjIndex;
   ```

   When beginning a sprite fetch, calculate its tile and add the `0–5` dot wait only if that tile differs from the last considered tile. Then perform the flat 6-dot fetch.

4. The pipeline cannot currently represent odd penalties reliably.

   Most drawing work advances in 2-dot chunks, but valid penalties include 7, 9, and 11 dots. `m_objWaitDots` therefore needs to be decremented one dot at a time, or the entire drawing pipeline should be stepped per dot.

5. Sprite selection breaks OAM tie priority.

   [GetCurrentSprite()](/C:/Projects/GameBoy/src/YAGECore/Source/PPU.cpp:579) overwrites `spriteIndex` while iterating, so equal-X sprites select the highest OAM index. Selection should be by lowest X, then lowest OAM index.

## Recommended implementation shape

For the targeted Mooneye fix:

- Preserve the existing background pipeline, since it currently produces the intended 172-dot baseline.
- Replace `m_spritePrefetchLine` with a “due sprite” selector.
- Do not start OBJ fetching until the background FIFO is non-empty.
- Add the per-background-tile wait calculation.
- Make the OBJ fetch itself exactly 6 dots.
- Pause pixel output throughout both the wait and OBJ fetch.
- Reset the tile/wait/active-object state in `TransitionToOAMScan()` or `TransitionToDraw()`.

For the more accurate long-term design, use one shared fetch sequencer rather than independent background and sprite `PixelFetcher` objects. The Reddit discussion confirms that the hardware has one pair of temporary tile-data registers capable of holding BG or OBJ data, while the pixel FIFOs stop during OBJ arbitration—not separate BG and OBJ temporary-register sets. [Reddit discussion](https://www.reddit.com/r/EmuDev/comments/s6cpis/gameboy_trying_to_understand_sprite_fifo_behavior/)

There is also a broader timing issue worth fixing during that refactor: [PPU.cpp](/C:/Projects/GameBoy/src/YAGECore/Source/PPU.cpp:354) advances `StateTransition` once per `Render()` call, while [VirtualMachine.cpp](/C:/Projects/GameBoy/src/YAGECore/Source/VirtualMachine.cpp:72) calls it with either 4 dots or 1 dot depending on debugger microstepping. Externally visible STAT timing can therefore depend on how the emulator is stepped. Transition latches should advance by elapsed dots, not API invocations.

`PixelFIFO.cpp` does not need a structural change for this particular test. The necessary changes are concentrated in `PPU.h`, `PPU.cpp`, and `PixelFetcher.cpp`.

I made no code changes and inspected no other emulator source. The next useful step would be implementing the targeted state changes first, then running the ROM to determine whether YAGE’s existing STAT transition latency introduces an additional one-M-cycle offset.

---

## Turn 2 — 2026-08-22T09:48:40.000Z

### User

Please create a detailed task list for the changes required to the PPU code, but without emitting code or pseudocode. Just in a way where I as the original creator can get enough context to start working with it again and implement the changes. I have not worked on that code for a year or so.


### Assistant (answer)

Below is an implementation-oriented task list, ordered to minimize regressions and help you rebuild the mental model of the code.

## 1. Re-establish the current PPU timing model

- [ ] Review `PPU::Render()` and confirm that its input is now T-cycles, despite the declaration still naming the argument `mCycles`.
- [ ] Trace how `processedCycles` and `m_cycleDebt` allow drawing operations to exceed the current call’s cycle budget.
- [ ] Confirm the current no-sprite Mode 3 duration. Static analysis indicates the pipeline produces 172 dots; preserve this while changing sprite behavior.
- [ ] Review the purpose of `StateTransition::Cycle0`, `Cycle1`, and `Cycle2`, especially their relationship to:
  - The externally visible STAT mode.
  - VRAM and OAM blocking.
  - STAT interrupt generation.
- [ ] Note that state transitions currently advance once per `Render()` invocation, not once per elapsed dot. This means debugger microstepping may change transition timing. Treat that as a related issue to address after the sprite pipeline works.
- [ ] Record passing results for the non-sprite PPU timing tests before changing anything:
  - `intr_2_mode0_timing`
  - `intr_2_mode3_timing`
  - `intr_2_oam_ok_timing`
  - Existing rendering tests such as Acid2.

Relevant files:

- [PPU.cpp](/C:/Projects/GameBoy/src/YAGECore/Source/PPU.cpp)
- [PPU.h](/C:/Projects/GameBoy/src/YAGECore/Source/PPU.h)
- [VirtualMachine.cpp](/C:/Projects/GameBoy/src/YAGECore/Source/VirtualMachine.cpp:65)

## 2. Establish the exact Mooneye target

- [ ] Treat the sprite X values in the ROM as raw OAM X coordinates. The visible left edge is OAM X minus 8.
- [ ] Preserve OAM scan behavior: the first ten vertically matching OAM entries are selected in OAM order.
- [ ] During drawing, process those selected sprites from left to right.
- [ ] Break equal-X ties using the lower OAM index.
- [ ] Do not fetch sprites with OAM X of 168 or greater.
- [ ] Understand the phase-dependent cost for the first sprite associated with a background tile:

| OAM X modulo 8 | First sprite penalty |
|---:|---:|
| 0 | 11 dots |
| 1 | 10 dots |
| 2 | 9 dots |
| 3 | 8 dots |
| 4 | 7 dots |
| 5 | 6 dots |
| 6 | 6 dots |
| 7 | 6 dots |

- [ ] Charge exactly 6 dots for subsequent sprites associated with the same background tile.
- [ ] Reset the “background tile already considered” tracking at the beginning of each scanline.
- [ ] Use the ROM’s 105 cases as a data-driven acceptance table. The timing model above matches all of them.

The test source is [intr_2_mode0_timing_sprites.s](/C:/Projects/GameBoy/assets/mooneye/acceptance/ppu/intr_2_mode0_timing_sprites.s:55).

## 3. Replace the existing left-edge prefetch logic

- [ ] Remove the role of `m_spritePrefetchLine`.
- [ ] Remove the special loop in `DrawPixels()` that searches offsets from 8 down to 1.
- [ ] Replace it with normal “sprite is due” detection based on the sprite’s visible left edge and the current output position.
- [ ] At screen X=0, consider every selected sprite whose visible left edge is at or before X=0. This includes raw OAM X values from 0 through 8.
- [ ] Ensure those left-edge sprites are selected by ascending OAM X, not by descending offset.
- [ ] Do not begin fetching these sprites before initial background data is available. The hardware’s OBJ fetch arbitration starts only once the background pipeline has produced FIFO data.
- [ ] Ensure a due sprite remains latched until its fetch completes. Do not rediscover it independently on every fetcher step.

This replaces the behavior around [PPU.cpp](/C:/Projects/GameBoy/src/YAGECore/Source/PPU.cpp:476).

## 4. Rewrite sprite selection semantics

- [ ] Replace or substantially revise `GetCurrentSprite()`.
- [ ] Separate these concepts:
  - A sprite selected during OAM scan.
  - A sprite that has become due for fetching.
  - A sprite whose fetch is currently active.
  - A sprite whose fetch has completed.
- [ ] Continue using `m_lineSpriteMask` for completed sprites if desired, but do not use it as the only representation of an in-progress fetch.
- [ ] Add explicit storage for the active sprite index.
- [ ] Add an explicit active-fetch flag or fetch-owner state.
- [ ] Ensure the current sprite stays stable while its wait and fetch stages execute.
- [ ] Fix equal-X priority. The current loop overwrites the candidate and therefore tends to select the highest matching OAM index.
- [ ] Confirm that reversing OAM order for sprites at different X coordinates has no timing effect.
- [ ] Confirm that reversing OAM order for equal-X sprites preserves lower-OAM-index pixel priority.

Relevant function: [PPU::GetCurrentSprite()](/C:/Projects/GameBoy/src/YAGECore/Source/PPU.cpp:579).

## 5. Reduce the OBJ fetch itself to six dots

- [ ] Review the four current `PixelFetcher` states: `GetTile`, `ReadTileLow`, `ReadTileHigh`, and `Sleep`.
- [ ] Remove the sprite-specific extra two-dot charge currently applied during `GetTile`.
- [ ] Keep each actual sprite fetch stage at two dots.
- [ ] Finish a sprite fetch immediately after the high tile-data byte has been obtained.
- [ ] Merge the sprite pixels into the sprite FIFO at that point.
- [ ] Do not send sprite fetching through the background fetcher’s `Sleep`/FIFO-push waiting state.
- [ ] Leave background behavior unchanged initially, because its existing fetch/push sequence contributes to the correct 172-dot baseline.
- [ ] Verify that one OBJ fetch with no background wait now stalls output for exactly six dots.

Relevant logic:

- [PixelFetcher::Step()](/C:/Projects/GameBoy/src/YAGECore/Source/PixelFetcher.cpp:76)
- [PixelFetcher::PushPixels()](/C:/Projects/GameBoy/src/YAGECore/Source/PixelFetcher.cpp:146)

## 6. Add background-fetch waiting penalties

- [ ] Add scanline state identifying the last background/window tile considered by an OBJ.
- [ ] Add a remaining OBJ wait-dot counter.
- [ ] When a sprite becomes active, identify the background tile containing its leftmost pixel.
- [ ] If a previous sprite already considered that tile, skip the background wait and apply only the six-dot OBJ fetch.
- [ ] Otherwise, calculate the zero-to-five-dot background wait from the sprite’s position within the tile.
- [ ] Mark the tile as considered when the sprite begins arbitration, not when its fetch finishes.
- [ ] Pause pixel output during both the background wait and the six-dot OBJ fetch.
- [ ] Prevent the background FIFO from shifting during this pause.
- [ ] Preserve the background fetcher’s state while OBJ work owns the fetch pipeline.
- [ ] Resume background fetching and pixel output after the OBJ data has been merged.

For the Mooneye test, the tile identity is determined with `SCX = 0` and no window. Avoid tying the long-term representation exclusively to that simplified case.

## 7. Preserve a latched SCX phase

- [ ] Do not use `m_fineScrollX` as both the original SCX phase and a mutable discard counter.
- [ ] Add or retain a separate value representing the low three SCX bits latched at the beginning of the scanline.
- [ ] Continue decrementing the existing discard counter as pixels are discarded.
- [ ] Use the unmodified latched value for general OBJ/background tile phase calculations.
- [ ] Verify the target ROM first with SCX zero.
- [ ] Later add coverage for every SCX phase, because the OBJ waiting penalty shifts with background tile alignment.

## 8. Support odd-dot stalls correctly

- [ ] Make the OBJ waiting counter capable of consuming one dot at a time. Valid total penalties include 7, 9, and 11 dots.
- [ ] Audit every assumption that Mode 3 work advances only in two-dot units.
- [ ] Verify that an odd Mode 3 length produces the correct Mode 0 entry time.
- [ ] Verify that HBlank still ends on dot 456 without rounding the scanline length.
- [ ] Review the HBlank correction that decrements `m_totalCycles` when it is odd. It is likely compensating for the current two-dot stepping model and should not remain as an unexplained timing adjustment.
- [ ] Audit `m_cycleDebt` with both positive and negative debt after odd-dot work.
- [ ] Ensure an overrun from one `Render()` call is carried forward without losing or duplicating a dot.
- [ ] Confirm that internal Mode 3 completion and externally visible STAT Mode 0 remain correctly aligned.

## 9. Introduce explicit fetch ownership

For the targeted fix, the two existing `PixelFetcher` instances can be retained temporarily. For long-term accuracy:

- [ ] Add an explicit owner for the tile-fetch hardware: background, window, OBJ, or idle.
- [ ] Centralize the temporary tile address and low/high tile bytes.
- [ ] Model one shared set of temporary fetch registers rather than independent background and sprite temporary storage.
- [ ] Preserve separate logical background and sprite FIFOs; they remain useful representations of the output pipeline.
- [ ] Make OBJ arbitration suspend the normal background/window fetch sequence.
- [ ] Make window activation use the same shared fetch ownership instead of independently resetting a background-only object.
- [ ] Keep this refactor separate from the initial Mooneye patch if it threatens the already-correct 172-dot baseline.

## 10. Review sprite FIFO merging and priority

- [ ] Preserve the rule that an incoming sprite pixel only replaces an existing transparent sprite FIFO pixel.
- [ ] Confirm that transparent pixels still occupy priority positions even though they do not produce visible color.
- [ ] Confirm that lower-X sprites have priority on DMG.
- [ ] Confirm that equal-X sprites use lower OAM index priority.
- [ ] Revisit the existing “odd position” placeholder insertion after fetch timing is correct. Ensure it represents actual FIFO alignment rather than compensating for two-pixel stepping.
- [ ] Verify partially off-left sprites at every X from 0 through 7.
- [ ] Verify the final partially visible sprite at X=167.
- [ ] Verify that X=168 is completely ignored by Mode 3.

`PixelFIFO.cpp` itself probably does not require a timing-oriented structural change.

## 11. Reset and serialization work

- [ ] Reset all new OBJ arbitration state at the beginning of every drawing scanline.
- [ ] Reset active sprite state when the LCD is disabled.
- [ ] Reset it when the PPU is re-enabled.
- [ ] Include every new state field in save states.
- [ ] Remember that `PPUData` is currently serialized as one raw block. Adding, removing, or reordering fields changes the save-state layout automatically.
- [ ] Decide whether old save states may be invalidated or whether the PPU serialization format needs versioning.
- [ ] Remove `m_spritePrefetchLine` from serialization once it is no longer used.
- [ ] Initialize all new fields explicitly in `PPUData`’s constructor.

## 12. Decouple state transitions from `Render()` calls

After sprite timing passes:

- [ ] Change `StateTransition` advancement so it is based on elapsed dots.
- [ ] Ensure passing four dots to `Render()` has the same observable result as four one-dot calls.
- [ ] Verify that debugger microstepping no longer affects:
  - STAT mode changes.
  - STAT interrupts.
  - LY updates.
  - VRAM/OAM accessibility.
- [ ] Recheck the special first-frame handling.
- [ ] Recheck the delayed LY=0 behavior on line 153.
- [ ] Re-run all non-sprite interrupt timing tests after this change.

## 13. Add diagnostics before running the ROM

- [ ] Add temporary logging for each scanline containing:
  - Line number.
  - Mode 3 start and end dot.
  - Selected sprite count.
  - Sprite OAM index and raw X.
  - Sprite activation dot.
  - Background wait length.
  - OBJ fetch start and end dots.
  - Background tile identity.
- [ ] Log the exact test case ID when the Mooneye ROM reports failure.
- [ ] Add a debug assertion that every completed scanline totals 456 dots.
- [ ] Add a debug assertion that no more than ten sprites are fetched on a scanline.
- [ ] Add a debug assertion that every completed OBJ fetch lasted six dots, excluding its background wait.

## 14. Validation order

- [ ] Confirm no-sprite Mode 3 remains 172 dots.
- [ ] Test one sprite at every OAM X from 0 through 17.
- [ ] Test one sprite at X=160 through 169.
- [ ] Test multiple coincident sprites and confirm only the first pays the background wait.
- [ ] Test sprites eight pixels apart and confirm each pays a new background wait.
- [ ] Test two separated groups and confirm two independent waits.
- [ ] Test reversed OAM order.
- [ ] Run all 105 cases in `intr_2_mode0_timing_sprites`.
- [ ] Re-run the non-sprite Mode 0 and Mode 3 timing ROMs.
- [ ] Re-run visual sprite-priority tests.
- [ ] Only then expand the tile-phase calculation to non-zero SCX and window tiles.

The safest implementation order is: fix sprite selection, make the OBJ fetch six dots, add the per-tile wait, support odd dots, pass the target ROM, and then unify the fetcher and repair call-dependent state transitions.

---

## Turn 3 — 2026-09-04T07:21:18.000Z

### User

This list is a lot. Please re formulate it by implementation order and mark checkpoints at which point some assumptions become re-checkable through test roms. For example 1.) rewrite the pixel FIFO[...] 2)... X) At this point all other tests should be timing-correct again even if sprites are not.

I just would like to not have to do this giant rewrite only to realize that your diagnosis was incorrect or suddenly some other timing issues arise&#x20;


### Assistant (answer)

You should not begin with a FIFO rewrite. The safest route is to prove the timing diagnosis with a disposable timing-only experiment, then make three small production changes with test checkpoints between them.

Commit each checkpoint separately so it can be reverted cleanly.

## 0. Capture the baseline

### Work

1. Build the current emulator without changing PPU behavior.
2. Record results for:
   - `intr_2_mode0_timing`
   - `intr_2_mode3_timing`
   - `intr_2_oam_ok_timing`
   - `intr_2_0_timing`
   - `intr_2_mode0_timing_sprites`
   - Any sprite-priority or visual tests you already trust.
3. Add temporary logging for:
   - Mode 3 start and end dot.
   - LY.
   - Number and X coordinates of selected sprites.
   - The failing Mooneye test-case ID, if practical.

### Checkpoint A: baseline validity

The non-sprite timing tests should pass exactly as they did before.

If `intr_2_mode0_timing` or `intr_2_mode3_timing` already fails, stop. Sprite work would otherwise obscure an existing STAT or base Mode 3 problem.

---

## 1. Prove the penalty formula with a disposable timing experiment

This step deliberately avoids changing the fetcher or FIFO.

### Work

1. On a temporary branch or behind a temporary debug switch, bypass normal sprite fetching during this particular timing experiment.
2. Calculate the aggregate sprite penalty from the sprites selected by OAM scan:
   - Process them left-to-right, with OAM index breaking equal-X ties.
   - Ignore X positions 168 and above.
   - Charge the first sprite in each background tile between 6 and 11 dots depending on its X phase.
   - Charge additional sprites in that tile 6 dots.
3. Hold Mode 3 open for that aggregate duration.
4. Do not worry about rendering correct sprite pixels in this experimental version.

### Checkpoint B: diagnosis confirmation

Run `intr_2_mode0_timing_sprites`.

Expected result: all 105 cases pass.

Then run the non-sprite timing tests. They should remain unchanged because the experiment only applies when sprites are present.

If the sprite ROM does not pass here, stop before touching the fetcher. Investigate:

- STAT Mode 0 visibility latency.
- `m_cycleDebt`.
- Odd-dot handling.
- Whether the ROM is reporting the expected case.
- Whether the selected sprites and raw OAM X coordinates match the source.

This checkpoint separates “the penalty model is wrong” from “the implementation of the pipeline is wrong.”

After confirming the result, revert or disable this timing-only experiment.

---

## 2. Fix sprite discovery without changing fetch timing

### Work

1. Remove the `m_spritePrefetchLine` approach.
2. Replace the descending offset search at the start of the scanline.
3. Define a sprite as due when its visible left edge has reached the current output position.
4. At output X=0, allow raw OAM X positions 0 through 8 to become due.
5. Select due sprites by:
   - Lowest raw OAM X first.
   - Lowest OAM index for equal X.
6. Ignore raw OAM X positions 168 and above.
7. Latch the active sprite until its fetch completes.
8. Do not start sprite work until the background FIFO contains initial data.
9. Continue using the existing fetcher timing for now, even though it is too long.

### Checkpoint C: selection correctness

Run visual sprite tests and inspect logging.

Confirm:

- X=0 sprites are now discovered.
- X=0 through X=8 are processed left-to-right.
- X=167 is fetched.
- X=168 is ignored.
- Reversing OAM order for differently positioned sprites does not change timing order.
- Equal-X sprites use lower OAM index first.

The target timing ROM is not expected to pass yet. With the existing roughly 10-dot sprite fetch, coincident-sprite cases should be too slow.

All non-sprite timing tests must still pass. If they regress, the new selection logic is interfering with background drawing when no sprite is active.

---

## 3. Make the OBJ data fetch exactly six dots

### Work

1. Remove the extra sprite-specific cycle charge from the `GetTile` state.
2. Keep the three actual OBJ stages at two dots each:
   - Establish/latch the tile address and sprite metadata.
   - Read the low tile byte.
   - Read the high tile byte.
3. Merge the sprite pixels after the high-byte stage.
4. Do not send OBJ fetching through the background fetcher’s `Sleep` state.
5. Preserve the existing background fetch sequence unchanged.
6. Continue pausing pixel output for the duration of an active OBJ fetch.

### Checkpoint D: flat fetch cost

Use logging to confirm every OBJ fetch now lasts exactly six dots.

Run the non-sprite timing tests. They must remain unchanged.

The complete sprite timing ROM is still not expected to pass, because the first sprite associated with a background tile is missing its zero-to-five-dot wait. In particular, the earliest X-phase cases should now be too fast.

If a sprite fetch is not exactly six dots, stop here. Do not add the background wait on top of an uncertain base fetch duration.

---

## 4. Add the per-background-tile wait

### Work

1. Add state for:
   - The currently active sprite.
   - Remaining background-wait dots.
   - The last background tile considered by a sprite.
   - Whether that last-tile value is valid.
2. Reset this state at the beginning of every drawing scanline.
3. When a sprite becomes active, identify the background tile containing its leftmost pixel.
4. If this is the first sprite associated with that tile, apply the phase-dependent wait:
   - X phases 0 through 7 produce total first-sprite penalties of 11, 10, 9, 8, 7, 6, 6, and 6 dots.
   - Since the OBJ fetch itself is six dots, the corresponding waits are 5, 4, 3, 2, 1, 0, 0, and 0 dots.
5. If the tile has already been considered, apply no background wait.
6. Mark the tile as considered when arbitration begins.
7. Pause pixel output during both the wait and OBJ fetch.
8. Consume the wait with single-dot precision.
9. Preserve the background fetcher state while OBJ work is active.

For now, implement the tile calculation only for the ROM’s configuration: SCX zero and window disabled. Keep the state representation extensible, but do not introduce window complexity yet.

### Checkpoint E: target ROM

Run `intr_2_mode0_timing_sprites`.

Expected result: all 105 cases pass.

Also verify these representative totals:

- Ten sprites at X=0: 65 added dots.
- Ten sprites at X=1: 64 added dots.
- Ten sprites at X=2: 63 added dots.
- Ten sprites at X=7: 60 added dots.
- Ten sprites eight pixels apart starting at X=0: 110 added dots.
- Ten sprites at X=168: no added dots.

At this checkpoint, all existing non-window, SCX-zero timing tests should be correct again, including sprite Mode 0 timing.

If the calculated totals are correct but the ROM still fails, the remaining problem is probably Mode 3-to-Mode 0 visibility or odd-dot accounting—not the sprite penalty model.

---

## 5. Stabilize odd-dot and HBlank behavior

Only begin this after Checkpoint E, or if Checkpoint E shows correct internal totals but incorrect observed STAT timing.

### Work

1. Verify that 7-, 9-, and 11-dot penalties survive `m_cycleDebt` without rounding.
2. Verify that ending Mode 3 on an odd dot produces the correct Mode 0 start.
3. Audit the HBlank code that adjusts `m_totalCycles` when it is odd.
4. Ensure HBlank ends at scanline dot 456 without silently adding or deleting a dot.
5. Ensure the total duration of Mode 2, Mode 3, and Mode 0 is always 456 dots.
6. Check that STAT exposes Mode 0 at the intended time, rather than only when the next four-dot CPU step happens.

### Checkpoint F: complete scanline timing

Run:

- `intr_2_mode0_timing_sprites`
- `intr_2_mode0_timing`
- `intr_2_mode3_timing`
- `intr_2_oam_ok_timing`
- `intr_2_0_timing`

All should pass together.

At this point, the sprite timing fix is complete for SCX zero and no window. Stop here and commit before attempting architectural cleanup.

---

## 6. Extend the proven model to SCX and the window

### Work

1. Preserve an immutable copy of the fine SCX value latched at scanline start.
2. Stop relying on the mutable fine-scroll discard counter for OBJ phase calculations.
3. Include SCX in:
   - The tile containing the sprite’s leftmost pixel.
   - The sprite’s phase within that tile.
4. Add a tile identity that distinguishes background tiles from window tiles.
5. When the window begins, ensure subsequent OBJ penalties use window tile alignment.
6. Test all eight SCX phases before combining them with window behavior.

### Checkpoint G: generalized penalties

Run available SCX and window timing ROMs after each of these two substeps:

1. SCX support with the window disabled.
2. Window support after SCX passes.

Do not combine both changes into one commit.

---

## 7. Optional shared-fetcher refactor

This is architectural cleanup, not required to validate the diagnosis or pass the target ROM.

### Work

1. Replace the independent temporary BG and OBJ fetch state with explicit ownership of one fetch engine.
2. Let that engine represent background, window, OBJ, or idle work.
3. Centralize the temporary tile address and low/high bytes.
4. Keep the background and sprite pixel FIFOs logically separate.
5. Preserve the already-proven wait and six-dot OBJ timings exactly.

### Checkpoint H: behavior-preserving refactor

Every test from Checkpoint F and G must produce identical results before and after the refactor.

If anything changes, revert the refactor rather than adjusting timing constants. The pre-refactor implementation is already your known-good behavioral reference.

---

## 8. Separately fix call-dependent `StateTransition`

Do not mix this with sprite fetching.

### Work

1. Make transition progress depend on elapsed dots rather than the number of `Render()` calls.
2. Compare four one-dot calls with one four-dot call.
3. Verify identical:
   - STAT modes.
   - STAT interrupts.
   - LY changes.
   - VRAM/OAM accessibility.
4. Recheck first-frame and line-153 special handling.

### Final checkpoint

Run the full PPU timing suite in both normal execution and debugger microstepping.

Both modes should now produce identical results.

The key safety gates are Checkpoint B and Checkpoint E. Checkpoint B proves the diagnosis before any pipeline rewrite; Checkpoint E proves the production implementation before SCX, window, shared-fetcher, or transition cleanup is attempted.

---

## Turn 4 — 2026-09-04T11:54:11.000Z

### User

So, I tried only modifying toe dot counts for sprites, for testing purposes. You can check PPU.cpp. When I log out the dot counts, I get the expected number, but the very first sprite test with a single sprite still fails. I assume this is because of the complex delay machinery that the PPU employs with the StateTransition enum and multiple dots before the internal state gets reflected to an outside observer ( i.e. the CPU). IS this StateTransition necessary to correctly emulate some of the hardware timing/bus quirks of the gameboy?&#x20;


### Assistant (progress)

I’ll inspect your current timing-only changes and trace the exact sequence from internal Mode 3 completion through the STAT value observed by the CPU. The key distinction is whether `StateTransition` represents a real hardware delay or an emulator scheduling workaround whose duration accidentally depends on `Render()` call boundaries.

### Assistant (progress)

Your temporary path exposes a real problem: after it advances Mode 3 by the correct 183 dots, `m_cycleDebt` makes the CPU “catch up,” but `StateTransition` advances by whole `Render()` calls. In normal execution those calls are four dots, so the STAT mode change is quantized and delayed independently of the actual Mode 3 endpoint. I’m checking which pieces of that enum were introduced for genuine one-dot bus quirks versus which are accidental scheduling delays.

### Assistant (answer)

Yes—some staged effects are necessary, but the current `StateTransition` abstraction is doing too many unrelated jobs and advances at the wrong granularity. I would not remove it wholesale yet. However, your first test failure is very likely caused by the temporary bulk-dot implementation interacting with `m_cycleDebt`, before any question of authentic hardware latency.

## What happens in your temporary implementation

For the first sprite, your log correctly reports Mode 3 as 183 dots: the 172-dot baseline plus an 11-dot sprite penalty.

But [PPU.cpp](/C:/Projects/GameBoy/src/YAGECore/Source/PPU.cpp:311) adds all 183 dots to `processedCycles` at once and sets `m_lineX` to 160. It does not transition to HBlank during that pass.

The resulting sequence is:

1. `processedCycles` jumps far beyond the four-dot budget of the current `Render()` call.
2. The loop exits with the internal state still `Drawing`.
3. [m_cycleDebt](/C:/Projects/GameBoy/src/YAGECore/Source/PPU.cpp:416) becomes strongly negative.
4. Many subsequent `Render()` calls perform no PPU work while that debt is repaid.
5. Once the debt reaches zero, the `m_lineX == 160` branch finally calls `TransitionToHBlank()`.
6. STAT is not changed then. It is changed later, when `StateTransition` reaches `Cycle2`.

Your Mode 3 log is generated from `m_totalCycles`, so it can report the correct 183-dot interval even though the CPU-visible STAT transition occurs at a different time.

That makes the log necessary evidence, but not sufficient evidence that the CPU observes the right boundary.

## Is `StateTransition` necessary?

The underlying timing distinctions are necessary. This particular shared three-stage enum is not inherently necessary.

It currently handles at least four different hardware behaviors:

| Behavior | Real timing distinction? | Should share one generic transition? |
|---|---:|---:|
| STAT mode bits changing | Yes | Probably not |
| OAM/VRAM read and write blocking | Yes | No—read and write edges may differ |
| LY and LYC behavior | Yes | No—line transitions have their own quirks |
| STAT interrupt edge detection | Yes | No—derive from the visible STAT sources |

The Game Boy does have bus-boundary quirks. CPU access to OAM and VRAM depends on the current PPU mode, and the exact access boundaries can differ slightly from simplified mode descriptions. [Pan Docs: Accessing VRAM and OAM](https://gbdev.io/pandocs/Accessing_VRAM_and_OAM.html)

STAT interrupts also require accurate edge handling: the enabled Mode 0/1/2 and LYC sources are ORed together, and an interrupt occurs only on a low-to-high transition. [Pan Docs: Interrupt Sources](https://gbdev.io/pandocs/Interrupt_Sources.html)

So the effects represented by the enum must not simply disappear. But the current implementation has a significant flaw:

- [StateTransition advances once per `Render()` call](/C:/Projects/GameBoy/src/YAGECore/Source/PPU.cpp:390).
- Normal execution calls `Render()` with four dots.
- Debugger microstepping can call it with one dot.
- A transition stage therefore lasts either one or four dots depending on how the emulator is driven.

That cannot represent fixed hardware timing.

## The best next experiment

Do not modify or remove `StateTransition` yet. Instead, change only how your temporary sprite delay is consumed:

1. Store the calculated Mode 3 duration or remaining sprite penalty.
2. Consume it progressively as emulated time advances.
3. Keep the PPU in Drawing throughout that interval.
4. Enter HBlank at the exact dot when the countdown expires.
5. Avoid jumping the PPU 183 dots ahead and then using negative debt to let the CPU catch up.

This should make the experimental sprite path enter HBlank through the same scheduling mechanism as the existing no-sprite path.

### Checkpoint

Run the first sprite case again.

- If it passes, the penalty formula is confirmed and the immediate failure was caused by bulk advancement/debt interaction.
- If it still fails, log three separate timestamps:
  - When internal `m_state` becomes HBlank.
  - When STAT bits 0–1 are written to Mode 0.
  - When the CPU executes the first relevant STAT read.

That will tell you whether the remaining discrepancy is internal Mode 3 timing, STAT publication latency, or CPU/PPU execution order.

## Two additional problems in the temporary calculation

These do not explain the single-sprite dot log, but they will break later cases:

- `tile` is unsigned. For OAM X below 8, the calculated tile is negative and becomes 255. Shifting a 32-bit value by that amount is undefined.
- The calculation uses all vertically selected sprites for the six-dot charge. X positions 168 and above must contribute no Mode 3 penalty.
- Tiles are marked in OAM order. For sprites at different positions in the same tile, the leftmost sprite must determine the initial wait, regardless of OAM order.
- A 32-bit bitmask is an awkward representation for the off-left tile. A signed tile identity or “last considered tile” value is safer.

My recommendation is therefore: preserve `StateTransition` for now, replace the bulk 183-dot jump with a progressive countdown, and retest. Only after the target sprite test passes should you split `StateTransition` into independent dot-timed events for STAT, memory access, LY, and interrupt evaluation.

---

## Turn 5 — 2026-09-04T12:18:48.000Z

### User

Does the "off left" tile count as its own tile for penalty calculations?


### Assistant (answer)

Yes. With `SCX = 0`, the off-left region counts as a distinct background-fetch tile:

- OAM X 0–7 → tile covering screen coordinates −8 through −1
- OAM X 8–15 → tile covering screen coordinates 0 through 7

Therefore:

- The first sprite in the off-left tile pays the phase-dependent wait plus the 6-dot fetch.
- Additional sprites in that same off-left tile pay only 6 dots.
- A sprite at X=8 belongs to the next tile and can incur a new wait.

For example, ten sprites at OAM X=0 cost 11 + 9×6 = 65 dots, matching the test’s expected value of 16 M-cycles after truncation.

This tile identity is based on the background fetch alignment, not permanently fixed screen-coordinate blocks. Non-zero `SCX` changes which background tile contains a sprite’s leftmost pixel. OAM X=0 additionally has special documented behavior concerning its initial wait.

---

## Turn 6 — 2026-09-04T12:23:07.000Z

### User

So I essentially need two loops, right? One for going through all sprites and calculating the smallest offset penalty for each tile, and a second loop to add all penalties together.


### Assistant (answer)

For a timing-only aggregate calculation, two loops are a valid approach—but you need the leftmost sprite per tile, not the smallest penalty.

1. First loop: group eligible sprites by background tile and record:

   - Number of sprites in that tile.
   - The leftmost raw OAM X in that tile.
   - Ignore X ≥ 168.

2. Second loop: for each occupied tile, add:

   - Six dots per sprite.
   - One phase-dependent wait based on that tile’s leftmost sprite.

Equivalently, sort sprites by `(X, OAM index)` and do everything in one pass: the first sprite encountered in each tile pays the wait, and every sprite pays six dots.

Important details:

- OAM X 0–7 belongs to the off-left tile. Represent that tile with a signed value or remap it safely; your current unsigned tile calculation underflows.
- Do not use original OAM order to choose the tile’s waiting penalty. The reversed-order Mooneye cases require left-to-right processing.
- It is not the smallest offset penalty in the tile. The leftmost sprite determines the wait—even if a sprite farther right would produce a smaller wait.

For the eventual real pipeline, you do not need to precompute the total: process sprites left-to-right and remember the last tile that incurred a wait. The two-loop calculation is ideal for your current timing-only diagnostic.

---

## Turn 7 — 2026-09-04T12:24:26.000Z

### User

Sorry, I meant the largest penality. A sprite further left will always produce a larger penalty


### Assistant (answer)

Correct. Within one background-fetch tile, the leftmost sprite produces the largest waiting penalty.

So for each occupied tile:

- Add 6 dots for every eligible sprite.
- Add the largest background-wait penalty among those sprites.

Equivalently:

- Add the largest complete first-sprite penalty in that tile.
- Add 6 dots for every remaining sprite.

For `SCX = 0`, the complete first-sprite penalties by X phase are `11, 10, 9, 8, 7, 6, 6, 6` dots. This approach is independent of OAM ordering and therefore handles the reversed-order test cases correctly.

---

## Turn 8 — 2026-09-04T12:35:40.000Z

### User

I adjusted the algorithm, but the current implementation of tilePenalties causes a stack corruption. I believe it is due to the memset I'm using. Could you check what I'm doing wrong?


### Assistant (progress)

I’ll inspect the new `tilePenalties` declaration, initialization, and indexing together—the corruption could come from `memset`, but an out-of-range tile index (especially the off-left tile) is equally likely.

### Assistant (answer)

The corruption is an out-of-bounds write, not the `memset`.

`tilePenalties` has 20 entries, indexed `0–19`, but sprites at OAM X `160–167` produce tile index 20:

- Off-left tile, X `0–7` → index 0
- Visible tiles, X `8–159` → indices 1–19
- Final partially visible tile, X `160–167` → index 20

So you need 21 penalty buckets.

There is also a calculation bug here:

```cpp
floor((posX - 8) / 8)
```

IntegerInteger division occurs before `floor`, so negative values are already truncated toward zero. For this SCX-zero experiment, the shifted tile index is simply `posX / 8`.

I would therefore:

- Make `tilePenalties` 21 bytes.
- Derive its index from `posX / 8`.
- Iterate over all 21 entries.
- Initialize it using `sizeof(tilePenalties)` so the byte count cannot drift from the declaration.
- Prefer the project’s `memset_y` wrapper, although ordinary `memset` is not the cause of this corruption.

The offending write is [PPU.cpp](/C:/Projects/GameBoy/src/YAGECore/Source/PPU.cpp:309). The debugger’s stack-corruption report likely appears when the function returns because index 20 has overwritten the byte immediately following the local array.

---

## Turn 9 — 2026-09-04T14:40:55.000Z

### User

Instead of this breakdown, could you give me a holistic picture of how the system is supposed to work?

## 2. Fix sprite discovery without changing fetch timing

### Work

1. Remove the `m_spritePrefetchLine` approach.
2. Replace the descending offset search at the start of the scanline.
3. Define a sprite as due when its visible left edge has reached the current output position.
4. At output X=0, allow raw OAM X positions 0 through 8 to become due.
5. Select due sprites by:
   - Lowest raw OAM X first.
   - Lowest OAM index for equal X.
6. Ignore raw OAM X positions 168 and above.
7. Latch the active sprite until its fetch completes.
8. Do not start sprite work until the background FIFO contains initial data.
9. Continue using the existing fetcher timing for now, even though it is too long.


### Assistant (answer)

The clean mental model is that sprite handling has two separate phases: Mode 2 decides which sprites belong to the scanline; Mode 3 decides when each selected sprite must interrupt pixel production.

## Mode 2: selecting the line’s sprites

During the 80-dot OAM scan, the PPU walks OAM from entry 0 to entry 39. It checks only vertical overlap and retains the first ten matching entries.

At this stage:

- OAM order determines which ten sprites survive the line limit.
- Horizontal position does not exclude a sprite.
- A sprite at X=0 or X≥168 can still consume one of the ten slots.
- No sorting by X happens yet.

The resulting `m_lineSprites` array should therefore remain in OAM order.

## Mode 3: background production begins

Mode 3 starts by priming the background fetch pipeline. The fetcher obtains background tile data and places pixels into the background FIFO.

No visible pixel is emitted immediately because the pipeline needs its initial data and alignment. This is important for sprites on the left edge: they are already due, but their fetch cannot begin until the background pipeline has reached the point where OBJ fetching can take control.

This is what `m_spritePrefetchLine` was trying to approximate. The hardware is better understood as having overdue sprites waiting at X=0, rather than the PPU explicitly walking through eight imaginary negative screen positions.

## Determining when a sprite is due

A sprite’s visible left edge is its raw OAM X minus 8.

As the PPU’s output position advances, a selected sprite becomes due when that left edge reaches the current output position.

Therefore, at output X=0:

- OAM X=0 has a left edge at −8 and is already overdue.
- OAM X=1 through 7 are partially off the left side and are already overdue.
- OAM X=8 begins exactly at screen X=0 and is due.
- OAM X=9 begins at screen X=1 and is not due yet.

Sprites that are already overdue are processed leftmost-first. Equal-X sprites are processed in OAM order.

This ordering is independent of the order in which they were collected during Mode 2. OAM order selected the ten candidates; horizontal position controls their processing order during Mode 3.

## What happens when a sprite becomes due

Assuming sprites are enabled, the PPU latches one due sprite as the active sprite. It must remain active until its fetch finishes.

While that sprite is active:

- The visible output position does not advance.
- Background pixels do not shift out.
- The fetch hardware is occupied by the sprite operation.
- Other due sprites remain pending.

The active sprite may first incur a wait based on the current background-fetch alignment. That is the variable zero-to-five-dot portion of the penalty. It then performs the fixed six-dot OBJ fetch.

Once its tile data has been fetched, its eight pixels are aligned with the current output position and merged into the sprite FIFO.

The active sprite is then marked complete.

## Multiple sprites in the same area

Because the output position is frozen during OBJ fetching, another sprite can still be due immediately after the first fetch completes.

For example, if ten sprites all have the same X:

1. The first sprite becomes active.
2. It pays the background-alignment wait and its six-dot fetch.
3. The output position remains unchanged.
4. The next equal-X sprite is still due.
5. It pays only its six-dot fetch.
6. This repeats until all ten have been fetched.

Only the first sprite associated with that background tile pays the background wait. Every sprite pays the six-dot fetch cost.

After all currently due sprites have been handled, background fetching and pixel output resume.

## FIFO behavior

Conceptually, the background and sprite pixel queues advance together when a pixel is sent to the LCD.

When OBJ fetching occurs, both are frozen. The newly fetched sprite row is merged into the existing sprite FIFO:

- Transparent incoming pixels do not replace existing sprite pixels.
- Opaque pixels fill positions that are still transparent.
- Processing sprites leftmost-first, then by OAM index, naturally establishes DMG sprite priority.

The background and sprite pixels are combined only when the next pixel is finally emitted.

## Repeating across the scanline

The complete Mode 3 loop is conceptually:

**Advance background fetching → check for due sprites → handle all due sprites → emit pixels → advance X → check again**

The output X advances only when a pixel is emitted. Fetcher work, FIFO stalls, SCX discards, window startup, and OBJ fetching consume dots without advancing it.

Once X reaches 160, visible production is finished and the PPU enters Mode 0. Selected sprites with raw X≥168 never become due, so they produce no Mode 3 penalty even though they occupied Mode 2 selection slots.

## State the emulator actually needs

This model only requires a few distinct pieces of sprite state:

- The ten sprites selected during Mode 2.
- A completion flag for each selected sprite.
- The current LCD output X.
- The currently latched active sprite, if any.
- The active sprite-fetch phase.
- Any remaining background-alignment wait.
- The background tile most recently considered for an OBJ wait.

There is no need for `m_spritePrefetchLine`. Off-left sprites fall out naturally: when background data becomes available at output X=0, they are already due and are handled before the first visible pixel is emitted.

---

## Turn 10 — 2026-09-04T14:48:06.000Z

### User

Instead of just hardcoding the formula for the dot penalty for a new sprite in a tile, and the penalty for each individual sprite, can I emulate what is actually causing those delays? Or is that unnecessary?


### Assistant (progress)

Yes—you can model the underlying contention instead of adding penalties directly. I’m checking the current Pan Docs fetch-step description so I can separate well-established behavior from details that are still best treated as an observed timing rule.

### Assistant (answer)

Yes, you can emulate the cause—but it is not necessary to pass this test, and rebuilding the physical mechanism immediately would be a considerably riskier change.

The penalty formula is essentially a compressed description of contention between the background fetcher, the OBJ fetch, and the pixel shifter.

## What physically causes the variable penalty

During Mode 3, background fetching and LCD pixel output overlap:

- The background fetcher progresses through tile index, low byte, high byte, sleep, and FIFO-push activity.
- Meanwhile, the FIFO emits roughly one pixel per dot.
- OBJ fetching uses the same underlying fetch machinery.
- When a sprite becomes due, pixel output stops.
- The OBJ fetch cannot always begin in the same fetcher phase.

The first sprite whose left edge occupies a particular background tile may encounter the background fetcher partway through fetching that tile. It must wait until the fetcher reaches an appropriate handoff point. Depending on the sprite’s position within that tile, this takes zero through five dots.

After that wait, fetching the sprite data takes six dots.

This produces:

| Background wait | OBJ fetch | Total |
|---:|---:|---:|
| 5 | 6 | 11 |
| 4 | 6 | 10 |
| 3 | 6 | 9 |
| 2 | 6 | 8 |
| 1 | 6 | 7 |
| 0 | 6 | 6 |

Subsequent sprites associated with the same background tile do not encounter that initial background-fetch alignment again, so they only incur the six-dot OBJ fetch.

The current detailed [Pan Docs FIFO description](https://gbdev.io/pandocs/pixel_fifo.html) describes this in terms of advancing the shared fetcher to an appropriate state, then performing the OBJ fetch sequence. The higher-level [rendering description](https://gbdev.io/pandocs/Rendering.html) deliberately reduces it to the observable 6–11-dot rule.

## Three possible accuracy levels

### 1. Aggregate line penalty

Calculate the total sprite penalty and extend Mode 3 by that amount.

This is enough for `intr_2_mode0_timing_sprites`, but it is not cycle-accurate within the line. It would mishandle effects such as a palette write occurring between two sprites.

This is appropriate only as your current diagnostic experiment.

### 2. Event-based penalty model

When a sprite becomes due:

- Stop pixel output at that X position.
- Apply the calculated background wait.
- Perform the six-dot OBJ fetch.
- Merge the result into the sprite FIFO.
- Resume output.

This uses the documented formula, but places the stall at the correct point in the scanline.

This should reproduce almost all externally observable effects relevant to normal DMG rendering:

- Correct Mode 3 length.
- Correct Mode 0 start.
- Correct timing of pixels following each sprite.
- Correct interaction with mid-scanline palette changes.
- Correct handling of multiple sprites in the same tile.
- Correct SCX and window alignment, once incorporated into the tile calculation.

This is the level I recommend for YAGE initially.

### 3. Fully mechanistic shared fetcher

Model one fetch engine that can be owned by:

- Background fetching.
- Window fetching.
- OBJ fetching.
- FIFO push attempts.

You would track the fetcher at dot-level resolution and derive the wait naturally from its current state when an OBJ becomes due.

This can eventually handle subtleties such as:

- Changing LCDC sprite enable during a fetch.
- OBJ-fetch cancellation.
- Exact VRAM access timing.
- Window activation during nearby fetcher phases.
- CGB-specific behavior.
- Mid-fetch register changes.

But it requires substantially more than replacing the penalty calculation. The existing two-dot `PixelFetcher::Step()` abstraction does not represent all the one-dot and three-dot advancements described by the detailed documentation. You would need to revisit the fetcher scheduler, FIFO push attempts, pixel emission, and CPU/PPU dot ordering together.

Some low-level details are also not fully confirmed—the Pan Docs section explicitly notes uncertainty around the timing of certain X=0/SCX and cancellation behavior. A mechanistic implementation based on incomplete assumptions could be less accurate than the measured formula.

## Recommended compromise

Use the event-based formula as the production implementation:

- Do not add the total penalty at the beginning or end of Mode 3.
- Apply each stall when the corresponding sprite becomes due.
- Let the six-dot portion drive the real sprite tile fetch.
- Let the zero-to-five-dot portion be an explicit wait state.
- Keep the background fetcher frozen or suspended throughout the OBJ operation.

Then retain your aggregate penalty calculator as a debug oracle. At the end of each static scanline, compare the actual accumulated OBJ stalls against the formula’s expected total. That gives you a strong safety net if you later replace the explicit waits with a fully shared fetcher.

In other words: emulate the observable contention now, without pretending to know every internal gate-level detail. Once all timing ROMs pass, you can replace individual formula-derived waits with mechanistic fetcher transitions one piece at a time, checking that their accumulated timing remains identical.

