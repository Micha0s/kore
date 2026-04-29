# Kore Programming Language 🚀

**The explicit, readable, and reactive language.**
*Fast core (C), modern syntax, built for learning and power.*


---

## ✨ Why Kore?

- 🔹 **Explicit syntax** – every statement starts with `--`
- 🔹 **Strict static typing** – no hidden type magic
- 🔹 **Reactive variables** – `--type watch` triggers callbacks automatically
- 🔹 **Probabilistic programming** – `maybe` type with weighted `--observe`
- 🔹 **Built for teaching** – `--mode learn` explains every step
- 🔹 **Fast C core** – the reference implementation is written in pure C

---

## 🚀 Quick Start

### Installation
Download the latest installer from the [Releases page](https://github.com/Micha0s/kore/releases) and run it. [Now language is only in beta version, so Releases page is empty]

### First Program
Create a file `hello.k`:

```kore
--init msg [{string}, {'Hello, Kore!'}] --type var
print(msg);```
