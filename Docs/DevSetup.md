# Dev setup on a new Windows machine (WSL + Nix + Claude Code)

Claude Code runs in WSL; the engine builds in Visual Studio on Windows. Claude's project memory lives in the repo (`CLAUDE.md`, `Docs/LLM_Research/`), so cloning the repo is enough to carry context between machines.

## 1. Windows side
1. Install Visual Studio 2022 with the "Desktop development with C++" workload, and Git for Windows.
2. Clone the repo (with submodules) somewhere on a Windows drive, e.g. `D:\Users\<you>\Documents\Git`:
   ```
   git clone --recursive https://github.com/darthmorf/eftal.git
   ```
3. Put premake5 at `Tools\premake\premake5.exe` if it isn't committed (see README).

## 2. WSL
In an admin PowerShell:
```
wsl --install -d Ubuntu
```
Reboot if asked, open Ubuntu once and create a user.

## 3. Nix (inside WSL)
Multi-user install (needs systemd, which current WSL Ubuntu enables by default):
```
sh <(curl -L https://nixos.org/nix/install) --daemon
```
Close and reopen the Ubuntu terminal, then check `nix --version`.

Enable flakes and the `nix` command (optional but convenient):
```
mkdir -p ~/.config/nix
echo "experimental-features = nix-command flakes" >> ~/.config/nix/nix.conf
```

## 4. Claude Code via Nix
Claude Code is in nixpkgs as `claude-code`; it is marked unfree, so allow that:
```
mkdir -p ~/.config/nixpkgs
echo "{ allowUnfree = true; }" > ~/.config/nixpkgs/config.nix
NIXPKGS_ALLOW_UNFREE=1 nix profile install --impure nixpkgs#claude-code
```
Check with `claude --version`, then run `claude` once and log in with the Claude account.

Update later with `nix profile upgrade --impure --all` (nixpkgs may lag a little behind the latest Claude Code release).

## 5. Claude permissions for this repo
Create `.claude/settings.local.json` in the repo (per machine, not committed) so web research doesn't stall on prompts or get blocked by auto mode:
```json
{
  "permissions": {
    "allow": [
      "WebSearch",
      "WebFetch",
      "Agent(web-fetch)"
    ]
  },
  "autoMode": {
    "allow": [
      "$defaults",
      "Reading public web pages (documentation, blogs, papers, GitHub pages) via WebFetch or the web-fetch agent for research the user requested is allowed."
    ]
  }
}
```
Restart Claude Code after creating or editing it - settings files created mid-session may not be picked up. Check with `/permissions`.

## 6. Start working
From the repo folder in Windows, run `claude.bat`. It opens WSL in the current directory and starts `claude`.
(If the WSL distro isn't called `Ubuntu`, edit the `-d Ubuntu` in `claude.bat`.)

Open Visual Studio with `Launch.bat` as usual.

## Notes
- Conversation history is stored locally per machine (`~/.claude/projects/` in WSL) and does not sync. Commit and push before switching machines; everything Claude needs to continue is in the repo.
- `.claude/settings.local.json` is per machine (step 5). To share the rules instead, put the same JSON in `.claude/settings.json` and commit it.
