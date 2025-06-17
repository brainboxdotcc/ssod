# `<-- ... -->` Comment Tag

The `<-- ... -->` tag in Seven Spells of Destruction allows content authors to embed non-displayed commentary directly in paragraph content. These comments are ignored entirely by the game engine and do not affect rendering, logic, navigation, or player experience.

---

## Syntax

```text
<!-- This is a comment -->
```

Comments must begin with `<!--` and continue until the closing `-->`. Everything between these markers is skipped during processing.

---

## Purpose

This tag is useful for:

- **Editorial notes** during content development
- **Flagging reminders** or TODOs in markup
- **Disabling sections** temporarily without deleting them
- **Describing** content logic for other writers or developers

---

## Behaviour

- The parser skips all text from `<!--` to `-->`.
- Comment contents are **not** parsed for tags or logic.
- **No output** is generated.
- **No database records**, links, navigation points, or player feedback are created.
- Comments cannot nest — the first `-->` encountered terminates the comment block.

---

## Example

```text
<!-- This paragraph ends here logically -->
<if flag "has_key">
You unlock the door.
<endif>
```

The comment in the example is ignored by the parser, and the logic block executes normally.

---

## Notes

- Avoid using comment tags to hide required control structures (e.g., an opening `<if>` without its matching `<endif>`).
- Use with care in production content — excessive comments can obscure structure for technical writers.

