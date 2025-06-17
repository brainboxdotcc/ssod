# `<LINK>` Tag Documentation

The `<LINK>` tag is used to create a clickable choice that navigates the player to a different paragraph or location within the game.

---

## Syntax

```
<LINK=destination_id,Optional Button Text>
```

- **destination_id**: The numeric ID of the paragraph to navigate to.
- **Optional Button Text**: This text appears only on the clickable button. It is **not** printed inline in the paragraph text.

---

## Behaviour

- Each `<LINK>` creates a navigable option shown as a button.
- Buttons are automatically prefixed with an emoji corresponding to their order:
  - First link: `1️⃣`
  - Second link: `2️⃣`
  - ...and so on.
- The button text is only visible on the button, not in the paragraph.
- The link is added to the paragraph’s navigation options and enables player movement.

---

## Example

```
You stand before a fork in the road.

Go left, <LINK=1024,Take the left path> or
go right <LINK=2048,Take the right path>
```

This renders for the player as:

```
Text content:
You stand before a fork in the road. Go left ⃣  or go right  ⃣

Two buttons will be added below the content:
1️⃣ Take the left path  
2️⃣ Take the right path
```

---

## Notes for Content Authors

- You can use multiple `<LINK>` tags per paragraph.
- Button text is not displayed inline, only its emoji placeholder.
- Links are shown in the order written.
- If no button text is provided, the system may fall back to a default or cause confusion — always include a label.
- Ensure the destination ID exists.
- For dynamic or conditional outcomes, consider pairing with `<IF>` or `<AUTOLINK>` tags.

---

## Best Practices

- Always include clear, descriptive button text.
- Avoid using vague terms like "Click here".
- Use `<LINK>` for basic navigation and branching.
- Combine with logic tags (like `<IF>`, `<TEST>`, or `<SNEAKTEST>`) to create meaningful interactive stories.