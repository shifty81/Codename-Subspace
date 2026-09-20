# Null Harbor identity change — verification is mandatory before the GitHub rename

The current root `project.control.json` has old IDs, old remote URL, old launcher and old state-directory path. Do not rename these blindly. The new name is **Null Harbor**; `Null Harbor Studio` is its standalone editor, not a separate game. The rebrand must preserve the one authoritative C++ project.

Run from repository root:

```powershell
python .\tools\identity\null_harbor_rebrand_audit.py --root . --json-out .\artifacts\audits\null-harbor-identity-inventory.json
python .\tools\identity\null_harbor_rebrand_audit.py --root . --verify
```

The second command should initially exit nonzero and is not a Full Quality Gate failure; it signals that rename work remains. It never changes the repo. Review the machine-readable receipt; classify active display strings, old URLs to change *after* GitHub rename, versioned schema and PCC IDs requiring a compatibility reader, historical docs that must remain historically truthful, and third-party attribution that cannot be rewritten.

Done only when: all active UI/installer/README/manifest records use Null Harbor; old schema/save/patch receipts open with verified migration; PCC internal patch intake and full build both pass; game+Studio executable and launchers run; histories and attribution are preserved; remote references are updated AFTER user renames the repository. Never automatically delete `.subspace` or rename C++ namespace/CLI/filenames by global replacement.

Official GitHub behavior: https://docs.github.com/en/repositories/creating-and-managing-repositories/renaming-a-repository . Repository name redirects generally work; GitHub-hosted Actions references and Pages URLs need explicit review. Only the user performs the repository rename once the verification gates succeed.
