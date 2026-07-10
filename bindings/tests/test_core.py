"""Smoke tests for the qwtcore PySide6 binding (POC: QwtInterval only)."""
import pytest

# Will fail at collection time if qwtcore cannot be imported — that's the POC gate.
import qwtcore


def test_qwtinterval_construct_and_accessors():
    """QwtInterval can be constructed and its accessors return the bounds."""
    iv = qwtcore.QwtInterval(0.0, 10.0)
    assert iv.minValue() == pytest.approx(0.0)
    assert iv.maxValue() == pytest.approx(10.0)


def test_qwtinterval_set_range():
    """setInterval mutates the interval bounds."""
    # NOTE: QwtInterval's actual API is setInterval(min, max, BorderFlags=IncludeBorders);
    # the brief referenced a non-existent setRange(). Using setInterval here.
    iv = qwtcore.QwtInterval()
    iv.setInterval(-5.0, 5.0)
    assert iv.minValue() == pytest.approx(-5.0)
    assert iv.maxValue() == pytest.approx(5.0)
