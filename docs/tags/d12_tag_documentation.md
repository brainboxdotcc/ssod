# `<d12>` Tag Documentation

## Overview

The `<d12>` tag in *The Seven Spells of Destruction* triggers a 12-sided dice roll (D12) when the paragraph is parsed. This tag is used to introduce random chance or probabilistic outcomes into the story or mechanics.

## Tag Syntax

```plaintext
<d12>
```

This tag does not accept any parameters or attributes.

## Behaviour

- When this tag is encountered during paragraph execution:
  - If the player has not already rolled a D12 in this context (`g_dice` is not set), it will trigger a `d12()` roll.
  - The result of the roll can then be referenced using subsequent logic within the same paragraph (e.g., `<if g_dice > 6>`).
- This tag does **not** output anything to the player directly; it silently rolls the die and stores the result for internal logic.

## Use Cases

- Chance-based challenges (e.g., randomly determine if a bridge collapses or a trap triggers).
- Luck-dependent loot drops or encounter outcomes.
- Branching paths based on a 1–12 roll.

## Example

```plaintext
<d12>
<if g_dice gt 8> You land safely. <br> <else> You sprain your ankle and lose 2 stamina. <br> <endif>
```

In this example, the `<d12>` tag determines whether the player succeeds or suffers a penalty based on their roll.

## Notes

- Only one D12 roll is stored in `g_dice` per paragraph unless explicitly reset or replaced by another roll (e.g., `<2d6>` or `<d6>`).
- Ensure conditional logic using `g_dice` follows the roll in the paragraph to prevent referencing an unset value.

