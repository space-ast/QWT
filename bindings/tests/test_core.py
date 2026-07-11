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


# ---------------------------------------------------------------------------
# Step 1: interval border flags + QwtScaleDiv construction
# ---------------------------------------------------------------------------

def test_qwtinterval_border_flags():
    """BorderFlag enum and BorderFlags bitwise OR are accessible."""
    iv = qwtcore.QwtInterval(0.0, 10.0)
    QwtInterval = qwtcore.QwtInterval
    # setBorderFlags takes a SINGLE BorderFlags (QFlags) argument, not two
    # BorderFlag args as the brief stated. The QFlags composite is built with
    # Python's bitwise OR over the enum values.
    iv.setBorderFlags(QwtInterval.ExcludeMinimum | QwtInterval.ExcludeMaximum)
    assert iv.borderFlags() is not None
    assert int(iv.borderFlags()) == int(QwtInterval.ExcludeMinimum) | int(QwtInterval.ExcludeMaximum)


def test_qwtscalediv_construct():
    """QwtScaleDiv can be built from bounds and tick lists."""
    sd = qwtcore.QwtScaleDiv(0.0, 100.0)
    assert sd.lowerBound() == pytest.approx(0.0)
    assert sd.upperBound() == pytest.approx(100.0)


# ---------------------------------------------------------------------------
# Step 2: QwtScaleMap transform + QwtTransform ownership
# ---------------------------------------------------------------------------

def test_qwtscalemap_transform_linear():
    """QwtScaleMap maps a value through a linear transformation."""
    sm = qwtcore.QwtScaleMap()
    sm.setScaleInterval(0.0, 10.0)
    sm.setPaintInterval(0.0, 100.0)
    # 5.0 in [0,10] -> 50.0 in [0,100]
    assert sm.transform(5.0) == pytest.approx(50.0)
    assert sm.invTransform(50.0) == pytest.approx(5.0)


def test_qwtscalemap_takes_ownership_of_transform():
    """Setting a QwtTransform transfers ownership to C++ (no double-free).

    This validates the define-ownership rule in the typesystem.
    """
    sm = qwtcore.QwtScaleMap()
    t = qwtcore.QwtLogTransform()
    sm.setTransformation(t)
    # If ownership transferred correctly, sm now owns t. Re-assigning should
    # not crash (the old transform is deleted by C++, and 't' is no longer
    # referenced from Python). This test mainly asserts no crash on teardown.
    sm2 = qwtcore.QwtScaleMap()
    sm2.setTransformation(qwtcore.QwtPowerTransform(2.0))
    # Force garbage collection to trigger any double-free.
    import gc
    gc.collect()
    del sm, sm2, t
    gc.collect()  # second collect exercises destructor paths


# ---------------------------------------------------------------------------
# Step 3: QwtColorMap tests (the v7.3.1 rgb signature)
# ---------------------------------------------------------------------------

def test_qwtlinearcolormap_rgb_signature():
    """QwtLinearColorMap.rgb(vMin, vMax, value) uses the v7.3.1 signature.

    Note: the OLD signature rgb(const QwtInterval&, double) was REMOVED.
    The current signature is rgb(double vMin, double vMax, double value).
    """
    from PySide6.QtGui import qRgb
    cm = qwtcore.QwtLinearColorMap()
    cm.setColorInterval("#000000", "#FFFFFF")
    # At value == vMin, should be black (0x000000).
    assert cm.rgb(0.0, 10.0, 0.0) == qRgb(0, 0, 0)
    # At value == vMax, should be white (0xFFFFFF).
    assert cm.rgb(0.0, 10.0, 10.0) == qRgb(255, 255, 255)


def test_qwtlinearcolormap_add_color_stop():
    """Adding a color stop and querying it back works."""
    from PySide6.QtGui import QColor
    cm = qwtcore.QwtLinearColorMap()
    cm.setColorInterval("#000000", "#FFFFFF")
    cm.addColorStop(0.5, QColor("#888888"))
    # stopPos() returns a plain Python list of float positions; the brief used
    # a list comprehension which works directly.
    stops = cm.stopPos()
    assert 0.5 in [pytest.approx(s) for s in stops]


