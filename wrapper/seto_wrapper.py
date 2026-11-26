#!/usr/bin/env python3
"""
Backward-compatible Python I/O wrapper for Seto (Section 6 adapters).
Supports JSON traces (generic, HTTP/API, OpenTelemetry-style) and XES logs:
  1. Optional upstream abstraction/filtering (noise reduction).
  2. Maps real string labels to compact indices e0..e{N-1} and emits a .g TS.
  3. Runs Seto in an isolated temporary directory (--work-dir) for parallel safety.
  4. Remaps output .dot files back to the original labels.
"""

from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Iterable


def load_traces_from_json(path: Path) -> list[list[str]]:
    data = json.loads(path.read_text(encoding="utf-8"))
    if isinstance(data, dict) and "traces" in data:
        data = data["traces"]
    if isinstance(data, dict) and "resourceSpans" in data:
        # Minimal OpenTelemetry JSON spans extractor grouped by traceId
        by_trace: dict[str, list[tuple[int, str]]] = {}
        for rs in data.get("resourceSpans", []):
            for ss in rs.get("scopeSpans", []):
                for sp in ss.get("spans", []):
                    tid = str(sp.get("traceId", "default"))
                    ts = int(sp.get("startTimeUnixNano", 0))
                    name = str(sp.get("name", "unknown"))
                    by_trace.setdefault(tid, []).append((ts, name))
        return [[name for _, name in sorted(evts)] for evts in by_trace.values()]
    if not isinstance(data, list):
        raise ValueError("JSON must be a list of traces, {'traces': [...]}, or OTel resourceSpans")

    traces: list[list[str]] = []
    for item in data:
        if isinstance(item, list):
            traces.append([str(x) for x in item])
        elif isinstance(item, dict) and "events" in item:
            traces.append([str(x) for x in item["events"]])
    return traces


def load_traces_from_xes(path: Path) -> list[list[str]]:
    tree = ET.parse(path)
    root = tree.getroot()
    traces: list[list[str]] = []
    for trace_el in root.iter():
        if not trace_el.tag.endswith("trace"):
            continue
        events: list[str] = []
        for ev_el in trace_el:
            if not ev_el.tag.endswith("event"):
                continue
            name = None
            for attr in ev_el:
                if attr.attrib.get("key") == "concept:name":
                    name = attr.attrib.get("value")
                    break
            if name:
                events.append(name)
        if events:
            traces.append(events)
    return traces


def reduce_traces(
    traces: list[list[str]],
    drop_labels: set[str],
    collapse_consecutive: bool,
) -> list[list[str]]:
    reduced: list[list[str]] = []
    for tr in traces:
        out: list[str] = []
        for ev in tr:
            if ev in drop_labels:
                continue
            if collapse_consecutive and out and out[-1] == ev:
                continue
            out.append(ev)
        if out:
            reduced.append(out)
    return reduced


def traces_to_ts_g(
    traces: Iterable[list[str]],
    out_g_path: Path,
) -> dict[str, str]:
    """
    Build a prefix/state-merged transition system in .g format.
    Returns mapping: internal_label ('e0', 'e1', ...) -> original_label.
    """
    label_to_id: dict[str, str] = {}
    id_to_label: dict[str, str] = {}
    edges: set[tuple[str, str, str]] = set()

    # Prefix-tree (trie) over traces so shared prefixes share states
    transitions: dict[tuple[str, str], str] = {}
    next_state = 1
    init_state = "s0"

    for tr in traces:
        cur = init_state
        for raw_label in tr:
            if raw_label not in label_to_id:
                eid = f"e{len(label_to_id)}"
                label_to_id[raw_label] = eid
                id_to_label[eid] = raw_label
            eid = label_to_id[raw_label]
            key = (cur, eid)
            if key not in transitions:
                nxt = f"s{next_state}"
                next_state += 1
                transitions[key] = nxt
            nxt = transitions[key]
            edges.add((cur, eid, nxt))
            cur = nxt

    lines = [
        ".model wrapped_ts",
        f".inputs {' '.join(id_to_label.keys())}",
        ".state graph",
    ]
    for src, eid, dst in sorted(edges):
        lines.append(f"{src} {eid} {dst}")
    lines.append(f".marking {{{init_state}}}")
    lines.append(".end")
    out_g_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return id_to_label


def remap_dot_text(dot_text: str, id_to_label: dict[str, str]) -> str:
    if not id_to_label:
        return dot_text
    pattern = re.compile(r"\b(" + "|".join(re.escape(k) for k in sorted(id_to_label, key=len, reverse=True)) + r")\b")
    return pattern.sub(lambda m: id_to_label[m.group(1)].replace('"', '\\"'), dot_text)


def main() -> int:
    parser = argparse.ArgumentParser(description="Seto backward-compatible trace wrapper")
    parser.add_argument("input", type=Path, help="Input trace file (.json, .xes, or .g/.ts)")
    parser.add_argument("--seto-bin", type=Path, default=Path("build_perf/Seto.exe"), help="Path to Seto executable")
    parser.add_argument("--mode", nargs="+", default=["SM", "DOT"], help="Seto flags (e.g. SM DOT, PN DOT, FC)")
    parser.add_argument("--out-dir", type=Path, default=Path("seto_output"), help="Directory for remapped outputs")
    parser.add_argument("--collapse-consecutive", action="store_true", help="Collapse consecutive identical events")
    parser.add_argument("--drop", nargs="*", default=[], help="Event labels to filter out before synthesis")
    args = parser.parse_args()

    args.out_dir.mkdir(parents=True, exist_ok=True)
    seto_bin = args.seto_bin.resolve()
    if not seto_bin.exists():
        raise FileNotFoundError(f"Seto binary not found: {seto_bin}")

    with tempfile.TemporaryDirectory(prefix="seto_worker_") as tmp_str:
        tmp_dir = Path(tmp_str)
        id_to_label: dict[str, str] = {}

        suffix = args.input.suffix.lower()
        if suffix == ".json":
            traces = load_traces_from_json(args.input)
            traces = reduce_traces(traces, set(args.drop), args.collapse_consecutive)
            ts_path = tmp_dir / "input.g"
            id_to_label = traces_to_ts_g(traces, ts_path)
        elif suffix == ".xes":
            traces = load_traces_from_xes(args.input)
            traces = reduce_traces(traces, set(args.drop), args.collapse_consecutive)
            ts_path = tmp_dir / "input.g"
            id_to_label = traces_to_ts_g(traces, ts_path)
        else:
            ts_path = args.input.resolve()

        cmd = [str(seto_bin), str(ts_path), *args.mode, f"--work-dir={tmp_dir}"]
        proc = subprocess.run(cmd, capture_output=True, text=True)
        if proc.returncode != 0:
            print(proc.stdout)
            print(proc.stderr)
            return proc.returncode

        for out_file in tmp_dir.glob("*.dot"):
            content = out_file.read_text(encoding="utf-8", errors="replace")
            remapped = remap_dot_text(content, id_to_label)
            (args.out_dir / out_file.name).write_text(remapped, encoding="utf-8")

        for out_file in tmp_dir.glob("*.g"):
            if out_file.name != "input.g":
                shutil.copy2(out_file, args.out_dir / out_file.name)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
