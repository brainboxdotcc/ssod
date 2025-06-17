# Understanding Paragraph State

The `paragraph` object is a read-only interface exposed to JavaScript that allows you to inspect key aspects of the current location during script execution. While you cannot modify it directly, it is invaluable for conditional logic and reactive narration.

> ⚠️ You cannot change paragraph fields in your script. Use `print()` to display changes, and use the `player` object (via setters) for modifications.

---

## 📚 What Is Paragraph State?

Each paragraph (location) that the player visits has some runtime state that can be read by your scripts. This state helps control branching logic based on recent tests or game events.

---

## 🔍 Read-Only Properties

| Property              | Type   | Description                                                                                                                                    |
| --------------------- | ------ | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| `paragraph.auto_test` | `bool` | Set automatically by the most recent `<test>` or `<sneaktest>` tag in this paragraph. `true` means the player passed.                          |
| `paragraph.safe`      | `bool` | Indicates this location is safe to resurrect to. Set to `false` if the paragraph contains combat, stat changes, or other irreversible effects. |
| `paragraph.words`     | `int`  | Number of words printed to the output stream so far in this paragraph. Useful for pacing.                                                      |

---

## 🧠 Common Use Cases

### ✅ Check a Test Result

```html
<test luck>
<script>
if (paragraph.auto_test) {
  print("Your gamble pays off.\n");
} else {
  print("Luck is not with you.\n");
}</script>
```

### 🚫 Avoid Danger When Safe

```html
<script>
if (!paragraph.safe) {
  print("You feel something is very wrong here.\n");
}
</script>
```

### 🕰️ Word Count Throttling

```html
<script>
if (paragraph.words < 30) {
  print("The silence drags on...\n");
}
</script>
```

---

## 🚫 What *Not* To Do

- ❌ `paragraph.safe = true;` → **Ignored. Paragraph state is immutable.**
- ❌ `paragraph.words += 10;` → **Won't change the printed output.**

To make meaningful state changes, always use the `player` setters like `player.gold += 10` or `set_key("some_flag", "true")`.

---

## 💡 Tips

- Use `paragraph.safe` to avoid soft-locking resurrection points.
- Pair `paragraph.auto_test` with `<AUTOLINK>` tags to show different choices on pass/fail.
- Use `paragraph.words` to avoid overly verbose locations.

---

Return to the [JavaScript Scripting Guide](../javascript-scripting-guide.md) or continue reading in [Common Scripting Patterns](../common-scripting-patterns.md).

