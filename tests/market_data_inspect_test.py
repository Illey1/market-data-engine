"""Run with: python3 tests/market_data_inspect_test.py ./build/market_data_inspect"""

from pathlib import Path
import subprocess
import sys
import unittest


if len(sys.argv) < 2:
    raise SystemExit("Usage: market_data_inspect_test.py <market_data_inspect>")
EXECUTABLE = str(Path(sys.argv.pop(1)).resolve())
PAYLOAD = b"S" + bytes(10) + b"O"
FRAME = len(PAYLOAD).to_bytes(2, "big") + PAYLOAD


class MarketDataInspectTest(unittest.TestCase):
    def inspect(self, data, *options):
        return subprocess.run(
            [EXECUTABLE, "-", *options], input=data,
            capture_output=True, timeout=5, check=False,
        )

    def assert_summary(self, output, end_reason):
        fields = dict(line.split(": ", 1) for line in output.decode().splitlines())
        self.assertEqual(fields, {
            "messages": "1", "system_events": "1", "stock_directory": "0",
            "add_orders": "0", "executions": "0", "cancels": "0",
            "deletes": "0", "replaces": "0", "other": "0",
            "active_orders": "0", "end_reason": end_reason,
        })

    def test_clean_eof(self):
        result = self.inspect(FRAME)
        self.assertEqual(result.returncode, 0, result.stderr.decode())
        self.assert_summary(result.stdout, "eof")
        warning = result.stderr.decode().lower()
        self.assertIn("warning", warning)
        self.assertIn("zero-length", warning)
        self.assertIn("terminator", warning)
        self.assertEqual(len(warning.splitlines()), 1)

    def test_session_terminator(self):
        result = self.inspect(FRAME + b"\x00\x00")
        self.assertEqual(result.returncode, 0, result.stderr.decode())
        self.assert_summary(result.stdout, "session_terminator")
        self.assertEqual(result.stderr, b"")

    def test_message_limit_precedes_malformed_tail(self):
        result = self.inspect(FRAME + b"\x00", "--max-messages", "1")
        self.assertEqual(result.returncode, 0, result.stderr.decode())
        self.assert_summary(result.stdout, "message_limit")
        self.assertEqual(result.stderr, b"")

    def test_partial_input_is_rejected(self):
        for tail in (b"\x00", b"\x00\x0cS"):
            with self.subTest(tail=tail):
                result = self.inspect(FRAME + tail)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(b"Error:", result.stderr)
                self.assertEqual(result.stdout, b"")

    def test_message_limit_does_not_wait_for_eof(self):
        with subprocess.Popen(
            [EXECUTABLE, "-", "--max-messages", "1"],
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        ) as process:
            try:
                process.stdin.write(FRAME)
                process.stdin.flush()
                process.wait(timeout=5)
                self.assertEqual(process.returncode, 0)
                self.assert_summary(process.stdout.read(), "message_limit")
                self.assertEqual(process.stderr.read(), b"")
            finally:
                if process.poll() is None:
                    process.kill()


if __name__ == "__main__":
    unittest.main()
