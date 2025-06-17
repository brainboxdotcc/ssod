# `<push>` Tag

The `<push>` tag stores the player's current location so they can return to it later using a `<pop>` tag.

It is most often used in situations where you send the player to a temporary paragraph (like a shop, a side-room, or a dialogue branch) and want to give them a return option.

---

## How It Works

- When `<push>` is encountered, the **current paragraph ID is stored on a stack**.
- The tag also **redirects the player** to the paragraph ID you specify.
- Later, a `<pop>` tag will make a **"Return" button** that links back to the stored paragraph.

---

## Syntax

```plaintext
<push 1234>
```

Where `1234` is the destination paragraph ID.

---

## Example

```plaintext
You climb the narrow staircase into a dusty attic. <push 5678>
```

Paragraph 5678 might be a room with some special interaction. When the player is ready to return, use:

```plaintext
<pop>
```

This brings them back to the original attic paragraph automatically.

---

## Notes

- Only one location can be stored at a time. Pushing again overwrites the stored value.
- This is separate from the normal navigation system and only works with `<pop>`.

