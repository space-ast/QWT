/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *
 * Modified by ChenZongYan in 2024 <czy.t@163.com>
 *   Summary of major modifications (see ChangeLog.md for full history):
 *   1. CMake build system & C++11 throughout.
 *   2. Core panner/ zoomer refactored:
 *        - QwtPanner → QwtCachePanner (pixmap-cache version)
 *        - New real-time QwtPlotPanner derived from QwtPicker.
 *   3. Zoomer supports multi-axis.
 *   4. Parasite-plot framework:
 *        - QwtFigure, QwtPlotParasiteLayout, QwtPlotTransparentCanvas,
 *        - QwtPlotScaleEventDispatcher, built-in pan/zoom on axis.
 *   5. New picker: QwtPlotSeriesDataPicker (works with date axis).
 *   6. Raster & color-map extensions:
 *        - QwtGridRasterData (2-D table + interpolation)
 *        - QwtLinearColorMap::stopColors(), stopPos() API rename.
 *   7. Bar-chart: expose pen/brush control.
 *   8. Amalgamated build: single QwtPlot.h / QwtPlot.cpp pair in src-amalgamate.
 *****************************************************************************/

#include "qwt_graphic.h"
#include "qwt_painter_command.h"
#include "qwt_math.h"

#include <qvector.h>
#include <qpainter.h>
#include <qpaintengine.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpainterpath.h>

// Define QWT_GRAPHIC_DEBUG at compile time (e.g. -DQWT_GRAPHIC_DEBUG)
// or uncomment the next line to enable verbose diagnostics for Qt6 icon issues.
// #define QWT_GRAPHIC_DEBUG
#ifdef QWT_GRAPHIC_DEBUG
#  include <qdebug.h>
#endif

#if QT_VERSION >= 0x050000

#include <qguiapplication.h>

static inline qreal qwtDevicePixelRatio()
{
    return qGuiApp ? qGuiApp->devicePixelRatio() : 1.0;
}

#endif

static bool qwtHasScalablePen( const QPainter* painter )
{
    const QPen pen = painter->pen();

    bool scalablePen = false;

    if ( pen.style() != Qt::NoPen && pen.brush().style() != Qt::NoBrush )
    {
        scalablePen = !pen.isCosmetic();
#if QT_VERSION < 0x050000
        if ( !scalablePen && pen.widthF() == 0.0 )
        {
            const QPainter::RenderHints hints = painter->renderHints();
            if ( hints.testFlag( QPainter::NonCosmeticDefaultPen ) )
                scalablePen = true;
        }
#endif
    }

    return scalablePen;
}

static QRectF qwtStrokedPathRect(
    const QPainter* painter, const QPainterPath& path )
{
    QPainterPathStroker stroker;
    stroker.setWidth( painter->pen().widthF() );
    stroker.setCapStyle( painter->pen().capStyle() );
    stroker.setJoinStyle( painter->pen().joinStyle() );
    stroker.setMiterLimit( painter->pen().miterLimit() );

    QRectF rect;
    if ( qwtHasScalablePen( painter ) )
    {
        QPainterPath stroke = stroker.createStroke( path );
        rect = painter->transform().map( stroke ).boundingRect();
    }
    else
    {
        QPainterPath mappedPath = painter->transform().map( path );
        mappedPath = stroker.createStroke( mappedPath );

        rect = mappedPath.boundingRect();
    }

    return rect;
}

