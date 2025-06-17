
# `<unset>` Tag

The `<unset>` tag is used to **remove a previously set player state flag**. This tag specifically unsets a temporary or permanent flag **only for the current player**, and **does not affect any global game state**.

---

## 📌 Syntax

```
<unset flag_name>
```

- `flag_name`: The name of the flag to unset. This corresponds to a previous `<set>` or `<tempset>` tag that had set `"gamestate_flag_name"`.

The tag must **not** include the `gamestate_` prefix — it is added automatically.

---

## ✅ Example

```text
<unset moon_rune>
```

This will delete the `gamestate_moon_rune` flag from the player's personal state record, if it exists.

---

## ⚠️ Important Notes

- This only affects **player-specific state**, not global flags.
- It does **not** unset anything created using `<setglobal>`.
- The unset operation is irreversible in the current session — if the player re-enters the location, the flag will remain unset unless it is set again.
- It matches any key that starts with the provided flag string (`LIKE 'gamestate_moon_rune%'`), allowing multiple versions or variants to be removed at once.

---

## 🧠 Use Case

Ideal for:
- Reversible puzzles where player actions can be undone.
- Situations where timed or conditional effects wear off or are dismissed.
