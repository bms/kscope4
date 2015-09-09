/***************************************************************************
 *
 * Copyright (C) 2005 Elad Lahav (elad_lahav@users.sourceforge.net)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <qpainter.h>
#include <QPainterPath>
#include <QAbstractGraphicsShapeItem>
#include <QPolygon>
#include "graphedge.h"
#include "graphnode.h"

/**
 * Class constructor.
 * @param	pParent	The parent on which to draw the edge
 * @param	pHead	The edge's starting point
 * @param	pTail	The edge's end point
 */
GraphEdge::GraphEdge(QGraphicsItem* pParent, GraphNode* pHead, GraphNode* pTail) :
	QAbstractGraphicsShapeItem(pParent),
	m_pHead(pHead),
	m_pTail(pTail),
	m_arrPoly(4)
{
}

/**
 * Class destructor.
 */
GraphEdge::~GraphEdge()
{
}

/**
 * Calculates the area surrounding the edge, and the bounding rectangle of
 * the edge's polygonal head.
 * The calculated area is used to find items on the canvas and to display
 * tips. The bounding rectangle defines the tip's region (@see QToolTip).
 * TODO: The function assumes that the we can simply append the polygon's
 * array to that of the splines. Is this always the case, or should we sort 
 * the points of the polygon in radial order?
 * @param	arrCurve	The control points of the edge's spline
 * @param	ai			Used to calculate the arrow head polygon
 */
void GraphEdge::setPoints(const QPolygon& arrCurve, const ArrowInfo& ai)
{
// 	ConvexHull ch;
	uint i;
	int nXpos, nYpos;

	// Redraw an existing edge
	prepareGeometryChange();

	// Store the point array for drawing
	m_arrCurve = arrCurve;

	// Calculate the arrowhead's polygon
	makeArrowhead(ai);

	// Calculate the head's bounding rectangle
	m_rcTip = QRect(m_arrPoly[0], m_arrPoly[0]);
	for (i = 1; i < m_arrPoly.size(); i++) {
		nXpos = m_arrPoly[i].x();
		if (nXpos < m_rcTip.left())
			m_rcTip.setLeft(nXpos);
		else if (nXpos > m_rcTip.right())
			m_rcTip.setRight(nXpos);

		nYpos = m_arrPoly[i].y();
		if (nYpos < m_rcTip.top())
			m_rcTip.setTop(nYpos);
		else if (nYpos > m_rcTip.bottom())
			m_rcTip.setBottom(nYpos);
	}
}

/**
 * Sets the call information associated with this edge.
 * @param	sFile	The call's file path
 * @param	sLine	The call's line number
 * @param	sText	The call's text
 */
void GraphEdge::setCallInfo(const QString& sFile, const QString& sLine,
	const QString& sText)
{
	m_sFile = sFile;
	m_sLine = sLine;
	m_sText = sText;
} 

/**
 * Constructs a tool-tip string for this edge.
 * @return	The tool-tip text
 */
QString GraphEdge::getTip() const
{
	QString sTip;

	sTip = m_sText + "<br><i>" + m_sFile + "</i>:" + m_sLine;
	return sTip;
}

/**
 * Draws the spline as a sequence of cubic Bezier curves.
 * @param	painter	Used for drawing the item on the canvas view
 */
void GraphEdge::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	uint i;

	// Set QPainter's brush to our own
	painter->setBrush(brush());

	// Draw the polygon
	painter->drawConvexPolygon(m_arrPoly);

#warning: loop condition rework
	// Draw the Bezier curves
	if (m_arrCurve.size())
	for (i = 0; i < m_arrCurve.size() - 1; i += 3) {
		QPainterPath path;

		path.moveTo(m_arrCurve.at(i));
		path.cubicTo(m_arrCurve.at(i + 1),
				m_arrCurve.at(i + 2),
				m_arrCurve.at(i + 3));
		painter->strokePath(path, painter->pen());
	}

}

QRectF GraphEdge::boundingRect() const
{
        return m_rcTip;
}

/**
 * Computes the coordinates of the edge's arrow head, based on its curve.
 * @param	ai	Pre-computed values used for all edges
 */
void GraphEdge::makeArrowhead(const ArrowInfo& ai)
{
	QPoint ptLast, ptPrev;
	double dX1, dY1, dX0, dY0, dX, dY, dDeltaX, dDeltaY, dNormLen;

	// The arrowhead follows the line from the second last to the last points
	// in the curve
	ptLast = m_arrCurve[m_arrCurve.size() - 1];
	ptPrev = m_arrCurve[m_arrCurve.size() - 2];

	// The first and last points of the polygon are the end of the curve
	m_arrPoly.setPoint(0, ptLast.x(), ptLast.y());
	m_arrPoly.setPoint(3, ptLast.x(), ptLast.y());

	// Convert integer points to double precision values
	dX1 = (double)ptLast.x();
	dY1 = (double)ptLast.y();
	dX0 = (double)ptPrev.x();
	dY0 = (double)ptPrev.y();

	// The values (x1-x0), (y1-y0) and sqrt(1 + tan(theta)^2) are useful
	dDeltaX = dX1 - dX0;
	dDeltaY = dY1 - dY0;

	// The normalised length of the arrow's sides
	dNormLen = ai.m_dLength / sqrt(dDeltaX * dDeltaX + dDeltaY * dDeltaY);

	// Compute the other two points
	dX = dX1 - ((dDeltaX - ai.m_dTan * dDeltaY) / ai.m_dSqrt) * dNormLen;
	dY = dY1 - ((dDeltaY + ai.m_dTan * dDeltaX) / ai.m_dSqrt) * dNormLen;
	m_arrPoly.setPoint(1, (int)dX, (int)dY);

	dX = dX1 - ((dDeltaX + ai.m_dTan * dDeltaY) / ai.m_dSqrt) * dNormLen;
	dY = dY1 - ((dDeltaY - ai.m_dTan * dDeltaX) / ai.m_dSqrt) * dNormLen;
	m_arrPoly.setPoint(2, (int)dX, (int)dY);
}
