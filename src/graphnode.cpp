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

#include <qpainter.h>
#include <qfontmetrics.h>
#include "graphnode.h"

/**
 * Class constructor.
 * @param	pParent		The parent graphics item
 * @param	sFunc		The node's function
 * @param	bMultiCall	Whether this node represents multiple calls
 */
GraphNode::GraphNode(QGraphicsItem* pParent, const QString& sFunc, bool bMultiCall) : 
	QGraphicsPolygonItem(pParent),
	m_sFunc(sFunc),
	m_bMultiCall(bMultiCall),
	m_bDfsFlag(false)
{
}

/**
 * Class destructor.
 */
GraphNode::~GraphNode()
{
	// Every node deletes its out-edges only
	qDeleteAll(m_dictOutEdges);
}

/**
 * Finds an edge leaving this node and reaching the given node.
 * If such an edge does not exist, a new one is created.
 * @param	pTail	The destination node
 * @return	The edge that ends at the given node
 */
GraphEdge* GraphNode::addOutEdge(GraphNode* pTail)
{
	GraphEdge* pEdge;

	// Look for the edge
	if ((pEdge = m_dictOutEdges.value(pTail->getFunc())) == NULL) {
		// Create a new edge
		pEdge = new GraphEdge(this, this, pTail);
		m_dictOutEdges.insert(pTail->getFunc(), pEdge);
		delete pTail->m_dictInEdges.take(m_sFunc);
		pTail->m_dictInEdges.insert(m_sFunc, pEdge);
	}

	// Return the new/constructed edge
	return pEdge;
}

/**
 * Performs a weak depth-first-search on the graph.
 * The search continues along all edges, both incoming and outgoing.
 */
void GraphNode::dfs()
{
	// Stop if this node is already marked
	if (m_bDfsFlag)
		return;

	// Mark the node as visited
	m_bDfsFlag = true;

	// Continue along outgoing edges
	QHash<QString, GraphEdge*>::iterator itrOut;
	for (itrOut = m_dictOutEdges.begin(); itrOut != m_dictOutEdges.end(); ++itrOut)
		(*itrOut)->getTail()->dfs();


	// Continue along incoming edges
	QHash<QString, GraphEdge*>::iterator itrIn;
	for (itrIn = m_dictInEdges.begin(); itrIn != m_dictInEdges.end(); ++itrIn)
		(*itrIn)->getHead()->dfs();
}

/**
 * Deletes all outgoing edges.
 */
void GraphNode::removeOutEdges()
{
	qDeleteAll(m_dictOutEdges);
	m_dictOutEdges.clear();
}

/**
 * Deletes all incoming edges.
 * To avoid double deletions, the function lets the head node of the edge remove
 * it.
 */
void GraphNode::removeInEdges()
{
	QHash<QString, GraphEdge*>::iterator itr;
	GraphNode* pNode;


	// Delete edges through their head nodes
	for (itr = m_dictInEdges.begin(); itr != m_dictInEdges.end(); ++itr) {
		if ((pNode = (*itr)->getHead()) != NULL)
			delete pNode->m_dictOutEdges.take(m_sFunc);
	}

	// remove edges from the local dictionary (will not delete them)
	m_dictInEdges.clear();
}

/**
 * Returns the first found node connected to this one.
 * This function is used with multi-call nodes for retrieving the parent node.
 * @param	pNode
 * @param	bCalled
 */
void GraphNode::getFirstNeighbour(GraphNode*& pNode, bool& bCalled)
{
	QHash<QString, GraphEdge*>::const_iterator itrIn = m_dictInEdges.constBegin();
	QHash<QString, GraphEdge*>::const_iterator itrOut = m_dictOutEdges.constBegin();

	if (itrIn.value()) {
		pNode = itrIn.value()->getHead();
		bCalled = false;
	} else if (itrOut.value()) {
		pNode = itrOut.value()->getTail();
		bCalled = true;
	} else {
		pNode = NULL;
	}
}

/**
 * Modifies the bounding rectangle of the node.
 * @param	rect	The new coordinates to set
 */
void GraphNode::setRect(const QRect& rect)
{
	QPolygon arr(4);

	m_rect = rect;

	arr.setPoint(0, m_rect.topLeft());
	arr.setPoint(1, m_rect.topRight());
	arr.setPoint(2, m_rect.bottomRight());
	arr.setPoint(3, m_rect.bottomLeft());
	setPolygon(arr);
}

/**
 * Draws the node.
 * @param	painter	Used for drawing the item on the canvas view
 */
void GraphNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	const QFont& font = painter->font();

	QGraphicsPolygonItem::paint(painter, option, widget);

	// Draw the text
	painter->setFont(m_font);
	if (m_bMultiCall)
		painter->drawText(m_rect, Qt::AlignCenter, "...");
	else
		painter->drawText(m_rect, Qt::AlignCenter, m_sFunc);

	painter->setFont(font);
}
