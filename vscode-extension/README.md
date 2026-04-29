# Kore Language Support for VS Code

Syntax highlighting and seamless Code Runner integration for the Kore programming language.

## Features
- Syntax highlighting for `.k`, `.kmt`, and `.kp` files.
- Automatic bracket closing and surrounding pairs.
- **One-click execution** via the built-in **Code Runner** extension.

## How to run Kore scripts
1. Install the **Code Runner** extension (`formulahendry.code-runner`) in VS Code.
2. Open any `.k` file.
3. Press the **Run Code** button (▶️) in the top-right corner, or use the shortcut `Ctrl+Alt+N`.
4. Code Runner will automatically execute `kore yourfile.k` in the terminal.

*No additional configuration is required if the `kore` command is in your PATH.*

## Manual Code Runner configuration
If the command is not picked up automatically, add this to your VS Code `settings.json`:
```json
"code-runner.executorMap": {
    "kore": "kore"
}