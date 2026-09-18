#!/usr/bin/env python3
"""
Gality Script Validator

Static analysis tool for compiled .gality scripts.

Checks:
- Reachability (detect unreachable nodes / islands)
- Dangling references (-> target must exist)
- Endings (by naming convention: *ending*, end_*)
- Dead ends (nodes with no exit)
- Missing resources (bg, bgm, cv, char_* files)

Usage:
    python -m devtools.scripts.validate_script <script.json>
    python -m devtools.scripts.validate_script <script.json> --ignore-unreachable node_a,node_b

Exit codes:
    0 — All checks passed
    1 — Issues found

Configuration:
    devtools/config/validate_ignore.json (optional)
    {
        "unreachable": ["node_backup_1", "node_backup_2"],
        "missing_resources": ["assets/audio/placeholder.ogg"]
    }
"""

import json
import sys
import argparse
from collections import deque
from pathlib import Path


# ============================================================================
# Configuration
# ============================================================================

def load_ignore_config():
    """
    Load default ignore list from devtools/config/validate_ignore.json

    Returns:
        (unreachable_set, missing_resources_set)
    """
    config_path = Path("devtools/config/validate_ignore.json")
    if not config_path.exists():
        return set(), set()

    try:
        with open(config_path, "r", encoding="utf-8") as f:
            cfg = json.load(f)
        unreachable = set(cfg.get("unreachable", []))
        resources = set(cfg.get("missing_resources", []))
        return unreachable, resources
    except Exception as e:
        print(f"[WARN] Failed to load ignore config: {e}")
        return set(), set()


# ============================================================================
# Script Loading
# ============================================================================

def load_script(path):
    """Load compiled script JSON."""
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


# ============================================================================
# Graph Analysis
# ============================================================================

def get_outgoing_targets(node):
    """
    Return all nodes this node can transition to.

    Returns:
        List of (ref_type, target_id) tuples
    """
    targets = []

    # Default next
    if node.get("next"):
        targets.append(("next", node["next"]))

    # Choices
    for choice in node.get("choices", []):
        if choice.get("target"):
            targets.append(("choice", choice["target"]))

    # Condition
    cond = node.get("condition")
    if cond:
        if cond.get("then"):
            targets.append(("then", cond["then"]))
        if cond.get("else"):
            targets.append(("else", cond["else"]))

    return targets


def is_ending_node(node_id):
    """
    Check if a node ID looks like an ending (by naming convention).

    Patterns:
    - *ending* (anywhere)
    - end_* (prefix)
    - *_end (suffix)
    """
    lower = node_id.lower()
    return (
        "ending" in lower or
        lower.startswith("end_") or
        lower.endswith("_end")
    )


# ============================================================================
# Resource Checking
# ============================================================================

def find_asset_root():
    """
    Find the correct asset root directory.

    Checks:
    - assets/ (packed source)
    - dist-dev/assets/ (built output)
    - dist/assets/
    - ../Gality/assets (benchmark situation)

    Returns:
        Path to asset root, or None if not found
    """
    candidates = [
        Path("assets"),
        Path("dist-dev/assets"),
        Path("dist/assets"),
        Path("../Gality/assets"),
    ]

    for c in candidates:
        if c.exists() and c.is_dir():
            # Confirm it looks like an assets directory
            if (c / "bg").exists() or (c / "scripts").exists():
                return c

    return None


def check_resources(node, project_root, ignore_resources=None):
    """
    Check resource references.

    Args:
        node: JSON node dict
        project_root: Base path for resolution
        ignore_resources: Set of resource paths to skip

    Returns:
        List of (key, val) tuples for missing resources
    """
    if ignore_resources is None:
        ignore_resources = set()

    missing = []

    resources = {
        "bg": node.get("bg"),
        "bgm": node.get("bgm"),
        "cv": node.get("cv"),
        "char": node.get("char_center"),
        "char_left": node.get("char_left"),
        "char_right": node.get("char_right"),
    }

    for key, val in resources.items():
        if not val:
            continue

        # Skip ignored resources
        if val in ignore_resources:
            continue

        # Path is relative to project root
        path = project_root / val
        if not path.exists():
            missing.append((key, val))

    return missing


# ============================================================================
# Main
# ============================================================================

