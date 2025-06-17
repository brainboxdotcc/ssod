# Common Scripting Patterns

This quick reference expands on the basics with reusable scripting patterns you can copy and adapt. Perfect for content authors who want their locations to feel alive and responsive.

> All scripts go inside a `<script>...</script>` tag. See the [Getting Started guide](scripting-quick-reference.md) if you're new.

---

## 🎁 Conditional Rewards

Give gold or XP based on a stat:

```html
<script>
if (player.sneak >= 7) {
  player.gold += 10;
  print("You quietly loot 10 gold.\n");
} else {
  print("You’re too loud — nothing gained.\n");
}</script>
```

---

## 💬 One-Time Dialogue or Encounter

Make something appear **only once**:

```html
<script>
if (!get_key("met_crone")) {
  print("An old crone beckons you closer.\n");
  set_key("met_crone", "true");
}</script>
```

---

## 🔁 Repeatable Daily Event

Use `get_last_use()` to throttle an event to once per in-game day:

```html
<script>
if (get_days() > get_last_use()) {
  print("You find a healing herb!\n");
  player.stamina += 1;
  set_last_use(get_days());
}</script>
```

---

## 🧪 Test with Branching Outcome

Combine with a `<test>` tag for chance-based results:

```html
<test sneak><script>
if (paragraph.auto_test) {
  print("You slip by unseen.\n");
} else {
  print("A guard spots you!\n");
}</script>
```

---

## 🔐 Lock or Unlock Links

Only show a link if a condition is met:

```html
<script>
if (player.scrolls >= 3) {
  print("<AUTOLINK=432, Cast the binding spell>");
}</script>
```

---

## 🛏️ Charge for Services

Require payment to proceed:

```html
<script>
if (player.gold >= 5) {
  player.gold -= 5;
  print("<AUTOLINK=310, Rent a room>");
} else {
  print("You can’t afford the room.\n");
}</script>
```

---

## 🧱 Block a Paragraph

Prevent continuing until a requirement is met:

```html
<script>
if (player.skill < 5) {
  print("You aren’t skilled enough to proceed.\n");
  tag("exit");
}</script>
```

---

## 🧵 Tip: Build Reusables

You can build a "soft library" of patterns by copy-pasting known good script blocks from other locations. The system doesn’t currently support user-defined functions — keep it simple!

For full JavaScript reference, see [JavaScript Scripting Guide](../javascript-scripting-guide.md).

---

Happy scripting! More examples in other guides.

