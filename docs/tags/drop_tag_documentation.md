# `<drop>` Tag Documentation

The `<drop>` tag is used to discard an item, spell, or herb from the player's inventory in *The Seven Spells of Destruction*. This tag can appear in a paragraph and will remove the specified item from the player’s possession when encountered.

## Syntax

```text
<drop Item Name>
```

- **Item Name**: The name of the item, spell, or herb to be dropped. It must match exactly (case-sensitive) the name in the player's inventory.

## Behaviour

- This tag checks the player's inventory for the named item.
- If the item is found as a possession, spell, or herb, it is removed.
- Once dropped, the item is no longer available to the player unless reacquired.
- Triggers the `"DISCARD"` achievement check for analytics or rewards.

## Example

```text
<drop Rusty Sword>
```

This would discard the item "Rusty Sword" from the player's inventory, spell list, or herb pouch, if present.

## Notes

- The item name may consist of multiple words. The tag ends at the first `>` character.
- This tag does not produce any visible output or player feedback unless paired with a message elsewhere in the paragraph.
- It is safe to use this tag even if the player does not possess the item—nothing will happen.

## Implementation Details

In code, the tag extracts the item name, removes the closing angle bracket, and then performs the following:
- `drop_possession`
- `drop_spell`
- `drop_herb`

It finally performs an `achievement_check` for the `"DISCARD"` achievement, with the item's name as context.
