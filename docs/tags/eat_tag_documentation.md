# `<eat>` Tag Documentation

## Overview

The `<eat>` tag is used within a paragraph to **force the player to consume a ration**. If the player has no rations available, the effect may instead penalise them by reducing stamina. This tag is primarily used to enforce survival mechanics in longer adventures or hostile environments.

## Syntax

```html
<eat>
```

## Behaviour

- When this tag is processed:
  - If the player **has rations**, one is consumed.
  - If **no rations are available**, stamina is reduced by a predefined penalty (handled internally).
  - The paragraph is marked **unsafe** (i.e. `p.safe = false`).
  - An achievement check for the `FORCED_EAT` achievement is triggered.
  - This tag does **nothing** if the paragraph was flagged with `didntmove`, typically used to detect inaction.

## Conditions

- Should only be used in paragraphs where **player progression requires time to pass**, such as travel or rest events.
- Does **not provide interactive choice**—it is a passive trigger.

## Example

```text
Night falls as you rest by the smouldering ruins.
<eat>
You dream of the war before the darkness came.
```

In this example, the player will automatically consume a ration (or lose stamina) as part of the narrative event.

## Notes

- Designed to simulate food consumption on travel or rest.
- This tag has **no parameters**.
- There is **no visual output** shown to the player unless a toast or notification is triggered via achievement or status change.

