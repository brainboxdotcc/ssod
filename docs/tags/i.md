# Tag Documentation: `<I>` – Buyable Item

The `<I>` tag defines a buyable item, typically used in shops or trading contexts. It represents an interactive option allowing the player to purchase and acquire an item, herb, spell, or special utility gear. When rendered, the player sees the item name, gold cost, and any associated stat effects.

---

## ✅ Syntax

```html
<I NAME="Item Name" COST="X">
<I NAME="Item Name" VALUE="CODE" COST="X">
```

- `NAME` (required): The display name of the item.
- `COST` (required): The cost in gold coins to purchase the item.
- `VALUE` (optional): Encodes the functional type or stat benefit of the item.

---

## 🧪 VALUE Formats

The `VALUE` attribute determines what kind of effect or flag is granted when the item is acquired.

### 📦 Stat Modifier
Affects a specific player stat when used.

```
ST+N / ST-N  → Stamina
SK+N / SK-N  → Skill
MA+N / MA-N  → Mana
SD+N / SD-N  → Speed
LK+N / LK-N  → Luck
EX+N / EX-N  → Experience
An         → Armour rating
Wn         → Weapon rating
```

**Example:**
```html
<I NAME="Stamina Elixir" VALUE="ST+6" COST="10">
```

### 🔮 Special Item Types

Some keywords trigger unique behaviours:

- `SPELL` → Adds the item as a spell
- `HERB` → Adds the item as an herb
- omitted → Basic backpack item with no flags or special effect

**Examples:**
```html
<I NAME="FLY" VALUE="SPELL" COST="1">
<I NAME="Healing Root" VALUE="HERB" COST="4">
<I NAME="Pointless Junk" COST="100">
```

---

## 🧰 Special Cases (Built-in Behaviour)

Certain `NAME` values trigger unique shop logic:

| Item Name        | Behaviour / Condition                                          |
|------------------|----------------------------------------------------------------|
| `horse`, `donkey`, `mule`, `pack pony` | Sets the `horse` flag; disallowed if already owned |
| `backpack`, `pack`     | Sets the `pack` flag; marked owned if already present       |
| `saddle bags`          | Sets the `saddlebags` flag; shown as owned if present       |

If any of these are already owned, the player sees a confirmation icon or special message.

---

## 📦 Inventory Limit Handling

- Players may only buy items if they have room in their backpack.
- If inventory is full, the link will be rendered but disabled with the label: `Inventory Full`.

---

## 💬 Display Format

The item will appear in the paragraph as:

```
**Buy: Healing Potion ✅ [In Inventory]** (*10 gold*) - Restores 6 stamina
```

…and will be interactively linked with a direction (e.g. `(A)`).

---

## 🧪 Examples

```html
<I NAME="Stamina Restorer +6" VALUE="ST+6" COST="10">
<I NAME="Pointless Junk" COST="100">
<I NAME="Fly" VALUE="SPELL" COST="1">
<I NAME="Backpack" COST="60">
<I NAME="Pack Pony" COST="120">
<I NAME="Healing Herb" VALUE="HERB" COST="5">
```
