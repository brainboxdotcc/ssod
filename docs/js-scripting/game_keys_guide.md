# Game Keys and Persistent Flags

The `set_key`, `get_key`, and `delete_key` functions provide a powerful way for content creators to track custom state across a player's entire game session. These values are persistent, meaning they are stored in the database and remembered between sessions.

> 🎯 Use keys to store custom flags, counters, or variables for each player.

---

## 🔑 What Are Keys?

Keys are simple string-based variables tied to a player. They allow you to:

- Track if the player made a specific choice
- Count the number of times an event happened
- Gate content until a flag is set or cleared

These are **not** paragraph-local. They persist globally per player.

---

## 🛠️ Functions

### `set_key(key, value)`

Sets a key to a string value.

```js
set_key("hideout_unlocked", "true");
```

> 🔐 Keys can only store strings. Convert numbers to strings with `String(value)` if needed.

---

### `get_key(key)`

Retrieves the stored string value for a key.

```js
if (get_key("hideout_unlocked") === "true") {
  print("You return to the hideout entrance.");
}
```

Returns `undefined` if the key has never been set.

---

### `delete_key(key)`

Removes a key permanently.

```js
delete_key("hideout_unlocked");
```

Useful for resetting state or allowing repeatable choices.

---

## 📘 Use Cases

### 📍 Custom Flags

```js
if (!get_key("intro_seen")) {
  print("Welcome, stranger! This must be your first time here.\n");
  set_key("intro_seen", "yes");
}
```

### 🔁 Optional Paths

```js
if (get_key("spoke_to_hermit") === "true") {
  print("The hermit gives you a knowing look.\n");
}
```

### ♻️ Repeatable Events

```js
let times = parseInt(get_key("fished_here") || "0");
times++;
set_key("fished_here", String(times));
```

> 🔎 You can track visit counts, number of wins, etc.

---

## ⚠️ Limitations & Best Practices

- Keys are **not typed** — everything is stored as a string.
- Don’t store large data; this is a flag system, not a savegame system.
- Don’t store sensitive data; keys can be inspected by admins.
- Clean up with `delete_key()` where appropriate.

---

## 🔗 Related Functions

If you want to store temporary, time-limited flags, consider using `<tempset>` instead. For global story flags shared across all players, use `set_global()` (via tag).

---

Return to the [JavaScript Scripting Guide](../javascript-scripting-guide.md) or continue with the [Achievement Key Functions](../achievement-keys.md).

