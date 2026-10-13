#!/usr/bin/env python3
"""Create a signed Trinity module package manifest.

The signature is Ed25519 over UTF-8 canonical JSON (RFC 8785-style member
ordering, compact separators, and ensure_ascii=False) with ``signature``
removed.  ``files`` covers every regular package file except module.json.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
from typing import Any

from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey


def _reject_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate JSON member: {key}")
        result[key] = value
    return result


def canonical_data(manifest: dict[str, Any]) -> bytes:
    signed = {key: value for key, value in manifest.items() if key != "signature"}
    return json.dumps(signed, ensure_ascii=False, separators=(",", ":"), sort_keys=True).encode("utf-8")


def package_files(root: Path, library: Path) -> dict[str, str]:
    files: dict[str, str] = {}
    library = library.resolve()
    for candidate in sorted(root.rglob("*")):
        if not candidate.is_file() or candidate.name == "module.json":
            continue
        resolved = candidate.resolve()
        if resolved != candidate and not resolved.is_relative_to(root.resolve()):
            raise ValueError(f"package file escapes package root: {candidate}")
        relative = candidate.relative_to(root).as_posix()
        files[relative] = hashlib.sha256(candidate.read_bytes()).hexdigest()
    library_relative = library.relative_to(root.resolve()).as_posix()
    if library_relative not in files:
        raise ValueError("library must be a regular file inside package root")
    return files


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--library", type=Path, required=True)
    parser.add_argument("--private-key", type=Path, required=True)
    parser.add_argument("--key-id", required=True)
    parser.add_argument("--package-root", type=Path)
    args = parser.parse_args()

    if not args.key_id or any(char not in "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._-" for char in args.key_id):
        raise SystemExit("key-id must be a non-empty safe basename")
    root = (args.package_root or args.manifest.parent).resolve()
    manifest_path = args.manifest.resolve()
    if manifest_path.parent != root:
        raise SystemExit("manifest must be directly under package root")
    try:
        manifest = json.loads(args.manifest.read_text(encoding="utf-8"), object_pairs_hook=_reject_duplicates)
    except (json.JSONDecodeError, UnicodeDecodeError, ValueError) as error:
        raise SystemExit(f"invalid manifest: {error}") from error
    if not isinstance(manifest, dict):
        raise SystemExit("manifest must be a JSON object")
    manifest["key_id"] = args.key_id
    manifest["files"] = package_files(root, args.library)
    manifest["library_sha256"] = manifest["files"][args.library.resolve().relative_to(root).as_posix()]
    manifest.pop("signature", None)
    private_bytes = args.private_key.read_bytes()
    if len(private_bytes) != 32:
        raise SystemExit("private key must contain exactly 32 raw bytes")
    signature = Ed25519PrivateKey.from_private_bytes(private_bytes).sign(canonical_data(manifest))
    manifest["signature"] = signature.hex()
    args.manifest.write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
