# `<AUTOLINK>` Tag Documentation

The `<AUTOLINK>` tag in Seven Spells is used to automatically create navigation buttons that are conditionally enabled based on the outcome of a test—such as `<TEST>` or `<SNEAKTEST>`. This tag **does not display inline** and is exclusively used in tandem with test-based logic.

---

## 📌 Purpose

`<AUTOLINK>` provides branching logic: only one of the two defined `AUTOLINK` tags will be enabled depending on whether the player **passes** or **fails** a test.

---

## 🔧 Syntax

```
<AUTOLINK=destination_id>
```

- `destination_id`: The numeric ID of the paragraph to jump to if this link is enabled.

---

## 🧠 Behaviour

- Must always appear **twice**, one immediately following the other.
- Should only be used when preceded by a `<TEST>` or `<SNEAKTEST>` tag.
- **The first `<AUTOLINK>` is activated if the player passes the test.**
- **The second `<AUTOLINK>` is activated if the player fails the test.**
- The player is not shown both links—only the one relevant to their outcome.

---

## ✅ Use Case Example

```text
You attempt to sneak past the guard.
<SNEAKTEST name="City Guard" sneak=9>
<AUTOLINK=1234>
<AUTOLINK=5678>
```

- If the player passes the sneak test, they are taken to paragraph `1234`.
- If they fail, they are instead taken to paragraph `5678`.

---

## ⚠️ Notes for Content Authors

- `<AUTOLINK>` is **not clickable by itself**; it creates hidden conditional navigation buttons.
- Ensure your `<TEST>` or `<SNEAKTEST>` precedes it, or the outcome logic won’t trigger.
- Only the **first valid test** result is used when multiple `<TEST>` or `<SNEAKTEST>` appear in one paragraph.

---

## 🚫 Common Mistakes

| Mistake | Why it’s a problem |
|--------|---------------------|
| Using only one `<AUTOLINK>` | You must provide **both** success and failure branches |
| Using `<AUTOLINK>` without a test | The system won’t know which link to activate |
| Placing unrelated tags between test and autolinks | May disrupt outcome resolution |

---

## 🔄 Related Tags

- [`<TEST>`](test_tag_doc.md)
- [`<SNEAKTEST>`](sneaktest_tag_doc.md)
- [`<LINK>`](link_tag_doc.md)

