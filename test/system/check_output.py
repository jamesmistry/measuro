# Measuro: a fast, easy-to-use header library for creating program metrics in
# modern C++
# 
# Copyright (c) 2017 James Mistry
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Judges the output of the Measuro system test. Compatible with Python 2.7+ and
# Python 3.2+

from __future__ import print_function
import os
import sys
import subprocess
import json

NUM_CREATED_METRICS = 1000
NUM_THREADS = 2

def fail_test(reason):
    print("FAILED: {0}".format(reason), file=sys.stderr)
    sys.exit(1)

def fail_test_if(cond, reason):
    if cond:
        fail_test(reason)

def validate_parsed_metrics(parsed):
    expected_key_count = 0
    # Check the dynamically generated metrics
    for t in range(NUM_THREADS):
        for m in range(NUM_CREATED_METRICS):
            expected_keys = ["TestMetric{0}_{1}_uint".format(m, t), "TestMetric{0}_{1}_int".format(m, t), "TestMetric{0}_{1}_float".format(m, t), "TestMetric{0}_{1}_sum_int".format(m ,t), "TestMetric{0}_{1}_rate_sum_int".format(m, t)]
            for key in expected_keys:
                fail_test_if(key not in parsed, "Metric '{0}' expected but not found".format(key))
                expected_key_count += 1

    # Check the basic metrics
    expected_keys = ["TestNum1", "TestNum2", "TestNum3", "TestNumRate", "TestStr", "TestBool", "TestFloat", "TestSum"]
    for key in expected_keys:
        fail_test_if(key not in parsed, "Metric '{0}' expected but not found".format(key))
        expected_key_count += 1

    fail_test_if(len(parsed) != expected_key_count, "Expected {0} keys, found {1}".format(expected_key_count, len(parsed)))

def main():
    min_py_ver_2 = (2, 7)
    min_py_ver_3 = (3, 2)

    py_ver = sys.version_info

    if py_ver[0] == 2:
        if py_ver[1] > min_py_ver_2[1]:
            fail_test("Require Python v2.7+ or Python v3.2+, but found {0}.{1}".format(py_ver[0], py_ver[1]))

    if py_ver[1] == 3:
        if py_ver[1] > min_py_ver_3[1]:
            fail_test("Require Python v2.7+ or Python v3.2+, but found {0}.{1}".format(py_ver[0], py_ver[1]))

    if len(sys.argv) != 2:
        sys.exit(1)

    subject = sys.argv[1]

    try:
        results = subprocess.check_output([subject]).decode("UTF-8")
    except subprocess.CalledProcessError as e:
        fail_test("Target process exited with non-zero code")

    results = results.strip().split("\n")
    
    fail_test_if(len(results) < 2, "Expected at least 2 results")

    i = 0
    for result in results:
        try:
            parsed = json.loads(result)

            validate_parsed_metrics(parsed)
        except Exception as e:
            fail_test(str(e))
        finally:
            i += 1

if __name__ == '__main__':
    main()
    print("Test passed :-)")
    sys.exit(0)
