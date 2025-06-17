# Player Object Guide: Working with Achievements

This guide explains how to use achievement-related functions in JavaScript scripts for **The Seven Spells of Destruction**. These are global functions available to script authors for checking and unlocking achievements based on in-game events.

> ⚠️ Most achievement scripts are **not written in paragraphs**. They are defined separately via the Achievement Editor in the admin panel and evaluated automatically by the engine when specific events occur.

---

## 🧠 How Achievement Checks Work

Each achievement is tied to a specific **event type** such as:

- `MOD` – when stats are modified
- `VISIT` – when visiting a location
- `COMBAT` – after a combat
- `PICKUP`, `BOOK`, etc.

When the relevant event occurs, **all enabled achievements of that type** have their script evaluated.

> ✨ If the condition passes and the achievement is not already unlocked, it will be granted and announced automatically.

**You do not need to call `achievement_check()` yourself.** That is handled internally by the engine and game scripts — all you need to do is make sure your logic is correct and the achievement is correctly configured.

---

## ✅ Functions You Can Use

### `has_ach("SLUG")`
Returns `true` if the player already has the specified achievement.

```js
if (!has_ach("WANTED")) {
  // still locked
}
```

---

### `unlock_ach("SLUG")`
Grants the achievement to the player if they don't already have it. No effect if already unlocked.

```js
unlock_ach("WANTED");
```

You don’t need to worry about duplicates — the game engine handles this.

---

## 🖼️ Example from Editor

From the Achievement Editor in the admin panel:

```js
if (stat == "notoriety" && stat >= 100 && !has_ach("WANTED")) {
  unlock_ach("WANTED");
}
```

This script runs when any `MOD` event happens. It checks whether the stat being changed is `notoriety`, whether the new value is at least 100, and unlocks the `WANTED` achievement if the player hasn't already earned it.

You can also award experience or show toasts in the same script.

---

## 🔄 Can I Use These in Paragraph Scripts?

You **can**, and sometimes it’s convenient to do so for minor branching logic:

```js
if (player.gold >= 500 && !has_ach("RICH")) {
  unlock_ach("RICH");
}
```

But generally, it’s better to let the engine handle achievement logic automatically via the Achievement Editor. This ensures:

- Reusability of scripts
- Proper tracking of `MOD`, `COMBAT`, `VISIT` etc. events
- Consistent evaluation across players

> Avoid trying to trigger or simulate `achievement_check()` yourself. It’s part of the internal engine logic.

---

## 🔐 Related Functions

- `set_ach_key(name, value)` – Store a custom variable for the achievement script
- `get_ach_key(name)` – Retrieve an achievement-specific variable
- `delete_ach_key(name)` – Remove a saved value

These are for internal scripting use — for example, you might track player-specific progress across multiple events.

---

## Next Up
Read the [Toast Message Guide](toast-message-guide.md) to learn how to notify players with visual overlays.

