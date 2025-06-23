# `<set>` Tag

The `<set>` tag assigns a permanent key–value pair to the player’s state. This is useful for marking decisions, tracking quest progress, or setting dynamic variables that may influence later logic.

## Syntax

```text
<set flagname [value]>
```

- `flagname` — The unique name of the variable or flag to store.
- `value` — The value to associate with that flag. This can include **whitespace** and does **not** need to be quoted. The value extends to the closing `>` of the tag.
- If the `value` is omitted entirely, the flag will be stored with a default value of `"1"` (a boolean-style marker).

## Behaviour

- The data is stored in the database and persists permanently.
- Keys are automatically namespaced with the paragraph ID to avoid accidental collisions.
- You can overwrite existing values.
- The stored data can later be used with the `<if>` or `<mod>` tags.

## Flags and Values

Unlike `add_flag`, the `<set>` tag stores a **named value**.

Examples:

```text
<set has_sword yes>
<set dungeon_code 1234>
<set companion_name Marigold the Bold>
<set completed_tutorial>
```

These can later be retrieved in `<if>` comparisons like:

```text
<if flag has_sword eq "yes">...
<if flag dungeon_code eq "1234">...
<if flag completed_tutorial eq "1">...
```

## Notes

- Flag values are stored as strings.
- You can use whitespace in values; everything after the flag name until the closing `>` is treated as part of the value.
- If the value is numeric and used in a `<mod stat flag flagname>`, the stat will be adjusted accordingly.
- You cannot nest tags within `<set>` values.

---

See also:

- [`<if>`](if.md)[ tag](if.md) — for comparing stored flags
- [`<mod>`](mod_tag_doc.md)[ tag](mod_tag_doc.md) — for using flags as numeric inputs