static inline void qwtExecCommand(
    QPainter* painter, const QwtPainterCommand& cmd,
    QwtGraphic::RenderHints renderHints,
    const QTransform& transform,
    const QTransform* initialTransform )
{
    switch( cmd.type() )
    {
        case QwtPainterCommand::Path:
        {
            bool doMap = false;

            if ( painter->transform().isScaling() )
            {
                bool isCosmetic = painter->pen().isCosmetic();
#if QT_VERSION < 0x050000
                if ( isCosmetic && painter->pen().widthF() == 0.0 )
                {
                    QPainter::RenderHints hints = painter->renderHints();
                    if ( hints.testFlag( QPainter::NonCosmeticDefaultPen ) )
                        isCosmetic = false;
                }
#endif

                if ( isCosmetic )
                {
                    // OpenGL2 seems to be buggy for cosmetic pens.
                    // It interpolates curves in too rough steps then

                    doMap = painter->paintEngine()->type() == QPaintEngine::OpenGL2;
                }
                else
                {
                    doMap = renderHints.testFlag( QwtGraphic::RenderPensUnscaled );
                }
            }

            if ( doMap )
            {
                const QTransform tr = painter->transform();

                painter->resetTransform();

                QPainterPath path = tr.map( *cmd.path() );
                if ( initialTransform )
                {
                    painter->setTransform( *initialTransform );
                    path = initialTransform->inverted().map( path );
                }

                painter->drawPath( path );

                painter->setTransform( tr );
            }
            else
            {
                painter->drawPath( *cmd.path() );
            }
            break;
        }
        case QwtPainterCommand::Pixmap:
        {
            const QwtPainterCommand::PixmapData* data = cmd.pixmapData();
            painter->drawPixmap( data->rect, data->pixmap, data->subRect );
            break;
        }
        case QwtPainterCommand::Image:
        {
            const QwtPainterCommand::ImageData* data = cmd.imageData();
            painter->drawImage( data->rect, data->image,
                data->subRect, data->flags );
            break;
        }
        case QwtPainterCommand::State:
        {
            const QwtPainterCommand::StateData* data = cmd.stateData();

            if ( data->flags & QPaintEngine::DirtyPen )
                painter->setPen( data->pen );

            if ( data->flags & QPaintEngine::DirtyBrush )
                painter->setBrush( data->brush );

            if ( data->flags & QPaintEngine::DirtyBrushOrigin )
                painter->setBrushOrigin( data->brushOrigin );

            if ( data->flags & QPaintEngine::DirtyFont )
                painter->setFont( data->font );

            if ( data->flags & QPaintEngine::DirtyBackground )
            {
                painter->setBackgroundMode( data->backgroundMode );
                painter->setBackground( data->backgroundBrush );
            }

            if ( data->flags & QPaintEngine::DirtyTransform )
            {
                painter->setTransform( data->transform * transform );
            }

            if ( data->flags & QPaintEngine::DirtyClipEnabled )
                painter->setClipping( data->isClipEnabled );

            if ( data->flags & QPaintEngine::DirtyClipRegion )
            {
                painter->setClipRegion( data->clipRegion,
                    data->clipOperation );
            }

            if ( data->flags & QPaintEngine::DirtyClipPath )
            {
                painter->setClipPath( data->clipPath, data->clipOperation );
            }

            if ( data->flags & QPaintEngine::DirtyHints )
            {
                for ( int i = 0; i < 8; i++ )
                {
                    const QPainter::RenderHint hint = static_cast< QPainter::RenderHint >( 1 << i );
                    painter->setRenderHint( hint, data->renderHints.testFlag( hint ) );
                }
            }

            if ( data->flags & QPaintEngine::DirtyCompositionMode )
                painter->setCompositionMode( data->compositionMode );

            if ( data->flags & QPaintEngine::DirtyOpacity )
                painter->setOpacity( data->opacity );

            break;
        }
        default:
            break;
    }
}

class QwtGraphic::PathInfo
{
  public:
    PathInfo()
        : m_scalablePen( false )
    {
        // QVector needs a default constructor
    }

    PathInfo( const QRectF& pointRect,
            const QRectF& boundingRect, bool scalablePen )
        : m_pointRect( pointRect )
        , m_boundingRect( boundingRect )
        , m_scalablePen( scalablePen )
    {
    }

    inline QRectF scaledBoundingRect( qreal sx, qreal sy, bool scalePens ) const
    {
        if ( sx == 1.0 && sy == 1.0 )
            return m_boundingRect;

        QTransform transform;
        transform.scale( sx, sy );

        QRectF rect;
        if ( scalePens && m_scalablePen )
        {
            rect = transform.mapRect( m_boundingRect );
        }
        else
        {
            rect = transform.mapRect( m_pointRect );

            const qreal l = qAbs( m_pointRect.left() - m_boundingRect.left() );
            const qreal r = qAbs( m_pointRect.right() - m_boundingRect.right() );
            const qreal t = qAbs( m_pointRect.top() - m_boundingRect.top() );
            const qreal b = qAbs( m_pointRect.bottom() - m_boundingRect.bottom() );

            rect.adjust( -l, -t, r, b );
        }

        return rect;
    }

