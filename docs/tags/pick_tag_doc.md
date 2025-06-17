
# `<pick>` Tag — Mutually Exclusive Free Item Choices

The `<pick>` tag allows the player to **choose one free item** from a list of mutually exclusive options.  
You may include **multiple `<pick>` tags** in the same paragraph, but the player can only select **one** of them.  
Once any `<pick>` link is chosen, the others are disabled for that player.

This tag is commonly used in situations where the player is offered a selection of tools, weapons, or boons —  
e.g. “You may pick one of the following.”

## Example Usage

```plaintext
You find a stash of items hidden beneath the floorboards. You may take one:

<pick name="Short Sword" value="[sharp]">
<pick name="Leather Armour" value="[light]">
<pick name="Healing Potion" value="[strong]">
```

- If the player’s inventory is full, the links will be greyed out and marked as “Inventory Full.”
- Once a pick is made, all other pick options are locked for that paragraph.

---

