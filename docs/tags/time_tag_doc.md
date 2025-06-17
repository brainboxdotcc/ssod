# `<time>` Tag Documentation

The `<time>` tag represents the **passage of one in-game night**.

Although currently inactive in the engine, it is intended to be used to simulate time progression and should be written into your content as if it is active. The game logic may be enabled in future updates.

---

## Purpose

This tag models the transition of a single night in the game world. It helps reflect pacing and realism when the player rests or camps.

---

## Behaviour

When the `<time>` tag is triggered:
- **One ration is consumed** from the player’s inventory (if available).
- **An achievement check** of type `TIME` is performed for tracking progression.
- The tag has no effect if the paragraph has the `didntmove` flag set (i.e. the player reloaded or navigated to the same place).

---

## Typical Usage Scenarios

Use the `<time>` tag in locations where:
- The player chooses to sleep at an inn or campfire.
- A narrative segment involves waiting until morning.
- A forced time delay or rest period is appropriate in the story.

---

## Example

```text
You roll out your bedroll and prepare to rest. <time>
```

---

## Notes for Content Authors

- Even though time isn't actively tracked yet, including this tag makes your content **future-proof**.
- The engine is designed to support future time-based mechanics, including hunger, fatigue, and day-night cycles.
- Make sure to accompany `<time>` with narrative that clearly indicates rest or time passing.

---

## Related Tags

- `<eat>` – Allows direct eating of rations (not the same as automatic consumption).
- `<tempset>` – For flags that might be used in timed progression logic.
