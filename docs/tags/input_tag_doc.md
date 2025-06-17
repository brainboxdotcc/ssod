# `<input>` Tag Documentation

The `<input>` tag initiates a **Discord modal prompt** that requests the player to type a specific answer in free text. The input is then validated against a pre-defined correct answer.

---

## Purpose

Used for puzzle-solving, password entry, name recognition, or any content that requires user-provided text input rather than predefined links.

---

## Syntax

```html
<input prompt="What is the name of the city of bones?" location="1309" value="Scythehold">
```

---

## Behaviour

- When this tag is rendered, the player sees a **question-style prompt** (`prompt`) on their screen and is asked to type an answer via a Discord modal.
- Once the player submits the modal input:
  - The engine performs a **case-insensitive** comparison of their response against the value given in `value`.
  - If the input **matches exactly** (after case normalization), the player is redirected to the paragraph ID given in `location`.
  - If the input does not match, they remain at the current paragraph with no further feedback unless scripted separately.
- One `<input>` tag can exist per paragraph, and it consumes a link slot.

---

## Attributes

| Attribute   | Required | Description                                                                 |
|-------------|----------|-----------------------------------------------------------------------------|
| `prompt`    | Yes      | The question shown to the user, rendered with a ❓ marker for clarity.       |
| `location`  | Yes      | The paragraph ID to redirect to if the correct answer is given.              |
| `value`     | Yes      | The correct answer, matched case-insensitively but otherwise exactly.        |

---

## Example

```html
<input prompt="Name the demon who escaped the Keep." location="810" value="Garneth">
```

If the player enters “garneth”, “GARNETH”, or “Garneth”, they proceed to paragraph 810. “Garnet”, “Gareth”, or “Garneth!” are all considered incorrect.

---

## Notes

- This tag adds one to the paragraph’s `.links` count.
- Input correctness is evaluated **on submission**, not client-side.
- No scripting or spell effects can alter the expected input behaviour.
- Consider pairing this with a `<set>` or `<mod>` tag in the destination paragraph to track success.
