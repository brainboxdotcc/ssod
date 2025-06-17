# Getting Started with JavaScript Scripting

This quick reference is for content authors new to scripting in *Seven Spells of Destruction*. It walks you through the basics of adding interactivity using JavaScript inside your paragraphs.

> You do **not** need to be a programmer — just follow these patterns.

---

## ✨ What Can Scripts Do?

Scripts can:
- Show different text depending on player stats
- Deduct or reward gold, rations, XP, etc.
- Lock or unlock links dynamically
- Store custom flags or values
- Create light randomness or flavour

---

## 🧾 Basic Format

Scripts go inside a `<script>` block **on a single line**, like this:

```html
<script>print("Hello, adventurer!");</script>
```

Never place the closing `</script>` on its own line. Always inline:

✅ Good:
```html
<script>print("Hello");</script>
```

❌ Bad:
```html
<script>
print("Hello");
</script>
```

---

## 💰 Check Gold Example

```html
<script>
if (player.gold >= 20) {
  player.gold -= 20;
  print("You hand over the coins.");
} else {
  print("You don’t have enough gold.");
}</script>
```

---

## 🪙 Add a Link After a Condition

```html
<script>
if (player.skill >= 6) {
  print("<AUTOLINK=123, Climb the cliff>");
}</script>
```

> The link only appears if skill is 6 or higher.

---

## 🔒 Use Key/Value Storage

```html
<script>
if (!get_key("has_met_lord")) {
  print("A cloaked man approaches.\n");
  set_key("has_met_lord", "true");
}</script>
```

> This only prints once, even if the player comes back.

---

## 🧪 Use Tests

You can combine scripts with `<test>`:

```html
<test luck><script>
if (paragraph.auto_test) {
  print("You narrowly escape!");
} else {
  print("You trip and fall!");
}</script>
```

---

## 🪧 Helpful Hints

- Use `print("...\n")` for new lines.
- Always test in-game to make sure it looks right.
- Use `log("...")` for silent debugging.
- Use key names like `quest_flag_tavern` for clarity.

---

## 🧵 Further Learning

You can do more complex scripting once you’re confident. See the full [JavaScript Scripting Guide](../javascript-scripting-guide.md) for details.

Happy scripting!

