#!/usr/bin/env python3
"""Copy the checked-in Trinity C ABI layout.

The public ABI is intentionally not derived from ScriptMgr headers. Core
commits may add or reorder C++ virtuals without changing this contract. Any
ABI change is an explicit schema/header revision and is reviewed as such.
"""
from __future__ import annotations

import json
import pathlib
import shutil
import sys

schema_path = pathlib.Path(sys.argv[1])
out = pathlib.Path(sys.argv[2])
schema = json.loads(schema_path.read_text(encoding="utf-8"))
if schema.get("abi_version") != 1:
    raise SystemExit("trinity ABI schema must explicitly declare abi_version=1")
if not schema.get("layout_revision"):
    raise SystemExit("trinity ABI schema must explicitly declare layout_revision")
frozen = schema.get("frozen_header")
if not frozen:
    raise SystemExit("trinity ABI schema must name frozen_header")
source = (schema_path.parent / frozen).resolve()
if not source.is_file():
    raise SystemExit(f"frozen ABI header does not exist: {source}")
out.parent.mkdir(parents=True, exist_ok=True)
if source != out.resolve():
    shutil.copyfile(source, out)
generated = out.read_text(encoding="utf-8")
required = ("TRINITY_ABI_V1", "TRINITY_CAPABILITY_CONFIG_V1", "TRINITY_CAPABILITY_COMMAND_V1")
if any(token not in generated for token in required):
    raise SystemExit("frozen ABI header is missing required v1 capability declarations")
