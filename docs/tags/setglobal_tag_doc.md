# `<setglobal>` Tag

The `<setglobal>` tag is used to permanently set a global flag in the game state, visible and applicable to **all players**. It should be used **extremely sparingly**, as it affects every player in the game world — not just the one who triggers the tag.

---

## 💡 Summary

- **Sets a global flag** visible to all players.
- **Does not reapply** if the flag is already set.
- **Triggers** a `GLOBAL_STATE` achievement check.

---

## 🛑 Use With Caution

Global flags **persist across all players** and **should not be used for personal or narrative decisions** unless it is a world-changing event intended to be shared. Misuse may result in **unexpected behaviour** for other players.

Use only for:

- World events (e.g., an in-game bridge is destroyed for all)
- Unlocking shared content or mechanics
- Community-driven goals

Avoid using for:

- Personal quest progression
- Individual player triggers or unlocks
- Inventory or stat tracking

---

## ✍️ Syntax

```
<setglobal FLAGNAME>
```

Where `FLAGNAME` is any alphanumeric identifier for the flag.

---

## 🧠 Example

```
<setglobal crater_opened>
```

This sets the `crater_opened` flag globally. All future players can check for this flag using `<if crater_opened>` to alter world behaviour accordingly.

---

## ✅ Best Practices

- Combine with `<if>` tags for conditional behaviour.
- Always consider the **long-term effects** on the shared world.
- Only use when changes must **impact every player.**

---

## ⚙️ Technical Behaviour

This tag:
- Writes to the `game_global_flags` table using SQL.
- Inserts or updates the row to ensure the flag exists.
- Does **not** unset or modify existing flags.
- Converts the flag to lowercase and strips the trailing `>`.