# ---------------------------------------------------------------------------
# Step 4: QwtPointSeriesData + boundingRect
# ---------------------------------------------------------------------------

def test_qwtpointseriesdata_samples_and_bounding_rect():
    """QwtPointSeriesData stores samples and computes bounding rect."""
    from PySide6.QtCore import QPointF

    # The constructor takes a QVector<QPointF> exposed as a Python sequence of
    # QPointF. (The brief used QList, but PySide6 6.7.3 exposes the QVector
    # conversion as a plain list / sequence — no QList import needed.)
    points = [QPointF(0.0, 0.0), QPointF(10.0, 20.0), QPointF(5.0, -5.0)]

    data = qwtcore.QwtPointSeriesData(points)
    assert data.size() == 3
    s0 = data.sample(0)
    assert s0.x() == pytest.approx(0.0)
    assert s0.y() == pytest.approx(0.0)

    br = data.boundingRect()
    assert br.left() == pytest.approx(0.0)
    assert br.width() == pytest.approx(10.0)
    assert br.top() == pytest.approx(-5.0)
    assert br.height() == pytest.approx(25.0)


# ---------------------------------------------------------------------------
# Step 5: the critical Python-subclassing test (QWT_PYTHON_WRAPPER payoff)
# ---------------------------------------------------------------------------

def test_subclass_qwtseriesdata_virtual_override():
    """A Python subclass of QwtSeriesData can override size/sample/boundingRect
    and Python-side calls reach the Python implementation.

    This is the core validation of the QWT_PYTHON_WRAPPER hook: without it,
    the pure-virtual methods would make the template base QwtSeriesData<T>
    abstract, so shiboken could not instantiate the concrete subclasses.
    Because QWT_PYTHON_WRAPPER is now forwarded to shiboken's clang parser
    (see Shiboken6ToolsConfig.cmake), size()/sample()/boundingRect() become
    non-pure-virtual default impls and the concrete subclasses instantiate.

    The template-inherited size()/sample() are additionally re-exposed on
    QwtPointSeriesData via <add-function> in the typesystem (the template base
    QwtArraySeriesData<QPointF> is unresolvable by shiboken). A Python subclass
    that overrides size()/sample() shadows the bound C++ method, so a Python
    call dispatches to the override.
    """
    from PySide6.QtCore import QPointF, QRectF

    class MySeries(qwtcore.QwtPointSeriesData):
        def __init__(self):
            super().__init__()
            self._called = {"size": 0, "sample": 0}

        def size(self):
            self._called["size"] += 1
            return 2

        def sample(self, i):
            self._called["sample"] += 1
            if i == 0:
                return QPointF(0.0, 0.0)
            return QPointF(1.0, 1.0)

        def boundingRect(self):
            return QRectF(0.0, 0.0, 1.0, 1.0)

    s = MySeries()
    # Calling size() from Python dispatches to the Python override.
    assert s.size() == 2
    assert s._called["size"] >= 1
    # Calling sample() from Python dispatches to the Python override.
    p = s.sample(0)
    assert p.x() == pytest.approx(0.0)
    assert s._called["sample"] >= 1
    # boundingRect override also dispatches.
    assert s.boundingRect() == QRectF(0.0, 0.0, 1.0, 1.0)


# ---------------------------------------------------------------------------
# Step 6: QwtScaleEngine functional test
# ---------------------------------------------------------------------------

def test_qwtlinearscaleengine_divide_scale():
    """QwtLinearScaleEngine divides an interval into a sane scale division."""
    eng = qwtcore.QwtLinearScaleEngine()
    sd = eng.divideScale(0.0, 100.0, 10, 5)
    # The engine may expand the interval slightly; just assert it covers
    # the requested range. (pytest.approx only supports ==, so use plain
    # numeric comparisons with an explicit tolerance.)
    assert sd.lowerBound() <= 0.0 + 0.01
    assert sd.upperBound() >= 100.0 - 0.01
    # Major ticks should exist and be within range.
    ticks = sd.ticks(qwtcore.QwtScaleDiv.MajorTick)
    assert len(ticks) > 0