    inline double scaleFactorX( const QRectF& pathRect,
        const QRectF& targetRect, bool scalePens ) const
    {
        if ( pathRect.width() <= 0.0 )
            return 0.0;

        const QPointF p0 = m_pointRect.center();

        const qreal l = qAbs( pathRect.left() - p0.x() );
        const qreal r = qAbs( pathRect.right() - p0.x() );

        const double w = 2.0 * qwtMinF( l, r )
            * targetRect.width() / pathRect.width();

        double sx;
        if ( scalePens && m_scalablePen )
        {
            sx = w / m_boundingRect.width();
        }
        else
        {
            const qreal pw = qwtMaxF(
                qAbs( m_boundingRect.left() - m_pointRect.left() ),
                qAbs( m_boundingRect.right() - m_pointRect.right() ) );

            sx = ( w - 2 * pw ) / m_pointRect.width();
        }

        return sx;
    }

    inline double scaleFactorY( const QRectF& pathRect,
        const QRectF& targetRect, bool scalePens ) const
    {
        if ( pathRect.height() <= 0.0 )
            return 0.0;

        const QPointF p0 = m_pointRect.center();

        const qreal t = qAbs( pathRect.top() - p0.y() );
        const qreal b = qAbs( pathRect.bottom() - p0.y() );

        const qreal h = 2.0 * qwtMinF( t, b )
            * targetRect.height() / pathRect.height();

        double sy;
        if ( scalePens && m_scalablePen )
        {
            sy = h / m_boundingRect.height();
        }
        else
        {
            const qreal pw = qwtMaxF(
                qAbs( m_boundingRect.top() - m_pointRect.top() ),
                qAbs( m_boundingRect.bottom() - m_pointRect.bottom() ) );

            sy = ( h - 2 * pw ) / m_pointRect.height();
        }

        return sy;
    }

  private:
    QRectF m_pointRect;
    QRectF m_boundingRect;
    bool m_scalablePen;
};

class QwtGraphic::PrivateData
{
    QWT_DECLARE_PUBLIC(QwtGraphic)
  public:
    PrivateData( QwtGraphic* p )
        : q_ptr( p )
        , boundingRect( 0.0, 0.0, -1.0, -1.0 )
        , pointRect( 0.0, 0.0, -1.0, -1.0 )
    {
    }

    QSizeF defaultSize;
    QVector< QwtPainterCommand > commands;
    QVector< QwtGraphic::PathInfo > pathInfos;

    QRectF boundingRect;
    QRectF pointRect;

    QwtGraphic::CommandTypes commandTypes;
    QwtGraphic::RenderHints renderHints;
};

/**
 * @brief Constructor
 *
 * @details Initializes a null graphic.
 *
 */
QwtGraphic::QwtGraphic()
    : QWT_PIMPL_CONSTRUCT
{
    setMode( QwtNullPaintDevice::PathMode );
}

/**
 * @brief Copy constructor
 *
 * @param[in] other Source graphic to copy from
 *
 */
QwtGraphic::QwtGraphic( const QwtGraphic& other )
    : QWT_PIMPL_CONSTRUCT
{
    setMode( other.mode() );
    *m_data = *other.m_data;
}

/**
 * @brief Destructor
 *
 * @details Deletes private data.
 *
 */
QwtGraphic::~QwtGraphic()
{
}

/**
 * @brief Assignment operator
 *
 * @param[in] other Source graphic to assign from
 * @return Reference to this object
 *
 */
QwtGraphic& QwtGraphic::operator=( const QwtGraphic& other )
{
    QWT_D(d);
    setMode( other.mode() );
    *d = *other.m_data;

    return *this;
}

/**
 * @brief Clear all stored commands
 *
 * @details Resets the graphic to its initial null state.
 *
 */
void QwtGraphic::reset()
{
    QWT_D(d);
    d->commands.clear();
    d->pathInfos.clear();

    d->commandTypes = CommandTypes();

    d->boundingRect = QRectF( 0.0, 0.0, -1.0, -1.0 );
    d->pointRect = QRectF( 0.0, 0.0, -1.0, -1.0 );
    d->defaultSize = QSizeF();
}

/**
 * @brief Check if the graphic is null
 *
 * @return True when no painter commands have been stored
 *
 */
bool QwtGraphic::isNull() const
{
    QWT_DC(d);
    return d->commands.isEmpty();
}

/**
 * @brief Check if the graphic is empty
 *
 * @return True when the bounding rectangle is empty
 *
 */
