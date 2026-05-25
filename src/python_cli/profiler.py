"""
MERO Compiler - Performance Profiler
Measures compilation performance and identifies bottlenecks.
Developer: MERO:TG@QP4RM
"""
import time
from typing import Dict, List
from dataclasses import dataclass, field


@dataclass
class PhaseMetrics:
    name: str
    duration_ms: float = 0.0
    memory_bytes: int = 0
    items_processed: int = 0


@dataclass
class ProfileReport:
    phases: List[PhaseMetrics] = field(default_factory=list)
    total_ms: float = 0.0
    peak_memory: int = 0
    input_size: int = 0
    output_size: int = 0

    def summary(self) -> str:
        lines = ["=== MERO Compilation Profile ==="]
        lines.append(f"Total: {self.total_ms:.2f}ms")
        lines.append(f"Input: {self.input_size} bytes")
        lines.append(f"Output: {self.output_size} bytes")
        lines.append("")

        if self.phases:
            max_name = max(len(p.name) for p in self.phases)
            for phase in self.phases:
                pct = (phase.duration_ms / self.total_ms * 100) if self.total_ms > 0 else 0
                bar = "#" * int(pct / 2)
                lines.append(f"  {phase.name:<{max_name}} {phase.duration_ms:>7.2f}ms ({pct:>5.1f}%) {bar}")

        return "\n".join(lines)


class Profiler:
    """Tracks compilation phase timings."""

    def __init__(self):
        self._phases: List[PhaseMetrics] = []
        self._start_time: float = 0
        self._phase_start: float = 0
        self._current_phase: str = ""

    def start(self):
        """Start profiling a compilation."""
        self._start_time = time.perf_counter()
        self._phases = []

    def begin_phase(self, name: str):
        """Begin timing a new phase."""
        if self._current_phase:
            self.end_phase()
        self._current_phase = name
        self._phase_start = time.perf_counter()

    def end_phase(self, items: int = 0):
        """End the current phase and record metrics."""
        if not self._current_phase:
            return
        elapsed = (time.perf_counter() - self._phase_start) * 1000
        self._phases.append(PhaseMetrics(
            name=self._current_phase,
            duration_ms=elapsed,
            items_processed=items,
        ))
        self._current_phase = ""

    def finish(self, input_size: int = 0, output_size: int = 0) -> ProfileReport:
        """Finish profiling and return the report."""
        if self._current_phase:
            self.end_phase()

        total = (time.perf_counter() - self._start_time) * 1000
        return ProfileReport(
            phases=self._phases,
            total_ms=total,
            input_size=input_size,
            output_size=output_size,
        )
