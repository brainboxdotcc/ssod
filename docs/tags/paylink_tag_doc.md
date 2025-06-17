# `<PAYLINK>` Tag

The `<PAYLINK>` tag is used to create a navigation link that is only enabled if the player has enough **gold** to pay the specified cost. It is typically used for situations like:

- Paying for a night at an inn
- Paying for passage on a ship
- Paying an entrance fee to a private area

This tag **should not** be used for buying items. For item purchases, use the `<I>` tag instead.

---

## Syntax

```
<PAYLINK=gold_amount,destination_id>
```

- `gold_amount`: How much gold the player must pay.
- `destination_id`: The paragraph ID the player is sent to if they pay.

---

## Behaviour

- If the player has at least the specified amount of gold:
  - A link button is displayed with the appropriate emoji (1️⃣, 2️⃣, etc).
  - Clicking the link deducts the gold and navigates the player to the destination paragraph.
- If the player does **not** have enough gold:
  - The button still appears, but is **disabled** with a label like “Not enough gold”.
- The paragraph is **not marked safe** by this tag (whether it is safe or not depends on other content).

---

## Example

```text
You approach the dockmaster.

<PAYLINK=10,781> 
```

If the player has 10 or more gold, a link button will be displayed to go to paragraph 781, and 10 gold will be deducted when it is clicked.

If not, the button will be disabled.

---

## Notes

- The player sees the gold requirement on hover or after clicking. Button text is not shown inline.
- This is a **navigation choice**, not a shop system. Do not confuse it with `<I>`, which is used to buy items.

---

## Visual Summary

- ✅ Enabled if: player.gold ≥ required amount
- ❌ Disabled if: player.gold < required amount
- 📦 Gold deducted **only** if player clicks the link
- 🔐 Link is disabled otherwise