bool QwtGraphic::isEmpty() const
{
    QWT_DC(d);
    return d->boundingRect.isEmpty();
}

/**
 * @brief Get the types of painter commands being used
 *
 * @return Types of painter commands
 *
 */
QwtGraphic::CommandTypes QwtGraphic::commandTypes() const
{
    QWT_DC(d);
    return d->commandTypes;
}

/**
 * @brief Toggle a render hint
 *
 * @param[in] hint Render hint to toggle
 * @param[in] on true to enable, false to disable
 *
 */
void QwtGraphic::setRenderHint( RenderHint hint, bool on )
{
    QWT_D(d);
    if ( on )
        d->renderHints |= hint;
    else
        d->renderHints &= ~hint;
}

/**
 * @brief Test a render hint
 *
 * @param[in] hint Render hint to test
 * @return true if the hint is enabled, false otherwise
 *
 */
bool QwtGraphic::testRenderHint( RenderHint hint ) const
{
    QWT_DC(d);
    return d->renderHints.testFlag( hint );
}

/**
 * @brief Get the render hints
 *
 * @return Render hints
 *
 */
QwtGraphic::RenderHints QwtGraphic::renderHints() const
{
    QWT_DC(d);
    return d->renderHints;
}

/**
 * @brief Get the bounding rectangle
 *
 * @details The bounding rectangle is the controlPointRect()
 * extended by the areas needed for rendering the outlines
 * with unscaled pens.
 *
 * @return Bounding rectangle of the graphic
 *
 */
QRectF QwtGraphic::boundingRect() const
{
    QWT_DC(d);
    if ( d->boundingRect.width() < 0 )
        return QRectF();

    return d->boundingRect;
}

/**
 * @brief Get the control point rectangle
 *
 * @details The control point rectangle is the bounding rectangle
 * of all control points of the paths and the target
 * rectangles of the images/pixmaps.
 *
 * @return Control point rectangle
 *
 */
QRectF QwtGraphic::controlPointRect() const
{
    QWT_DC(d);
    if ( d->pointRect.width() < 0 )
        return QRectF();

    return d->pointRect;
}

/**
 * @brief Calculate the target rectangle for scaling the graphic
 *
 * @param[in] sx Horizontal scaling factor
 * @param[in] sy Vertical scaling factor
 *
 * @note In case of paths that are painted with a cosmetic pen
 *       ( see QPen::isCosmetic() ) the target rectangle is different to
 *       multiplying the bounding rectangle.
 *
 * @return Scaled bounding rectangle
 *
 */
QRectF QwtGraphic::scaledBoundingRect( qreal sx, qreal sy ) const
{
    QWT_DC(d);
    if ( sx == 1.0 && sy == 1.0 )
        return d->boundingRect;

    const bool scalePens = !( d->renderHints & RenderPensUnscaled );

    QTransform transform;
    transform.scale( sx, sy );

    QRectF rect = transform.mapRect( d->pointRect );

    for ( int i = 0; i < d->pathInfos.size(); i++ )
        rect |= d->pathInfos[i].scaledBoundingRect( sx, sy, scalePens );

    return rect;
}

//! @return Ceiled defaultSize()
QSize QwtGraphic::sizeMetrics() const
{
    const QSizeF sz = defaultSize();
    return QSize( qwtCeil( sz.width() ), qwtCeil( sz.height() ) );
}

/**
 * @brief Set a default size
 *
 * @details The default size is used in all methods rendering the graphic,
 * where no size is explicitly specified. Assigning an empty size
 * means, that the default size will be calculated from the bounding
 * rectangle.
 *
 * The default setting is an empty size.
 *
 * @param[in] size Default size
 *
 */
void QwtGraphic::setDefaultSize( const QSizeF& size )
{
    QWT_D(d);
    const double w = qwtMaxF( 0.0, size.width() );
    const double h = qwtMaxF( 0.0, size.height() );

    d->defaultSize = QSizeF( w, h );
}

/**
 * @brief Get the default size
 *
 * @details When a non empty size has been assigned by setDefaultSize() this
 * size will be returned. Otherwise the default size is the size
 * of the bounding rectangle.
 *
 * The default size is used in all methods rendering the graphic,
 * where no size is explicitly specified.
 *
 * @return Default size
 *
 */
QSizeF QwtGraphic::defaultSize() const
{
    QWT_DC(d);
    if ( !d->defaultSize.isEmpty() )
        return d->defaultSize;

    return boundingRect().size();
}

