# `<dice>` Tag Documentation

## Overview

The `<dice>` tag is used in Seven Spells of Destruction to initiate a standard six-sided die roll (1D6) for the current player, storing the result in a temporary state variable for use later in the paragraph logic.

This tag does not produce visible output or alter game text, but it does affect conditional logic through references to the generated dice value (e.g., for `<if>` tests against the result).

## Syntax

```
<dice>
```

- There are no attributes or inner content for this tag.
- It must be placed on its own line or in a position where it can execute prior to any `<if>` tests or navigation logic that reference the dice value.

## Behaviour

- When the tag is encountered:
  - A random number from 1 to 6 is generated (as if rolling a standard die).
  - The result is stored in `current_player.g_dice`, accessible for conditional logic later in the same paragraph.

- If a dice result has already been set earlier in the paragraph, this tag will not override it.

## Example

```text
You come to a rickety bridge across a dark chasm. You must roll a die to determine your fate.

<dice>
<if dice >= 4> You cross safely. <br> <else> The bridge breaks and you fall! <br> <endif>
```

## Notes

- This tag is functionally similar to `<d12>` and `<2d6>`, but specifically rolls 1D6.
- Dice results are ephemeral and only valid during the current paragraph evaluation.
- This tag has no visible output and is silent to the player unless paired with logic that displays outcomes based on the result.
