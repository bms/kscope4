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

#ifndef GRAPHEDGE_H
#define GRAPHEDGE_H

#include <QAbstractGraphicsShapeItem>
#include <QPolygon>

class GraphNode;

/**
 * Information used to draw arrow heads at the end of graph edges.
 */
struct ArrowInfo
{
	/** The length of the arrow. */
	double m_dLength;

	/** The tangent of the arrow's angle from the main line. */
	double m_dTan;

	/** Holds the value sqrt(1 + dTan^2). */
	double m_dSqrt;
};

/**
 * Draws a directed edge on a canvas.
 * The edge is composed of a spline, which is its body, and a polygon, which
 * is its head.
 * @author Elad Lahav
 */
class GraphEdge : public QAbstractGraphicsShapeItem
{
public:
	GraphEdge(QGraphicsItem*, GraphNode*, GraphNode*);
	~GraphEdge();

	void setCallInfo(const QString&, const QString&, const QString&);
	void setPoints(const QPolygon&, const ArrowInfo&);
	QString getTip() const;

	/**
	 * @return	The head node of the edge
	 */
	GraphNode* getHead() const { return m_pHead; }

	/**
	 * @return	The tail node of the edge
	 */
	GraphNode* getTail() const { return m_pTail; }

	/**
	 * @return	 The bounding rectangle of the edge's head
	 */
	QRect tipRect() const { return m_rcTip; }

	/**
	 * @return	The file path for this call
	 */
	const QString& getFile() const { return m_sFile; }

	/**
	 * @return	The line number for this call
	 */
	uint getLine() const { return m_sLine.toUInt(); }

	/**
	 * @return	The call's text
	 */
	const QString& getText() const { return m_sText; }

	/**
	 * @return	The type value
	 */
	enum { Type = UserType + 2 };

	virtual int type() const { return Type; }

protected:
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
	virtual QRectF boundingRect() const;

private:
	/** The edge's starting point. */
	GraphNode* m_pHead;

	/** The edge's end point. */
	GraphNode* m_pTail;

	/** The points of the polygon part of the edge. */
	QPolygon m_arrPoly;

	/** Control points for the spline part of the edge. */
	QPolygon m_arrCurve;

	/** The bounding rectangle of the edge's head, used for displaying the
		edge's tool-tip. */
	QRect m_rcTip;

	/** The call's source file. */
	QString m_sFile;

	/** The call's line number. */
	QString m_sLine;

	/** The call's text. */
	QString m_sText;

	void makeArrowhead(const ArrowInfo&);
};

#endif