/**
 * @brief Find the height for a given width
 *
 * @details The height is calculated using the aspect ratio of defaultSize().
 *
 * @param[in] width Width to calculate height for
 * @return Calculated height
 *
 */
qreal QwtGraphic::heightForWidth( qreal width ) const
{
    const QSizeF sz = defaultSize();
    if ( sz.isEmpty() )
        return 0.0;

    return sz.height() * width / sz.width();
}

/**
 * @brief Find the width for a given height
 *
 * @details The width is calculated using the aspect ratio of defaultSize().
 *
 * @param[in] height Height to calculate width for
 * @return Calculated width
 *
 */
qreal QwtGraphic::widthForHeight( qreal height ) const
{
    const QSizeF sz = defaultSize();
    if ( sz.isEmpty() )
        return 0.0;

    return sz.width() * height / sz.height();
}

/**
 * @brief Replay all recorded painter commands
 *
 * @param[in] painter Qt painter
 *
 */
void QwtGraphic::render( QPainter* painter ) const
{
    renderGraphic( painter, nullptr );
}

void QwtGraphic::renderGraphic( QPainter* painter, QTransform* initialTransform ) const
{
    QWT_DC(d);
    if ( isNull() )
        return;

    const int numCommands = d->commands.size();
    const QwtPainterCommand* commands = d->commands.constData();

    const QTransform transform = painter->transform();

    painter->save();

    for ( int i = 0; i < numCommands; i++ )
    {
        qwtExecCommand( painter, commands[i],
            d->renderHints, transform, initialTransform );
    }

    painter->restore();
}

/**
 * @brief Replay all recorded painter commands scaled to fit into given size
 *
 * @details The graphic is scaled to fit into the rectangle
 * of the given size starting at ( 0, 0 ).
 *
 * @param[in] painter Qt painter
 * @param[in] size Size for the scaled graphic
 * @param[in] aspectRatioMode Mode how to scale - See Qt::AspectRatioMode
 *
 */
void QwtGraphic::render( QPainter* painter, const QSizeF& size,
    Qt::AspectRatioMode aspectRatioMode ) const
{
    const QRectF r( 0.0, 0.0, size.width(), size.height() );
    render( painter, r, aspectRatioMode );
}

/**
 * @brief Replay all recorded painter commands scaled to fit into given rectangle
 *
 * @param[in] painter Qt painter
 * @param[in] rect Rectangle for the scaled graphic
 * @param[in] aspectRatioMode Mode how to scale - See Qt::AspectRatioMode
 *
 */
void QwtGraphic::render( QPainter* painter, const QRectF& rect,
    Qt::AspectRatioMode aspectRatioMode ) const
{
    QWT_DC(d);
    if ( isEmpty() || rect.isEmpty() )
    {
#ifdef QWT_GRAPHIC_DEBUG
        qDebug() << "[QwtGraphic::render] SKIPPED"
                 << "isEmpty()=" << isEmpty()
                 << "rect.isEmpty()=" << rect.isEmpty()
                 << "boundingRect()=" << boundingRect()
                 << "commands count=" << d->commands.size()
                 << "defaultSize()=" << defaultSize();
#endif
        return;
    }

    double sx = 1.0;
    double sy = 1.0;

    if ( d->pointRect.width() > 0.0 )
        sx = rect.width() / d->pointRect.width();

    if ( d->pointRect.height() > 0.0 )
        sy = rect.height() / d->pointRect.height();

    const bool scalePens = !d->renderHints.testFlag( RenderPensUnscaled );

    for ( int i = 0; i < d->pathInfos.size(); i++ )
    {
        const PathInfo& info = d->pathInfos[i];

        const double ssx = info.scaleFactorX(
            d->pointRect, rect, scalePens );

        if ( ssx > 0.0 )
            sx = qwtMinF( sx, ssx );

        const double ssy = info.scaleFactorY(
            d->pointRect, rect, scalePens );

        if ( ssy > 0.0 )
            sy = qwtMinF( sy, ssy );
    }

    if ( aspectRatioMode == Qt::KeepAspectRatio )
    {
        const qreal s = qwtMinF( sx, sy );
        sx = s;
        sy = s;
    }
    else if ( aspectRatioMode == Qt::KeepAspectRatioByExpanding )
    {
        const qreal s = qwtMaxF( sx, sy );
        sx = s;
        sy = s;
    }

    QTransform tr;
    tr.translate( rect.center().x() - 0.5 * sx * d->pointRect.width(),
        rect.center().y() - 0.5 * sy * d->pointRect.height() );
    tr.scale( sx, sy );
    tr.translate( -d->pointRect.x(), -d->pointRect.y() );

    const QTransform transform = painter->transform();

    painter->setTransform( tr, true );

    if ( !scalePens && transform.isScaling() )
    {
        // we don't want to scale pens according to sx/sy,
        // but we want to apply the scaling from the
        // painter transformation later

        QTransform initialTransform;
        initialTransform.scale( transform.m11(), transform.m22() );

        renderGraphic( painter, &initialTransform );
    }
    else
    {
        renderGraphic( painter, nullptr );
    }

    painter->setTransform( transform );
}

