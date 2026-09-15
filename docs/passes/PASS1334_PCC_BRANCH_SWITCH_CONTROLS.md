# Pass1334 — Internal PCC Safe Branch Switching

## Purpose

The standalone Codename Subspace Project Control Center now exposes branch switching
directly from the normal project-owned PCC.

Main menu:

`8. Branches / switch / create / toggle`

## Supported operations

- list local branches with current branch, short HEAD, upstream, and tracking state;
- switch between clean local branches;
- switch to an `origin/*` branch and create its local tracking branch when required;
- create a new branch from the current HEAD;
- toggle back to the previous branch with the equivalent of `git switch -`;
- fetch/prune `origin`.

## Safety rules

Branch switching is fail-closed.

It refuses to switch while:

- root `.patch` files are pending;
- the working tree has staged, modified, or untracked files.

It does **not** automatically:

- stash;
- reset;
- force checkout;
- delete branches;
- merge;
- rebase;
- force push.

Git remains the authority and normal Git worktree/branch protections are allowed to
reject a switch.

## GREEN gate behavior

Before a branch change the PCC records the current branch and HEAD.

If the destination has the exact same HEAD:

- the source is byte-identical at Git authority level;
- the current GREEN certification is retained.

If the destination HEAD differs:

- `.subspace/last-green-quality-gate.json` is archived under
  `.subspace/control-center/branch-gates/`;
- the normal current GREEN marker is cleared;
- `.subspace/control-center/branch-switch-state.json` records the transition;
- PCC option 2 is blocked;
- a new option 1 Full Quality Gate is required.

A successful Full Gate clears the branch-switch requirement.

## Older branches

The branch manager warns before switching to a target branch that does not contain the
current Pass1334 branch manager. The currently running PCC remains available for the
session, so the user can immediately toggle back, but a fresh PCC launch on that older
branch naturally exposes whatever PCC version that branch contains.