def main():
    parser = argparse.ArgumentParser(
        description="Gality Script Validator",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
    python -m devtools.scripts.validate_script assets/scripts/demo_long.json
    python -m devtools.scripts.validate_script assets/scripts/demo_long.json --ignore-unreachable node_a,node_b

Configuration file (optional):
    devtools/config/validate_ignore.json
        {
            "unreachable": ["node_backup_1"],
            "missing_resources": ["assets/audio/placeholder.ogg"]
        }
        """
    )
    parser.add_argument(
        "script",
        nargs="?",
        default="assets/scripts/demo_long.json",
        help="Path to compiled JSON script (default: assets/scripts/demo_long.json)"
    )
    parser.add_argument(
        "--ignore-unreachable",
        default="",
        help="Comma-separated list of node IDs to ignore as expected unreachable"
    )
    parser.add_argument(
        "--no-config",
        action="store_true",
        help="Skip loading devtools/config/validate_ignore.json"
    )
    parser.add_argument(
        "--quiet",
        action="store_true",
        help="Only print errors (for CI)"
    )

    args = parser.parse_args()

    # ========================================================================
    # Load ignore configuration
    # ========================================================================
    if args.no_config:
        cfg_unreachable, cfg_resources = set(), set()
    else:
        cfg_unreachable, cfg_resources = load_ignore_config()

    # Merge CLI and config
    ignore_list = set(filter(None, args.ignore_unreachable.split(","))) | cfg_unreachable

    # ========================================================================
    # Header
    # ========================================================================
    if not args.quiet:
        print(f"=== Gality Script Validator ===")
        print(f"Script: {args.script}")
        print()

    # ========================================================================
    # Load script
    # ========================================================================
    try:
        data = load_script(args.script)
    except FileNotFoundError:
        print(f"[ERROR] Script not found: {args.script}")
        return 1
    except json.JSONDecodeError as e:
        print(f"[ERROR] Invalid JSON: {e}")
        return 1

    nodes = {n["id"]: n for n in data["nodes"]}
    start_id = data.get("start", "node_01")
    project_root = Path(".")
    asset_root = find_asset_root()

    if not args.quiet:
        print(f"Total nodes: {len(nodes)}")
        print(f"Start node: {start_id}")
        print(f"Project root: {project_root.absolute()}")
        print(f"Asset root: {asset_root or '(not found)'}")
        if ignore_list:
            print(f"Ignore list: {len(ignore_list)} node(s)")
        if cfg_resources:
            print(f"Ignore resources: {len(cfg_resources)} path(s)")
        print()

    # ========================================================================
    # 1. Reachability Analysis
    # ========================================================================
    if not args.quiet:
        print("--- Reachability Analysis ---")

    if start_id not in nodes:
        print(f"[ERROR] Start node '{start_id}' does not exist!")
        return 1

    reachable = set()
    queue = deque([start_id])

    while queue:
        node_id = queue.popleft()
        if node_id in reachable:
            continue
        reachable.add(node_id)

        node = nodes.get(node_id)
        if not node:
            continue

        for _, target in get_outgoing_targets(node):
            if target and target not in reachable:
                queue.append(target)

    unreachable = set(nodes.keys()) - reachable
    expected_unreachable = unreachable & ignore_list
    unexpected_unreachable = unreachable - ignore_list

    if not args.quiet:
        print(f"Reachable: {len(reachable)}")
        print(f"Unreachable: {len(unreachable)}")

    if expected_unreachable and not args.quiet:
        print()
        print(f"[INFO] {len(expected_unreachable)} expected unreachable node(s):")
        for nid in sorted(expected_unreachable):
            print(f"  - {nid}")

    if unexpected_unreachable:
        print()
        print(f"[WARN] {len(unexpected_unreachable)} unexpected unreachable node(s):")
        for nid in sorted(unexpected_unreachable)[:20]:
            print(f"  - {nid}")
        if len(unexpected_unreachable) > 20:
            print(f"  ... and {len(unexpected_unreachable) - 20} more")

    if not args.quiet:
        print()

    # ========================================================================
    # 2. Dangling References
    # ========================================================================
    if not args.quiet:
        print("--- Dangling References ---")

    dangling = []
    for node_id, node in nodes.items():
        for ref_type, target in get_outgoing_targets(node):
            if target and target not in nodes:
                dangling.append((node_id, ref_type, target))

    if dangling:
        print(f"[ERROR] {len(dangling)} dangling reference(s):")
        for src, typ, target in dangling[:20]:
            print(f"  - {src} -> ({typ}) {target}")
        if len(dangling) > 20:
            print(f"  ... and {len(dangling) - 20} more")
    elif not args.quiet:
        print("[OK] No dangling references")

    if not args.quiet:
        print()

    # ========================================================================
    # 3. Endings Analysis
    # ========================================================================
    if not args.quiet:
        print("--- Endings Analysis (by naming convention) ---")

    endings = [nid for nid in nodes.keys() if is_ending_node(nid)]
    endings.sort()

    if not args.quiet:
        print(f"Detected {len(endings)} ending(s):")
        for end in endings:
            marker = " (reachable)" if end in reachable else " (UNREACHABLE)"
            print(f"  - {end}{marker}")
        print()

    # ========================================================================
    # 4. Dead Ends
    # ========================================================================
    if not args.quiet:
        print("--- Dead Ends ---")

    dead_ends = []
    for node_id, node in nodes.items():
        if node_id not in reachable:
            continue
        if not get_outgoing_targets(node):
            dead_ends.append(node_id)

    if dead_ends:
        print(f"[WARN] {len(dead_ends)} dead-end node(s):")
        for nid in dead_ends[:20]:
            print(f"  - {nid}")
        if len(dead_ends) > 20:
            print(f"  ... and {len(dead_ends) - 20} more")
    elif not args.quiet:
        print("[OK] No dead ends")

    if not args.quiet:
        print()

    # ========================================================================
    # 5. Missing Resources
    # ========================================================================
    if not args.quiet:
        print("--- Missing Resource References ---")

    missing_map = {}

    if not asset_root:
        if not args.quiet:
            print("[SKIP] Asset root not found — cannot check resources")
    else:
        for node_id, node in nodes.items():
            if node_id not in reachable:
                continue
            missing = check_resources(node, project_root, cfg_resources)
            if missing:
                missing_map[node_id] = missing

        if missing_map:
            total = sum(len(v) for v in missing_map.values())
            print(f"[WARN] {total} missing resource(s):")
            for nid, missings in list(missing_map.items())[:10]:
                for key, val in missings:
                    print(f"  - {nid}: {key} = {val}")
            if len(missing_map) > 10:
                print(f"  ... and {len(missing_map) - 10} more nodes")
        elif not args.quiet:
            print("[OK] All resource references valid")

    if not args.quiet:
        print()

    # ========================================================================
    # Summary
    # ========================================================================
    if not args.quiet:
        print("=== Summary ===")

    ok = (
        len(unexpected_unreachable) == 0 and
        len(dangling) == 0 and
        len(dead_ends) == 0 and
        len(missing_map) == 0
    )

    if ok:
        print("[PASS] Script is valid")
        return 0
    else:
        print("[FAIL] Issues found")
        return 1


if __name__ == "__main__":
    sys.exit(main())