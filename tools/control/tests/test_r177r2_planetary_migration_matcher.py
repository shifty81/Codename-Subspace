from pathlib import Path
import importlib.util, json, sys

root=Path(sys.argv[1]).resolve()
spec=importlib.util.spec_from_file_location("migration",root/"tools/planetary/apply_planetary_command_r158_r177.py")
m=importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)
plan=json.loads((root/"tools/planetary/r158_r177_transforms.json").read_text(encoding="utf-8"))

seen_indent_normalized=False
for t in plan["transforms"]:
    source="\n".join("    "+line if line else line for line in t["before"].splitlines())
    wrapped="BEFORE_SENTINEL\n"+source+"\nAFTER_SENTINEL\n"
    out,changed,mode=m.replace_approved(
        wrapped,t["before"],t["after"],t["marker"],t["label"],t["path"])
    assert changed, t["label"]
    assert mode in ("exact","indent-normalized"), (t["label"],mode)
    assert t["marker"] in out, t["label"]
    seen_indent_normalized |= mode=="indent-normalized"

assert seen_indent_normalized

amb="    A\n    B\nx\n        A\n        B\n"
try:
    m.replace_approved(amb,"A\nB","C","C","ambiguous","fixture")
    raise AssertionError("ambiguous match should fail")
except RuntimeError as e:
    assert "indentation-normalized=2" in str(e)

print("PASS: R177R2 indentation-safe certified-preimage matcher")
