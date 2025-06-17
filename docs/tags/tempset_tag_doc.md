# `<tempset>` Tag — Temporary Flag Setter

The `<tempset>` tag is used to **temporarily set a state flag** on a player for a given duration. This is useful when you want an effect, access condition, or game state to persist only for a limited amount of real-world time.

---

### 🔧 Syntax

```
<tempset duration flagname>
```

- **duration**: Number of seconds the flag should remain active. This is real-time duration and is stored in the database. Long durations (e.g., days) are perfectly acceptable.
- **flagname**: The name of the flag to apply. The flag is automatically lowercased. Do **not** include the `>` — it is assumed to be the closing character.

---

### ✅ Example

```
<tempset 3600 cursed_mark>
```

This would apply the temporary flag `cursed_mark` to the player, lasting for one hour (3600 seconds).

---

### 🧠 Use Cases

- Time-limited curses or blessings.
- Limited access to special areas.
- Effects that wear off over time (without needing to use full passive effect infrastructure).
- Cooldowns on certain decisions or branches.

---

### 📌 Notes

- Flags set with `<tempset>` are **tracked per-player**.
- These flags **expire automatically** and are removed by the game engine after their time elapses.
- You can check for their presence using standard `<IF>` or `<TEST>` conditions.
- Flags are stored in the database table `timed_flags`.

---

### 🏆 Achievement Integration

When a flag is applied, the system will automatically check for any relevant achievements using the flag's name.

---

### ⚠️ Tips for Authors

- Avoid using `<tempset>` for irreversible or critical plot decisions—use `<set>` or `<setglobal>` for that.
- Ideal for flavour mechanics, delays, puzzles, or access gating.