/**
 * @brief Replay all recorded painter commands aligned to a position
 *
 * @details The graphic is scaled to the defaultSize() and aligned
 * to a position.
 *
 * @param[in] painter Qt painter
 * @param[in] pos Reference point, where to render
 * @param[in] alignment Flags how to align the target rectangle to pos
 *
 */
void QwtGraphic::render( QPainter* painter,
    const QPointF& pos, Qt::Alignment alignment ) const
{
    QRectF r( pos, defaultSize() );

    if ( alignment & Qt::AlignLeft )
    {
        r.moveLeft( pos.x() );
    }
    else if ( alignment & Qt::AlignHCenter )
    {
        r.moveCenter( QPointF( pos.x(), r.center().y() ) );
    }
    else if ( alignment & Qt::AlignRight )
    {
        r.moveRight( pos.x() );
    }

    if ( alignment & Qt::AlignTop )
    {
        r.moveTop( pos.y() );
    }
    else if ( alignment & Qt::AlignVCenter )
    {
        r.moveCenter( QPointF( r.center().x(), pos.y() ) );
    }
    else if ( alignment & Qt::AlignBottom )
    {
        r.moveBottom( pos.y() );
    }

    render( painter, r );
}

/**
 * @brief Convert the graphic to a QPixmap in default size
 *
 * @details All pixels of the pixmap get initialized by Qt::transparent
 * before the graphic is scaled and rendered on it.
 *
 * The size of the pixmap is the default size ( ceiled to integers )
 * of the graphic.
 *
 * @param[in] devicePixelRatio Device pixel ratio for the pixmap.
 *                              If devicePixelRatio <= 0.0 the pixmap
 *                              is initialized with the system default.
 *
 * @return The graphic as pixmap in default size
 *
 */
QPixmap QwtGraphic::toPixmap( qreal devicePixelRatio ) const
{
    if ( isNull() )
        return QPixmap();

    const QSizeF sz = defaultSize();

    const int w = qwtCeil( sz.width() );
    const int h = qwtCeil( sz.height() );

    QPixmap pixmap( w, h );

#if QT_VERSION >= 0x050000
    if ( devicePixelRatio <= 0.0 )
        devicePixelRatio = qwtDevicePixelRatio();

    pixmap.setDevicePixelRatio( devicePixelRatio );
#else
    Q_UNUSED( devicePixelRatio )
#endif

    pixmap.fill( Qt::transparent );

    const QRectF r( 0.0, 0.0, sz.width(), sz.height() );

    QPainter painter( &pixmap );
    render( &painter, r, Qt::KeepAspectRatio );
    painter.end();

    return pixmap;
}

/**
 * @brief Convert the graphic to a QPixmap with specified size
 *
 * @details All pixels of the pixmap get initialized by Qt::transparent
 * before the graphic is scaled and rendered on it.
 *
 * @param[in] size Size of the image
 * @param[in] aspectRatioMode Aspect ratio how to scale the graphic
 * @param[in] devicePixelRatio Device pixel ratio for the pixmap.
 *                              If devicePixelRatio <= 0.0 the pixmap
 *                              is initialized with the system default.
 *
 * @return The graphic as pixmap
 *
 */
