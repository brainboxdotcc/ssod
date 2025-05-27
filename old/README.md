# Seven Spells (2001 Edition) - Historical Source Dump

This is the original **Seven Spells of Destruction** web engine, written circa 2001 in raw HTML, JavaScript, and **C++ CGI**. It is presented **only for educational and historical interest**.

> **⚠️ WARNING: DO NOT RUN THIS CODE IN ANY PRODUCTION OR PUBLIC ENVIRONMENT.**

---

## ❌ This Code Is Dangerous

This early version was written long before modern web standards, security best practices, or compiler sanitisation tools were widespread. It contains **multiple critical vulnerabilities** including:

- [**SQL injection**](https://github.com/brainboxdotcc/ssod/blob/main/old/cgi-bin/main.cpp#L447) vectors throughout the query layer  
- [**Cross-site scripting (XSS)**](https://github.com/brainboxdotcc/ssod/blob/main/old/cgi-bin/main.cpp#L421) via unescaped user input echoed to HTML  
- [**Buffer overflows**](https://github.com/brainboxdotcc/ssod/blob/main/old/cgi-bin/config.h#L6) and unsafe use of functions like `strcpy` and `sprintf`  
- Numerous assumptions about user trust, input formatting, and execution order

These flaws are well-known and **will never be fixed**.

---

## 🧑‍💻 About the Author (2001)

At the time this was written, I was still **very much a beginner**. The code reflects an early stage in my learning journey with C++, web programming, and server-side scripting. Like many developers of that era, I learned by experimentation - often without understanding the deeper consequences of unchecked input, memory safety, or security hygiene.

While I’m proud of what I accomplished with the tools and knowledge I had, it’s important to see this for what it is: **a snapshot of amateur-era coding**, not a modern or safe foundation.

---

## 🛠 Requirements (2001 Stack)

To even compile or run this, you'd need a near-replica of the original early-2000s server stack:

- `g++` version **2.95.1**
- `libmysqlclient` for **MySQL 3.x**
- An HTTP server that can run traditional **.cgi** binaries (e.g. Apache with `mod_cgi`)
- A Linux distro from the early 2000s such as:
  - **Mandrake Linux 7.2** (released October 2000)
  - Red Hat Linux 7.0 (2000)
  - Slackware 7.1 (2000)
  - Debian 2.2 "Potato" (August 2000)

Even if you get it running, expect **segfaults**, encoding bugs, and undefined behaviour.

---

## 🤖 Why This Code Is Abandoned

The modern *Seven Spells* Discord bot engine is built entirely from scratch using modern C++20 and the [D++](https://github.com/brainboxdotcc/DPP) library.  
**None of the original C++ CGI code was reused** - deliberately.

Only the **game data** (locations, items, character stats) was exported and preserved. This 2001 engine is now **archival** only.

---

## 📚 Educational Use Only

If you're studying:
- Early web RPG architecture
- CGI scripting in C++
- Pre-ORM SQL patterns
- What not to do in a secure application

...then you're welcome to explore the source. Just don’t deploy it.

---

## 📦 Contents

- `cgi-bin/` - Game code (C++ CGI scripts)
- `images/` - Game UI graphics and icons

