
# `<script>` Tag

The `<script>` tag allows you to embed custom JavaScript logic inside a location. This is a powerful and flexible tag for advanced behaviour — but should only be used by experienced content authors.

**Important:** Players should refer to the separate [Embedded JavaScript Reference Guide] for the full list of available functions, structures, and safe execution rules. This guide will be made available separately.

---

## Behaviour

When a `<script>` tag is encountered in a paragraph, the engine collects all text until it sees a matching `</script>` closing tag. This embedded text is then passed to the internal JavaScript engine for execution.

The script has full access to:
- The **current paragraph**, including text, flags, ID, secure ID, and state markers.
- The **current player**, including all attributes, inventory state, stat values, and flags.

These are provided as JS objects: `player` and `paragraph`.

Scripts **must not** attempt to modify the existing paragraph text directly. Instead, use the special `print(...)` function to append new content to the paragraph output.

## ✅ Example

```html
<script>
  if (player.rations < 1) {
    print("\nYou feel hungry and weak.\n");
  } else {
    print("\nYou still have food.\n");
  }
</script>
```

This script will insert one of the two lines into the paragraph output based on whether the player has rations.


---

## Best Practices

- Only use `<script>` when tag-based tools (like `<if>`, `<mod>`, `<set>`) cannot do what you need.
- Be mindful of performance: avoid heavy loops or blocking code.
- Do **not** attempt to access external systems or APIs — this is sandboxed.
- Remember that scripts **cannot persist changes** to the wider game world unless exposed via defined hooks or APIs.

---

## Safety

Scripts are sandboxed. They:
- Cannot access the file system or network.
- Cannot exceed defined execution timeouts.
- Will return errors on invalid syntax or restricted access.

---

## Output

After running the script, the modified `paragraph` and `player` state are used as-is. This allows the script to:
- Add or remove paragraph links.
- Modify the paragraph output.
- Change player stats or inventory (via the returned object).
- Alter visibility or flag logic dynamically.
- You **cannot** mutate or remove existing paragraph content.
- You **must** use `print(...)` to append output.
- Scripts run during paragraph rendering, and should be fast and side-effect free (avoid changing flags or stats unless absolutely required).
- Abuse of script tags can reduce readability for other authors and increase rendering load.

---

## Reference

See the separate [Embedded JavaScript Reference Guide] (coming soon) for:
- Variable structures for `player` and `paragraph`
- Allowed functions and syntax
- Error handling behaviour

## 📚 Use Cases

- Adding dynamic messages based on the player's state (e.g., stamina, race, items)
- Providing custom conditional logic that isn't supported by simple `<if>` tags
- Displaying hints, warnings, or flavour text

## 💡 Tips

- Always test your scripts thoroughly.
- If a script becomes too long or complex, consider moving it to a reusable passive effect or backend condition.
- Don’t forget to close the tag with `</script>`.