QPixmap QwtGraphic::toPixmap( const QSize& size,
    Qt::AspectRatioMode aspectRatioMode, qreal devicePixelRatio ) const
{
    QPixmap pixmap( size );

#if QT_VERSION >= 0x050000
    if ( devicePixelRatio <= 0.0 )
        devicePixelRatio = qwtDevicePixelRatio();

    pixmap.setDevicePixelRatio( devicePixelRatio );
#else
    Q_UNUSED( devicePixelRatio )
#endif
    pixmap.fill( Qt::transparent );

    const QRect r( 0, 0, size.width(), size.height() );

    QPainter painter( &pixmap );
    render( &painter, r, aspectRatioMode );
    painter.end();

    return pixmap;
}

/**
 * @brief Convert the graphic to a QImage with specified size
 *
 * @details All pixels of the image get initialized by 0 ( transparent )
 * before the graphic is scaled and rendered on it.
 *
 * The format of the image is QImage::Format_ARGB32_Premultiplied.
 *
 * @param[in] size Size of the image ( will be multiplied by the devicePixelRatio )
 * @param[in] aspectRatioMode Aspect ratio how to scale the graphic
 * @param[in] devicePixelRatio Device pixel ratio for the image.
 *                              If devicePixelRatio <= 0.0 the pixmap
 *                              is initialized with the system default.
 *
 * @return The graphic as image
 *
 */
QImage QwtGraphic::toImage( const QSize& size,
    Qt::AspectRatioMode aspectRatioMode, qreal devicePixelRatio  ) const
{
#if QT_VERSION >= 0x050000
    if ( devicePixelRatio <= 0.0 )
        devicePixelRatio = qwtDevicePixelRatio();

    QImage image( size* devicePixelRatio, QImage::Format_ARGB32_Premultiplied );
    image.setDevicePixelRatio( devicePixelRatio );
#else
    Q_UNUSED( devicePixelRatio )
    QImage image( size, QImage::Format_ARGB32_Premultiplied );
#endif

    image.fill( 0 );

    const QRect r( 0, 0, size.width(), size.height() );

    QPainter painter( &image );
    render( &painter, r, aspectRatioMode );
    painter.end();

    return image;
}

/**
 * @brief Convert the graphic to a QImage in default size
 *
 * @details All pixels of the image get initialized by 0 ( transparent )
 * before the graphic is scaled and rendered on it.
 *
 * The format of the image is QImage::Format_ARGB32_Premultiplied.
 *
 * The size of the image is the default size ( ceiled to integers )
 * of the graphic multiplied by the devicePixelRatio.
 *
 * @param[in] devicePixelRatio Device pixel ratio for the image.
 *                              If devicePixelRatio <= 0.0 the pixmap
 *                              is initialized with the system default.
 *
 * @return The graphic as image in default size
 *
 */
QImage QwtGraphic::toImage( qreal devicePixelRatio ) const
{
    if ( isNull() )
        return QImage();

    const QSizeF sz = defaultSize();

    int w = qwtCeil( sz.width() );
    int h = qwtCeil( sz.height() );

#if QT_VERSION >= 0x050000
    if ( devicePixelRatio <= 0.0 )
        devicePixelRatio = qwtDevicePixelRatio();

    w *= devicePixelRatio;
    h *= devicePixelRatio;

    QImage image( w, h, QImage::Format_ARGB32 );
    image.setDevicePixelRatio( devicePixelRatio );
#else
    Q_UNUSED( devicePixelRatio )
    QImage image( w, h, QImage::Format_ARGB32 );
#endif

    image.fill( 0 );

    const QRect r( 0, 0, sz.width(), sz.height() );

    QPainter painter( &image );
    render( &painter, r, Qt::KeepAspectRatio );
    painter.end();

    return image;
}

/*!
   Store a path command in the command list

   @param path Painter path
   @sa QPaintEngine::drawPath()
 */
void QwtGraphic::drawPath( const QPainterPath& path )
{
    QWT_D(d);
    const QPainter* painter = paintEngine()->painter();
    if ( painter == nullptr )
        return;

    d->commands += QwtPainterCommand( path );
    d->commandTypes |= QwtGraphic::VectorData;

    if ( !path.isEmpty() )
    {
        const QPainterPath scaledPath = painter->transform().map( path );

        QRectF pointRect = scaledPath.boundingRect();
        QRectF boundingRect = pointRect;

#ifdef QWT_GRAPHIC_DEBUG
        qDebug() << "[QwtGraphic::drawPath]"
                 << "painter->transform()=" << painter->transform()
                 << "path.boundingRect()=" << path.boundingRect()
                 << "scaledPath.boundingRect()=" << pointRect
                 << "devicePixelRatioF=" << paintEngine()->paintDevice()->devicePixelRatioF();
#endif

        if ( painter->pen().style() != Qt::NoPen
            && painter->pen().brush().style() != Qt::NoBrush )
        {
            boundingRect = qwtStrokedPathRect( painter, path );
        }

        updateControlPointRect( pointRect );
        updateBoundingRect( boundingRect );

        d->pathInfos += PathInfo( pointRect,
            boundingRect, qwtHasScalablePen( painter ) );
    }
}

