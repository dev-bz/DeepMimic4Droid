/*
 * Copyright (c) 2006-2013 Erin Catto http://www.box2d.org
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */
#ifndef DEBUGDRAW_H
#define DEBUGDRAW_H
#ifdef DEMO
#include <LinearMath/btIDebugDraw.h>
#endif
struct b2AABB;
struct GLRenderPoints;
struct GLRenderLines;
struct GLRenderTriangles;
struct GLRenderObjects;
//
// This class implements debug drawing callbacks that are invoked
// inside b2World::Step.
struct Camera {
	int m_width, m_height;
	bool vr;
	void BuildProjectionMatrix(float *proj, float f);
};
class DebugDraw
#ifdef DEMO
		: public btIDebugDraw
#endif
{
public:
	DebugDraw();
	~DebugDraw();
	void Create();
	void Destroy();
#ifdef DEMO
	virtual void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color);
	virtual void drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime,
																const btVector3 &color);
	virtual void reportErrorWarning(const char *warningString);
	virtual void draw3dText(const btVector3 &location, const char *textString);
	virtual void setDebugMode(int debugMode);
	virtual int getDebugMode() const;
#endif
	/*void DrawPolygon(const b2Vec2 *vertices, int32 vertexCount, const b2Color &color);
	void DrawSolidPolygon(const b2Vec2 *vertices, int32 vertexCount, const b2Color &color);
	void DrawCircle(const b2Vec2 &center, float32 radius, const b2Color &color);
	void DrawSolidCircle(const b2Vec2 &center, float32 radius, const b2Vec2 &axis, const b2Color &color);
	void DrawObject(const float32 *mtx, const void *v, const void *n, const void *i, int vc, int tc, const b2Color
	&color); void DrawSegment(const b2Vec2 &p1, const b2Vec2 &p2, const b2Color &color); void DrawTransform(const
	b2Transform &xf); void DrawPoint(const b2Vec2 &p, float32 size, const b2Color &color); void DrawString(int x, int y,
	const char *string, ...); void DrawString(const b2Vec2 &p, const char *string, ...); void DrawAABB(b2AABB *aabb, const
	b2Color &color);*/
	void Flush();
	GLRenderObjects *m_objects;
	GLRenderLines *m_lines;
	GLRenderTriangles *m_triangles;
	GLRenderPoints *m_points;
};
extern DebugDraw g_debugDraw;
#endif