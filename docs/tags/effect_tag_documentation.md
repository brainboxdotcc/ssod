# `<effect>` Tag Documentation

The `<effect>` tag in *The Seven Spells of Destruction* is used to apply an environmental or status effect to the player, triggered from within paragraph content.

---

## Syntax

```text
<effect EffectName>
```

---

## Description

When encountered in paragraph content, the `<effect>` tag causes a named passive effect to be triggered for the current player. These effects are defined in the **passive_effects** database table and must be pre-configured.

---

## Behaviour

- The tag reads the effect name directly from the tag contents.
- The trailing `>` is removed.
- The passive effect is then triggered via the `trigger_effect()` function.
- An effect cannot re-trigger if it is currently running or cooling down.

---

## Passive Effect Mechanics

Each passive effect includes:

- A **start script**: This runs immediately on activation. It can:
  - Modify the player’s stats (e.g. reduce stamina, boost skill)
  - Send toast messages or notifications
  - Set internal gamestate flags

- An **end script**: This runs after the effect duration has elapsed. It is often used to:
  - Revert any changes made by the start script
  - Provide a narrative conclusion to the effect

- An **after-cooldown script**: Runs once cooldown ends. Can be used to:
  - Signal readiness to re-apply the effect
  - Trigger delayed consequences

Each script is written in server-side JavaScript (ES5-style), and the engine provides various bindings for interacting with the player’s state.

---

## Timing

- **Duration**: The number of in-game minutes the effect remains active.
- **Cooldown**: The time after the effect ends during which it cannot be re-applied.

---

## Use Cases

This tag is used to simulate:

- Temporary magical buffs (e.g. fire resistance, stealth boost)
- Negative afflictions (e.g. poisons, curses)
- Environmental reactions (e.g. choking gas, blinding mist)

---

## Example

```text
<effect VenomPoison>
```

This might apply a poison effect to the player, reducing stamina over time and preventing combat healing for several turns.

---

## Notes

- If the effect is already active or cooling down, it will **not** be triggered again.
- This system allows reusable, flexible status effect design using JS-defined logic.

---

## Related Tags

- None (this tag operates standalone but may work in conjunction with `<time>`, `<mod>`, or `<toast>` via effect scripts).