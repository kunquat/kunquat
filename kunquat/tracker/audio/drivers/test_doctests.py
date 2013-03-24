import unittest
import doctest
from kunquat.tracker.audio.drivers import pulseaudio

def load_tests(loader, tests, ignore):
    tests.addTests(doctest.DocTestSuite(pulseaudio))
    return tests
