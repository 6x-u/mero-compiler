"""
MERO Compiler - Test Runner
Tests Python source files by compiling and verifying output.
Developer: MERO:TG@QP4RM
"""
import time
from pathlib import Path
from dataclasses import dataclass, field
from typing import List, Optional

from .compiler import MeroCompiler, CompileResult
from .config import CompilerConfig
from .logger import Logger, LogLevel


@dataclass
class TestCase:
    name: str
    source: str
    expected_output: Optional[str] = None
    should_fail: bool = False


@dataclass
class TestResult:
    name: str
    passed: bool
    duration_ms: float
    error: Optional[str] = None


@dataclass
class TestSuiteResult:
    total: int = 0
    passed: int = 0
    failed: int = 0
    duration_ms: float = 0.0
    results: List[TestResult] = field(default_factory=list)


class TestRunner:
    """Runs compilation tests and reports results."""

    def __init__(self, config: CompilerConfig = None, logger: Logger = None):
        self.config = config or CompilerConfig()
        self.logger = logger or Logger(level=LogLevel.INFO)
        self.compiler = MeroCompiler(self.config)

    def run_directory(self, directory: str) -> TestSuiteResult:
        """Run all test files in a directory."""
        suite = TestSuiteResult()
        test_dir = Path(directory)

        if not test_dir.exists():
            self.logger.error(f"Test directory not found: {directory}")
            return suite

        test_files = sorted(test_dir.rglob("test_*.py"))
        suite.total = len(test_files)

        start = time.perf_counter()
        for file in test_files:
            result = self._run_test_file(file)
            suite.results.append(result)
            if result.passed:
                suite.passed += 1
            else:
                suite.failed += 1

        suite.duration_ms = (time.perf_counter() - start) * 1000
        self._report(suite)
        return suite

    def run_test(self, test: TestCase) -> TestResult:
        """Run a single test case."""
        start = time.perf_counter()
        result = self.compiler.compile(test.source, test.name)
        duration = (time.perf_counter() - start) * 1000

        if test.should_fail:
            passed = not result.success
            error = "Expected failure but compilation succeeded" if result.success else None
        else:
            passed = result.success
            error = "; ".join(result.errors) if not result.success else None

            if passed and test.expected_output:
                if result.output.strip() != test.expected_output.strip():
                    passed = False
                    error = "Output mismatch"

        return TestResult(name=test.name, passed=passed, duration_ms=duration, error=error)

    def _run_test_file(self, file: Path) -> TestResult:
        """Compile a test file and check for success."""
        start = time.perf_counter()
        result = self.compiler.compile_file(str(file))
        duration = (time.perf_counter() - start) * 1000

        return TestResult(
            name=str(file.name),
            passed=result.success,
            duration_ms=duration,
            error="; ".join(result.errors) if not result.success else None,
        )

    def _report(self, suite: TestSuiteResult):
        """Print test results."""
        for result in suite.results:
            status = "PASS" if result.passed else "FAIL"
            self.logger.info(f"  [{status}] {result.name} ({result.duration_ms:.1f}ms)")
            if result.error:
                self.logger.error(f"         {result.error}")

        self.logger.info(
            f"\n{suite.passed}/{suite.total} passed in {suite.duration_ms:.0f}ms"
        )