/*!
   @brief Store a pixmap command in the command list

   @param rect target rectangle
   @param pixmap Pixmap to be painted
   @param subRect Reactangle of the pixmap to be painted

   @sa QPaintEngine::drawPixmap()
 */
void QwtGraphic::drawPixmap( const QRectF& rect,
    const QPixmap& pixmap, const QRectF& subRect )
{
    QWT_D(d);
    const QPainter* painter = paintEngine()->painter();
    if ( painter == nullptr )
        return;

    d->commands += QwtPainterCommand( rect, pixmap, subRect );
    d->commandTypes |= QwtGraphic::RasterData;

    const QRectF r = painter->transform().mapRect( rect );
    updateControlPointRect( r );
    updateBoundingRect( r );
}

/*!
   @brief Store a image command in the command list

   @param rect target rectangle
   @param image Image to be painted
   @param subRect Reactangle of the pixmap to be painted
   @param flags Image conversion flags

   @sa QPaintEngine::drawImage()
 */
void QwtGraphic::drawImage( const QRectF& rect, const QImage& image,
    const QRectF& subRect, Qt::ImageConversionFlags flags )
{
    QWT_D(d);
    const QPainter* painter = paintEngine()->painter();
    if ( painter == nullptr )
        return;

    d->commands += QwtPainterCommand( rect, image, subRect, flags );
    d->commandTypes |= QwtGraphic::RasterData;

    const QRectF r = painter->transform().mapRect( rect );

    updateControlPointRect( r );
    updateBoundingRect( r );
}

/*!
   @brief Store a state command in the command list

   @param state State to be stored
   @sa QPaintEngine::updateState()
 */
void QwtGraphic::updateState( const QPaintEngineState& state )
{
    QWT_D(d);
    d->commands += QwtPainterCommand( state );

    if ( state.state() & QPaintEngine::DirtyTransform )
    {
        if ( !( d->commandTypes & QwtGraphic::Transformation ) )
        {
            /*
                QTransform::isScaling() returns true for all type
                of transformations beside simple translations
                even if it is f.e a rotation
             */
            if ( state.transform().isScaling() )
                d->commandTypes |= QwtGraphic::Transformation;
        }
    }
}

void QwtGraphic::updateBoundingRect( const QRectF& rect )
{
    QWT_D(d);
    QRectF br = rect;

    const QPainter* painter = paintEngine()->painter();
    if ( painter && painter->hasClipping() )
    {
        QRectF cr = painter->clipRegion().boundingRect();
        cr = painter->transform().mapRect( cr );

        br &= cr;
    }

    if ( d->boundingRect.width() < 0 )
        d->boundingRect = br;
    else
        d->boundingRect |= br;
}

void QwtGraphic::updateControlPointRect( const QRectF& rect )
{
    QWT_D(d);
    if ( d->pointRect.width() < 0.0 )
        d->pointRect = rect;
    else
        d->pointRect |= rect;
}

/**
 * @brief Get the list of recorded paint commands
 *
 * @return List of recorded paint commands
 *
 */
const QVector< QwtPainterCommand >& QwtGraphic::commands() const
{
    QWT_DC(d);
    return d->commands;
}

/**
 * @brief Append paint commands
 *
 * @param[in] commands Paint commands to append
 *
 */
void QwtGraphic::setCommands( const QVector< QwtPainterCommand >& commands )
{
    reset();

    const int numCommands = commands.size();
    if ( numCommands <= 0 )
        return;

    // to calculate a proper bounding rectangle we don't simply copy
    // the commands.

    const QwtPainterCommand* cmds = commands.constData();

    const QTransform noTransform;
    const RenderHints noRenderHints;

    QPainter painter( this );
    for ( int i = 0; i < numCommands; i++ )
        qwtExecCommand( &painter, cmds[i], noRenderHints, noTransform, nullptr );

    painter.end();
}
